class eAIGroup
{
	static autoptr array<autoptr eAIGroup> GROUPS = new array<autoptr eAIGroup>();

	private static int m_IDCounter = 0;

	private autoptr array<eAITargetInformation> m_Targets;
	private int m_ID;

	// other groups can also reference this
    private autoptr eAIGroupTargetInformation m_TargetInformation;
	private autoptr eAIWaypointTargetInformation m_WaypointTargetInformation;

	// Ordered array of group members. 0 is the leader.
	private autoptr array<PlayerBase> m_Members;
	
	// What formation the group should keep
	private autoptr eAIFormation m_Form = new eAIFormationVee();
	
	// Group identity 
	private autoptr eAIFaction m_Faction = new eAIFactionRaiders();

	void eAIGroup()
	{
		
		m_TargetInformation = new eAIGroupTargetInformation(this);
		m_WaypointTargetInformation = new eAIWaypointTargetInformation(this);
		m_Targets = new array<eAITargetInformation>();

		m_IDCounter++;
		m_ID = m_IDCounter;

		m_Members = new array<PlayerBase>();

		GROUPS.Insert(this);
	}

	void ~eAIGroup()
	{
		int idx = GROUPS.Find(this);
		if (idx != -1) GROUPS.RemoveOrdered(idx);
	}
	
	void DeleteMembers() {
		for (int j = 0; j < m_Members.Count(); j++) {
			if (GetMember(j))
				GetGame().ObjectDelete(GetMember(j));
		}
		m_Members.Clear();
	}
	
	int AddWaypoint(vector pos) {
		return m_WaypointTargetInformation.AddWaypoint(pos);
	}
	
	void ClearWaypoints() {
		m_WaypointTargetInformation.ClearWaypoints();
	}
	
	int SkipWaypoint() {
		return m_WaypointTargetInformation.SkipWaypoint();
	}
	
	void SetLooping(bool loop) {
		m_WaypointTargetInformation.SetLooping(loop);
	}
	
	eAIFaction GetFaction() {
		return m_Faction;
	}
	
	void SetFaction(eAIFaction f) {
		m_Faction = f;
	}

	int GetID()
	{
		return m_ID;
	}

	void OnTargetAdded(eAITargetInformation target)
	{
		m_Targets.Insert(target);
	}

	void OnTargetRemoved(eAITargetInformation target)
	{
		m_Targets.RemoveItem(target);
	}

	void ProcessTargets()
	{
		for (int i = m_Targets.Count() - 1; i >= 0; i--) m_Targets[i].Process(m_ID);
	}

    eAITargetInformation GetTargetInformation()
    {
		return m_TargetInformation;
    }
	
	eAITargetInformation GetWaypointTargetInformation()
    {
		return m_WaypointTargetInformation;
    }

	void Update(float pDt)
	{
		ProcessTargets();

		m_TargetInformation.Update(pDt);
		m_WaypointTargetInformation.Update(pDt);
	}

	void SetLeader(PlayerBase leader)
	{
		if (!IsMember(leader)) AddMember(leader);

		PlayerBase temp = m_Members[0];
		if (temp == leader) return;
		m_Members[0] = leader;

		for (int i = 1; i < Count(); i++)
		{
			if (m_Members[i] == leader)
			{
				m_Members[i] = temp;
				return;
			}
		}
	}

	PlayerBase GetLeader()
	{
		for (int i = 0; i < m_Members.Count(); i++)
			if (m_Members[i])
				return m_Members[i];
		return null;
	}
	
	eAIFormation GetFormation()
	{
		return m_Form;
	}
	
	void SetFormation(eAIFormation f)
	{
		m_Form = f;
	}

	bool IsMember(PlayerBase player)
	{
		return m_Members.Find(player) != -1;
 	}
	
	int AddMember(PlayerBase member)
	{
		return m_Members.Insert(member);
	}
		
	bool RemoveMember(int i)
	{
		if (i < 0 || i >= m_Members.Count()) return false;

		m_Members.RemoveOrdered(i);
		return true;
	}
	
	PlayerBase GetMember(int i)
	{
		return m_Members[i];
	}

	int GetIndex(PlayerBase player)
	{
		return m_Members.Find(player);
	}
	
	int Count()
	{
		return m_Members.Count();
	}
};