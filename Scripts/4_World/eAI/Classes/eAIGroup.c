class eAIGroup
{
	static autoptr array<eAIGroup> GROUPS = new array<eAIGroup>();

	private static int m_IDCounter = 0;

	private autoptr array<eAITargetInformation> m_Targets;
	private int m_ID;

	// other groups can also reference this
    private autoptr eAIGroupTargetInformation m_TargetInformation;

	// Ordered array of group members. 0 is the leader.
	private autoptr array<PlayerBase> m_Members;
	
	// What formation the group should keep
	private autoptr eAIFormation m_Form = new eAIFormationVee();

	void eAIGroup()
	{
		m_TargetInformation = new eAIGroupTargetInformation(this);
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

	void Update(float pDt)
	{
		ProcessTargets();

		m_TargetInformation.Update(pDt);
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
		return m_Members[0];
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