class eAIPatrol : Managed
{
	private static autoptr array<ref eAIPatrol> m_AllPatrols = new array<ref eAIPatrol>();
	private static const float UPDATE_RATE_IN_SECONDS = 15.0;
	
	private ref Timer m_Timer;

	/**
	 * @brief Creates a dynamic patrol which spawns a patrol under the right conditions.
	 * 
	 * @param pos the position that the trigger distance is calculated from
	 * @param waypoints an array of points which the patrol will traverse
	 * @param behaviour how the waypoints will be traversed
	 * @param loadout the loadout each member is given @todo change to AI "type" which may have a different FSM/Goal tree
	 * @param count the number of ai to be spawned in the patrol
	 * @param minR miminum distance between the patrol and nearest player for a patrol to not (re)spawn
	 * @param maxR maximum distance between the patrol and nearest player for a patrol to (re)spawn
	 * 
	 * @return the patrol instance
	 */
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

	/**
	 * @brief Destroys a patrol
	 * 
	 * @param patrol the patrol to destroy
	 */
	static void DeletePatrol(eAIPatrol patrol)
	{
		int index = m_AllPatrols.Find(patrol);
		m_AllPatrols.Remove(index);
		delete patrol;
	}

	/**
	 * @brief Privated constructor to prevent calling/storing in ref. The instance is managed through 'Create(X)Patrol' and 'DeletePatrol'
	 */
	private void eAIPatrol()
	{
		m_AllPatrols.Insert(this);
	}

	/**
	 * @brief Privated destructor to prevent calling/storing in ref. The instance is managed through 'Create(X)Patrol' and 'DeletePatrol'
	 */
	private void ~eAIPatrol()
	{
		int idx = m_AllPatrols.Find(this);
		if (idx != -1) m_AllPatrols.RemoveOrdered(idx);

		Stop();
	}

	/**
	 * @brief Destroys this patrol on the next frame
	 */
	void Delete()
	{
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Call(DeletePatrol, this);
	}

	/**
	 * @brief Waits around a second and then calls OnUpdate at a frequency specified in UPDATE_RATE_IN_SECONDS in a new timer
	 */
	void Start()
	{
		//DelayedStart();
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.DelayedStart, Math.RandomInt(1, 1000), false);

		OnUpdate();
	}

	private void DelayedStart()
	{
		if (!m_Timer) m_Timer = new Timer(CALL_CATEGORY_GAMEPLAY);
		m_Timer.Run(UPDATE_RATE_IN_SECONDS, this, "OnUpdate");
	}

	/**
	 * @brief Stops the timer and OnUpdate from being called.
	 */
	void Stop()
	{
		if (m_Timer && m_Timer.IsRunning()) m_Timer.Stop();
	}

	/**
	 * @brief Abstract function. 
	 */
	void OnUpdate()
	{

	}
};