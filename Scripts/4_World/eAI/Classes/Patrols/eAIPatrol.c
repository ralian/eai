class eAIPatrol : Managed
{
	private static autoptr array<ref eAIPatrol> m_AllPatrols = new array<ref eAIPatrol>();
	static const float UPDATE_RATE_IN_SECONDS = 5.0;
	
	private ref Timer m_Timer;
	private bool m_IsBeingDestroyed;

	/**
	 * @brief Destroys a patrol
	 * 
	 * @param patrol the patrol to destroy
	 */
	static void DeletePatrol(eAIPatrol patrol)
	{
		int index = m_AllPatrols.Find(patrol);
		m_AllPatrols.Remove(index);
	}

	/**
	 * @brief Privated constructor to prevent calling/storing in ref. The instance is managed through 'Create(X)Patrol' and 'DeletePatrol'
	 */
	private void eAIPatrol()
	{
        //eAITrace trace(this, "eAIPatrol");
		m_AllPatrols.Insert(this);
	}

	/**
	 * @brief Privated destructor to prevent calling/storing in ref. The instance is managed through 'Create(X)Patrol' and 'DeletePatrol'
	 */
	private void ~eAIPatrol()
	{
        //eAITrace trace(this, "~eAIPatrol");
		int idx = m_AllPatrols.Find(this);
		if (idx != -1) m_AllPatrols.RemoveOrdered(idx);

		Stop();
	}

	/**
	 * @brief Destroys this patrol on the next frame
	 */
	void Delete()
	{
        //eAITrace trace(this, "Delete");
		m_IsBeingDestroyed = true;
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Call(DeletePatrol, this);
	}

	/**
	 * @brief Returns true if the patrol is being destroyed
	 */
	bool IsBeingDestroyed()
	{
        //eAITrace trace(this, "IsBeingDestroyed");
		return m_IsBeingDestroyed;
	}

	/**
	 * @brief Waits around a second and then calls OnUpdate at a frequency specified in UPDATE_RATE_IN_SECONDS in a new timer
	 */
	void Start()
	{
        //eAITrace trace(this, "Start");
		//DelayedStart();
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.DelayedStart, Math.RandomInt(1, 1000), false);

		OnUpdate();
	}

	private void DelayedStart()
	{
        //eAITrace trace(this, "DelayedStart");
		if (!m_Timer) m_Timer = new Timer(CALL_CATEGORY_GAMEPLAY);
		m_Timer.Run(UPDATE_RATE_IN_SECONDS, this, "OnUpdate", null, true);
	}

	/**
	 * @brief Stops the timer and OnUpdate from being called.
	 */
	void Stop()
	{
        //eAITrace trace(this, "Stop");
		if (m_Timer && m_Timer.IsRunning()) m_Timer.Stop();
	}

	/**
	 * @brief Abstract function. 
	 */
	void OnUpdate()
	{

	}
};