class eAIFSM
{
    static const int EXIT = 0;
    static const int CONTINUE = 1;

    private autoptr array<ref eAIState> m_States;
    private autoptr array<ref eAITransition> m_Transitions;

    private eAIState m_CurrentState;
    private eAIState m_ParentState;
    private bool m_Running;

    protected string m_Name;
    protected string m_DefaultState;
    protected eAIBase m_Unit;

    void eAIFSM(eAIBase unit, eAIState parentState)
    {
        m_Unit = unit;
        m_ParentState = parentState;

        m_States = new array<ref eAIState>();
        m_Transitions = new array<ref eAITransition>();
    }

    string GetName()
    {
        return m_Name;
    }

    eAIBase GetUnit()
    {
        return m_Unit;
    }

    eAIState GetParent()
    {
        return m_ParentState;
    }

    void AddState(eAIState state)
    {
        m_States.Insert(state);
    }

    void AddTransition(eAITransition transition)
    {
        m_Transitions.Insert(transition);
    }

    void SortTransitions()
    {
        //TODO: if the source transition is null, push to the back of the array
        //TODO: if the destination transition is null, remove from the array
    }
	
	eAIState GetState()
    {
        return m_CurrentState;
    }

    eAIState GetState(string type)
    {
        for (int i = 0; i < m_States.Count(); i++) if (m_States[i].ClassName() == type) return m_States[i];

        return null;
    }

    eAIState GetState(typename type)
    {
        for (int i = 0; i < m_States.Count(); i++) if (m_States[i].Type() == type) return m_States[i];

        return null;
    }
	
	bool StartDefault()
	{
        //eAITrace trace(this, "StartDefault");

        eAIState src = m_CurrentState;
        eAIState dst = GetState(m_DefaultState);

        if (m_Running && src)
        {
            eAILogger.Debug("Exiting state: " + src);
            src.OnExit("", true, dst);
        }
	
		m_CurrentState = dst;
		
        if (m_CurrentState)
        {
            eAILogger.Debug("Starting state: " + m_CurrentState);
            m_CurrentState.OnEntry("", src);
            return true;
        }
		
        eAILogger.Warn("No valid state found.");
		
        return false;
	}

    bool Start(string e = "")
    {
        //eAITrace trace(this, "Start", e);

        Param2<eAIState, bool> new_state = FindSuitableTransition(m_CurrentState, "");

        eAIState src = m_CurrentState;
        eAIState dst = new_state.param1;

        if (dst == null)
        {
            eAILogger.Warn("No valid state found. Aborting.");

            return false;
        }

        if (m_Running && m_CurrentState && m_CurrentState != dst)
        {
            eAILogger.Debug("Exiting state: " + m_CurrentState);
            m_CurrentState.OnExit(e, true, dst);
        }

        m_CurrentState = dst;

        if (m_CurrentState && src != m_CurrentState)
        {
            eAILogger.Debug("Starting state: " + m_CurrentState);
            m_CurrentState.OnEntry(e, src);
            return true;
        }

        eAILogger.Warn("No valid state found.");

        return false;
    }

    bool Abort(string e = "")
    {
        //eAITrace trace(this, "Abort", e);

        if (m_Running && m_CurrentState)
        {
            eAILogger.Debug("Exiting state: " + m_CurrentState);
            m_CurrentState.OnExit(e, true, null);
            return true;
        }

        return false;
    }

    /**
     * @return true Tell the parent FSM that the child FSM is complete
     * @return false Tell the parent FSM that the child FSM is still running
     */
    int Update(float pDt, int pSimulationPrecision)
    {
        //eAITrace trace(this, "Update", pDt.ToString(), pSimulationPrecision.ToString());

        //eAILogger.Debug("m_CurrentState: %1", "" + m_CurrentState);

        if (m_CurrentState && m_CurrentState.OnUpdate(pDt, pSimulationPrecision) == CONTINUE) return CONTINUE;

        Param2<eAIState, bool> new_state = FindSuitableTransition(m_CurrentState, "");
        if (!new_state.param2 || (new_state.param2 && m_CurrentState == new_state.param1))
        {	
            if (!m_CurrentState) return EXIT;

            return CONTINUE;
        }

        eAIState src = m_CurrentState;

        if (m_CurrentState) m_CurrentState.OnExit("", false, new_state.param1);

        m_CurrentState = new_state.param1;

        if (new_state.param1 == null) return EXIT;
		
		eAILogger.Info("State transition " + src.GetName() + " -> " + m_CurrentState.GetName());

        m_CurrentState.OnEntry("", src);

        return CONTINUE;
    }
	
	Param2<eAIState, bool> FindSuitableTransition(eAIState s, string e = "")
	{
        //eAITrace trace(this, "FindSuitableTransition", "" + s, e);

        // returns tuple as a valid destination can still be null

        //TODO: store a reference to the transitions inside the state for that state

		eAIState curr_state = s;

		int count = m_Transitions.Count();
		for (int i = 0; i < count; ++i)
		{
			auto t = m_Transitions.Get(i);
			if ((t.GetSource() == curr_state || t.GetSource() == null) && (e == "" || (e != "" && t.GetEvent() == e)))
			{
				int guard = t.Guard();
                switch (guard)
                {
                case eAITransition.SUCCESS:
				    return new Param2<eAIState, bool>(t.GetDestination(), true);
                case eAITransition.FAIL:
				    break;
                }
			}
		}

		return new Param2<eAIState, bool>(null, false);
	}
};