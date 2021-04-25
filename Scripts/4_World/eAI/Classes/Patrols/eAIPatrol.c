class eAIPatrol : Managed
{
	private static autoptr array<ref eAIPatrol> m_AllPatrols = new array<ref eAIPatrol>();
	private static const float UPDATE_RATE_IN_SECONDS = 15.0;
	
	private ref Timer m_Timer;

	// @brief Create a dynamic patrol object which spawns a patrol under the right conditions.
	// @param pos the position that the trigger distance is calculated from
	// @param waypoints the waypoints the patrol follows. Only stores a soft pointer.
	// @param loadout the loadout each member is given
	// @param count the number of ai
	// @param minR minimum radius a player can be away from the spawn point, if a player teleports or spawns right on top of it, they won't
	// @param maxR distance at which an incoming player will spawn the patrol
	static eAIDynamicPatrol CreateDynamicPatrol(vector pos, array<vector> waypoints, eAIWaypointBehavior behaviour, string loadout = "SoldierLoadout.json", int count = 1, float minR = 300, float maxR = 800)
	{
		eAIDynamicPatrol patrol = new eAIDynamicPatrol();
		patrol.m_Position = pos;
		patrol.m_Waypoints = waypoints;
		patrol.m_WaypointBehaviour = behaviour;
		patrol.m_MinimumRadius = minR;
		patrol.m_MaximumRadius = maxR;
		patrol.m_DespawnRadius = maxR * 1.1;
		patrol.m_NumberOfAI = count;
		patrol.m_Loadout = loadout;
		return patrol;
	}

	static void DeletePatrol(eAIPatrol patrol)
	{
		int index = m_AllPatrols.Find(patrol);
		m_AllPatrols.Remove(index);
		delete patrol;
	}

	private void eAIPatrol()
	{
		m_AllPatrols.Insert(this);
	}

	private void ~eAIPatrol()
	{
		int idx = m_AllPatrols.Find(this);
		if (idx != -1) m_AllPatrols.RemoveOrdered(idx);

		Stop();
	}

	void Delete()
	{
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Call(DeletePatrol, this);
	}

	void Start()
	{
		//DelayedStart();
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.DelayedStart, Math.RandomInt(1, 1000), false);

		OnUpdate();
	}

	private void DelayedStart()
	{
		m_Timer = new Timer(CALL_CATEGORY_GAMEPLAY);
		m_Timer.Run(UPDATE_RATE_IN_SECONDS, this, "OnUpdate");
	}

	void Stop()
	{
		if (m_Timer && m_Timer.IsRunning())
			m_Timer.Stop();
	}

	void OnUpdate()
	{

	}
};