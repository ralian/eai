enum eAIWaypointBehavior
{
	HALT,
	LOOP,
	REVERSE
};

class eAIGroup
{
	static autoptr array<eAIGroup> GROUPS = new array<eAIGroup>();

	private static int m_IDCounter = 0;

	private autoptr array<eAITargetInformation> m_Targets;
	private int m_ID;

	//! Refer to eAIGroup::GetTargetInformation
    private autoptr eAIGroupTargetInformation m_TargetInformation;

	// Ordered array of group members. 0 is the leader.
	private autoptr array<DayZPlayerImplement> m_Members;
	
	// What formation the group should keep
	private autoptr eAIFormation m_Form = new eAIFormationVee();
	
	// Group identity 
	private autoptr eAIFaction m_Faction = new eAIFactionRaiders();

	private autoptr array<vector> m_Waypoints;
	private eAIWaypointBehavior m_WaypointBehaviour = eAIWaypointBehavior.REVERSE;

	void eAIGroup()
	{
		m_TargetInformation = new eAIGroupTargetInformation(this);
		m_Targets = new array<eAITargetInformation>();

		m_IDCounter++;
		m_ID = m_IDCounter;

		m_Members = new array<DayZPlayerImplement>();
		
		m_Waypoints = new array<vector>();

		GROUPS.Insert(this);
	}

	void ~eAIGroup()
	{
		int idx = GROUPS.Find(this);
		if (idx != -1) GROUPS.RemoveOrdered(idx);
	}
	
	void AddWaypoint(vector pos)
	{
		m_Waypoints.Insert(pos);
	}

	array<vector> GetWaypoints()
	{
		return m_Waypoints;
	}
	
	void ClearWaypoints()
	{
		m_Waypoints.Clear();
	}

	//TODO: rename to SetWaypointBehaviour
	void SetLooping(eAIWaypointBehavior bhv)
	{
		m_WaypointBehaviour = bhv;
	}

	eAIWaypointBehavior GetWaypointBehaviour()
	{
		return m_WaypointBehaviour;
	}
	
	eAIFaction GetFaction() {
		return m_Faction;
	}
	
	void SetFaction(eAIFaction f) {
		m_Faction = f;
	}

	/**
	 * @brief The unique ID for this group
	 *
	 * @return int
	 */
	int GetID()
	{
		return m_ID;
	}

	/**
	 * @brief Internal event fired when this group needs to know that is now targetting something
	 *
	 * @param target The target being added
	 */
	void OnTargetAdded(eAITargetInformation target)
	{
		m_Targets.Insert(target);
	}

	/**
	 * @brief Internal event fired when this group needs to know that is no longer targetting something
	 *
	 * @param target The target being removed
	 */
	void OnTargetRemoved(eAITargetInformation target)
	{
		m_Targets.RemoveItem(target);
	}

	/**
	 * @brief Processes all the targets so they can be removed when the time has been exceeded
	 */
	void ProcessTargets()
	{
		for (int i = m_Targets.Count() - 1; i >= 0; i--) m_Targets[i].Process(m_ID);
	}

	/**
	 * @brief This target is both used by the owned AI's and enemy groups.
	 * The owned AI's will use this to get the position they should move to
	 * The enemy AI's will use this similar to a normal entity if they are 
	 * targetting the group as a whole and not a singular AI. If they are 
	 * targetting a singular AI then they would use GetTargetInformation.
	 *
	 * @return the target
	 */
    eAITargetInformation GetTargetInformation()
    {
		return m_TargetInformation;
    }

	void Update(float pDt)
	{
		ProcessTargets();

		m_TargetInformation.Update(pDt);
	}

	void SetLeader(DayZPlayerImplement leader)
	{
		if (!IsMember(leader)) AddMember(leader);

		DayZPlayerImplement temp = m_Members[0];
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

	DayZPlayerImplement GetLeader()
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

	bool IsMember(DayZPlayerImplement player)
	{
		return m_Members.Find(player) != -1;
 	}
	
	int AddMember(DayZPlayerImplement member)
	{
		return m_Members.Insert(member);
	}
		
	bool RemoveMember(int i)
	{
		if (i < 0 || i >= m_Members.Count()) return false;

		m_Members.RemoveOrdered(i);
		return true;
	}
	
	DayZPlayerImplement GetMember(int i)
	{
		return m_Members[i];
	}

	int GetIndex(DayZPlayerImplement player)
	{
		return m_Members.Find(player);
	}
	
	int Count()
	{
		return m_Members.Count();
	}
};