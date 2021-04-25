class eAIDynamicPatrol : eAIPatrol
{
	vector m_Position;
	autoptr array<vector> m_Waypoints;
	eAIWaypointBehavior m_WaypointBehaviour;
	float m_MinimumRadius;
	float m_MaximumRadius;
	float m_DespawnRadius; // m_MaximumRadius + 10%
	int m_NumberOfAI;
	string m_Loadout;

	eAIGroup m_Group;
	int m_LastSpawnIndex;

	private eAIBase SpawnAI(vector pos)
	{
		eAIBase ai;
		if (!Class.CastTo(ai, GetGame().CreateObject(GetRandomAI(), pos))) return null;

		HumanLoadout.Apply(ai, m_Loadout);
				
		return ai;
	}

	private bool IsGroupDestroyed()
	{
		for (int i = 0; i < m_Group.Count(); i++)
		{
			if (m_Group.GetMember(i) && m_Group.GetMember(i).IsAlive())
			{
				return false;
			}
		}

		return true;
	}

	override void OnUpdate()
	{
		super.OnUpdate();
		
		if (!m_Group) m_LastSpawnIndex++;

		vector patrolPos = m_Position;
		if (m_Group && m_Group.GetLeader()) patrolPos = m_Group.GetLeader().GetPosition();
		
		autoptr array<Man> players = {};
		GetGame().GetPlayers(players);
		float minimumDistance = 50000.0;
		foreach (auto player : players)
		{
			float dist = vector.Distance(patrolPos, player.GetPosition());
			if (dist < minimumDistance) minimumDistance = dist;
		}

		if (m_Group)
		{
			if (IsGroupDestroyed() || minimumDistance > m_DespawnRadius)
			{
				m_Group.RemoveAllMembers();
				m_LastSpawnIndex = 0;
			}
		}
		else
		{
			if (minimumDistance < m_MaximumRadius && minimumDistance > m_MinimumRadius)
			{
				eAIBase ai = SpawnAI(m_Position);
				m_Group = ai.GetGroup();
				m_Group.SetWaypointBehaviour(m_WaypointBehaviour);
				foreach (vector v : m_Waypoints) m_Group.AddWaypoint(v);

				int count = m_NumberOfAI;
				while (count > 1)
				{
					ai = SpawnAI(m_Position);
					ai.SetGroup(m_Group);
					count--;
				}
			}
		}
	}
};