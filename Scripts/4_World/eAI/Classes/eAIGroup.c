enum eAIWaypointBehavior
{
	HALT,
	LOOP,
	REVERSE
};

enum eAIGroupFormationState
{
	NONE,
	IN
};

//TODO: sync to the client automatically within DayZPlayerImplement
// only data that needs to be known is just the id, members and faction
class eAIGroup
{
	private static autoptr array<ref eAIGroup> s_AllGroups = new array<ref eAIGroup>();

	private static int s_IDCounter = 0;

	private autoptr array<eAITargetInformation> m_Targets;
	private int m_ID;

	//! Refer to eAIGroup::GetTargetInformation
    private autoptr eAIGroupTargetInformation m_TargetInformation;

	// Ordered array of group members. 0 is the leader.
	private autoptr array<DayZPlayerImplement> m_Members;
	
	// What formation the group should keep
	private autoptr eAIFormation m_Form;
	
	// Group identity 
	private autoptr eAIFaction m_Faction = new eAIFactionRaiders();

	private autoptr array<vector> m_Waypoints;
	private eAIWaypointBehavior m_WaypointBehaviour = eAIWaypointBehavior.REVERSE;

	private eAIGroupFormationState m_FormationState = eAIGroupFormationState.IN;
	
	// return the group owned by leader, otherwise create a new one.
	static eAIGroup GetGroupByLeader(DayZPlayerImplement leader, bool createIfNoneExists = true)
	{
        //eAITrace trace(null, "eAIGroup::GetGroupByLeader", Object.GetDebugName(leader), createIfNoneExists.ToString());
		for (int i = 0; i < s_AllGroups.Count(); i++) if (s_AllGroups[i].GetLeader() == leader) return s_AllGroups[i];
		
		if (!createIfNoneExists) return null;
		
		eAIGroup group = CreateGroup();
		leader.SetGroup(group);
		return group;
	}

	static eAIGroup GetGroupByID(int id, bool createIfNoneExists = false)
	{
        //eAITrace trace(null, "eAIGroup::GetGroupByID", id.ToString(), createIfNoneExists.ToString());
		for (int i = 0; i < s_AllGroups.Count(); i++) if (s_AllGroups[i].GetID() == id) return s_AllGroups[i];
		
		if (!createIfNoneExists) return null;
		
		eAIGroup group = new eAIGroup();
		group.m_ID = id;
		return group;
	}

	static eAIGroup CreateGroup()
	{
        //eAITrace trace(null, "eAIGroup::CreateGroup");
		
		eAIGroup group = new eAIGroup();

		s_IDCounter++;
		group.m_ID = s_IDCounter;
		
		return group;
	}

	static void DeleteGroup(eAIGroup group)
	{
        //eAITrace trace(null, "eAIGroup::DeleteGroup");
		int index = s_AllGroups.Find(group);
		s_AllGroups.Remove(index);
		delete group;
	}

	private void eAIGroup()
	{
        //eAITrace trace(this, "eAIGroup");

		m_TargetInformation = new eAIGroupTargetInformation(this);
		m_Targets = new array<eAITargetInformation>();
		
		m_Form = new eAIFormationVee(this);

		m_Members = new array<DayZPlayerImplement>();
		
		m_Waypoints = new array<vector>();

		s_AllGroups.Insert(this);
	}

	private void ~eAIGroup()
	{
        //eAITrace trace(this, "~eAIGroup");

		int idx = s_AllGroups.Find(this);
		if (idx != -1) s_AllGroups.RemoveOrdered(idx);
	}

	void Delete()
	{
        //eAITrace trace(this, "Delete");
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Call(DeleteGroup, this);
	}
	
	void AddWaypoint(vector pos)
	{
        //eAITrace trace(this, "AddWaypoint");
		m_Waypoints.Insert(pos);
	}

	array<vector> GetWaypoints()
	{
        //eAITrace trace(this, "GetWaypoints");
		return m_Waypoints;
	}
	
	void ClearWaypoints()
	{
        //eAITrace trace(this, "ClearWaypoints");
		m_Waypoints.Clear();
	}

	void SetWaypointBehaviour(eAIWaypointBehavior bhv)
	{
        //eAITrace trace(this, "SetWaypointBehaviour");
		m_WaypointBehaviour = bhv;
	}

	eAIWaypointBehavior GetWaypointBehaviour()
	{
        //eAITrace trace(this, "GetWaypointBehaviour");
		return m_WaypointBehaviour;
	}

	void SetFormationState(eAIGroupFormationState state)
	{
        //eAITrace trace(this, "SetFormationState")
		m_FormationState = state;
	}

	eAIGroupFormationState GetFormationState()
	{
        //eAITrace trace(this, "GetFormationState");
		return m_FormationState;
	}
	
	void SetFaction(eAIFaction f)
	{
        //eAITrace trace(this, "SetFaction");
		m_Faction = f;
	}
	
	eAIFaction GetFaction()
	{
        //eAITrace trace(this, "GetFaction");
		return m_Faction;
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
        //eAITrace trace(this, "OnTargetAdded", target.DebugString());
		m_Targets.Insert(target);
	}

	/**
	 * @brief Internal event fired when this group needs to know that is no longer targetting something
	 *
	 * @param target The target being removed
	 */
	void OnTargetRemoved(eAITargetInformation target)
	{
        //eAITrace trace(this, "OnTargetRemoved", target.DebugString());
		m_Targets.RemoveItem(target);
	}

	/**
	 * @brief Processes all the targets so they can be removed when the time has been exceeded
	 */
	void ProcessTargets()
	{
        //eAITrace trace(this, "ProcessTargets");
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
        //eAITrace trace(this, "GetTargetInformation");
		return m_TargetInformation;
    }

	void Update(float pDt)
	{
        //eAITrace trace(this, "Update");
		ProcessTargets();

		m_Form.Update(pDt);
	}

	static void UpdateAll(float pDt)
	{
        ////eAITrace trace(null, "eAIGroup::UpdateAll");
        // don't process if we aren't the server
        if (!GetGame().IsServer()) return;

		for (int i = 0; i < s_AllGroups.Count(); i++) s_AllGroups[i].Update(pDt);
	}

	int GetMemberIndex(eAIBase ai)
	{
        //eAITrace trace(this, "GetMemberIndex", Object.GetDebugName(ai));

		int pos = 0;
		vector position = "0 0 0";
				
		for (int i = 0; i < m_Members.Count(); i++)
		{
			// ignore members who have died so their position can be taken over
			if (!m_Members[i].IsAlive()) continue;
			
			if (m_Members[i] == ai) return pos;

			pos++;
		}
		
		return -1;
	}

	vector GetFormationPosition(eAIBase ai)
	{
        //eAITrace trace(this, "GetFormationPosition", Object.GetDebugName(ai));

		int pos = GetMemberIndex(ai);
		vector position = "0 0 0";
		if (pos != -1) position = m_Form.GetPosition(pos);
		return m_Form.ToWorld(position);
	}

	void SetLeader(DayZPlayerImplement leader)
	{
        //eAITrace trace(this, "SetLeader", Object.GetDebugName(leader));

		if (!IsMember(leader)) AddMember(leader);

		DayZPlayerImplement temp = m_Members[0];
		if (temp == leader) return;
		m_Members[0] = leader;
		m_Members[0].SetGroupMemberIndex(0);

		for (int i = 1; i < Count(); i++)
		{
			if (m_Members[i] && m_Members[i] == leader)
			{
				m_Members[i] = temp;
				m_Members[i].SetGroupMemberIndex(i);
				return;
			}
		}
	}

	DayZPlayerImplement GetLeader()
	{
        //eAITrace trace(this, "GetLeader");
		return m_Members[0];
	}
	
	eAIFormation GetFormation()
	{
        //eAITrace trace(this, "GetFormation");
		return m_Form;
	}
	
	void SetFormation(eAIFormation f)
	{
        //eAITrace trace(this, "SetFormation");
		m_Form = f;
	}

	bool IsMember(DayZPlayerImplement member)
	{
        //eAITrace trace(this, "IsMember", Object.GetDebugName(member));
		return m_Members.Find(member) != -1;
 	}
	
	int AddMember(DayZPlayerImplement member)
	{
        //eAITrace trace(this, "AddMember", Object.GetDebugName(member));
		return m_Members.Insert(member);
	}

	void Client_SetMemberIndex(DayZPlayerImplement member, int index)
	{
        //eAITrace trace(this, "Client_SetMemberIndex", Object.GetDebugName(member), index.ToString());
		if (index >= m_Members.Count())
		{
			m_Members.Resize(index + 1);
		}

		m_Members[index] = member;

		int removeFrom = m_Members.Count();

		for (int i = m_Members.Count() - 1; i > index; i--)
		{
			if (m_Members[i] != null) break;
			removeFrom = i;
		}

		m_Members.Resize(removeFrom);
	}

	bool RemoveMember(DayZPlayerImplement member, bool autoDelete = true)
	{
        //eAITrace trace(this, "RemoveMember", Object.GetDebugName(member), autoDelete.ToString());
		return RemoveMember(m_Members.Find(member), autoDelete);
	}

	bool RemoveMember(int i, bool autoDelete = true)
	{
        //eAITrace trace(this, "RemoveMember", i.ToString(), autoDelete.ToString());
		if (i < 0 || i >= m_Members.Count()) return false;

		m_Members.RemoveOrdered(i);

		if (autoDelete && m_Members.Count() == 0)
		{
			Delete();
		}

		return true;
	}

	void RemoveAllMembers(bool autoDelete = true)
	{
        //eAITrace trace(this, "RemoveAllMembers", autoDelete.ToString());
		m_Members.Clear();

		if (autoDelete)
		{
			Delete();
		}
	}
	
	DayZPlayerImplement GetMember(int i)
	{
        //eAITrace trace(this, "GetMember", i.ToString());
		return m_Members[i];
	}

	int GetIndex(DayZPlayerImplement player)
	{
        //eAITrace trace(this, "GetIndex", Object.GetDebugName(player));
		return m_Members.Find(player);
	}
	
	int Count()
	{
        //eAITrace trace(this, "Count");
		return m_Members.Count();
	}

	static void DeleteAllAI()
	{
        //eAITrace trace(null, "eAIGroup::DeleteAllAI");
		foreach (eAIGroup group : s_AllGroups)
		{
			for (int i = group.Count() - 1; i > -1; i--)
			{
				eAIBase ai;
				if (Class.CastTo(ai, group.GetMember(i)))
				{
					group.RemoveMember(i);
					GetGame().ObjectDelete(ai);
				}
			}	
		}
	}

	static void OnHeadlessClientConnect(PlayerIdentity identity)
	{
        //eAITrace trace(null, "eAIGroup::OnHeadlessClientConnect");
		foreach (eAIGroup group : s_AllGroups)
		{
			for (int i = 0; i < group.Count(); i++)
			{
				eAIBase ai;
				if (Class.CastTo(ai, group.GetMember(i)) && ai.IsAlive()) GetRPCManager().SendRPC("eAI", "HCLinkObject", new Param1<PlayerBase>(ai), false, identity);
			}
		}
	}
};