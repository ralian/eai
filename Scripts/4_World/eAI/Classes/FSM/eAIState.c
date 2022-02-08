class eAIState
{
	static const int EXIT = 0;
	static const int CONTINUE = 1;

	string m_Name;
	string m_ClassName;

	//! only used if there is a sub-fsm
	ref eAIFSM m_SubFSM;

	/* STATE VARIABLES */
	//eAIFSM fsm;
	eAIState parent;
	eAIBase unit;

	void eAIState(eAIFSM _fsm, eAIBase _unit)
	{
		//fsm = _fsm;
		parent = _fsm.GetParent();
		unit = _unit;
	}
	
	string GetName()
	{
		return m_Name;
	}

	/* IMPLEMENTED IN XML */
	void OnEntry(string Event, eAIState From)
	{

	}

	void OnExit(string Event, bool Aborted, eAIState To)
	{

	}

	int OnUpdate(float DeltaTime, int SimulationPrecision)
	{
		return CONTINUE;
	}

	#ifdef EAI_DEBUG_FSM
	//! Current active debug block
	CF_DebugUI_Block m_Debug;
	int m_Depth;

	private string Debug_Prefix()
	{
		int depth = m_Depth;
		string str = "";
		while (depth > 0)
		{
			str += " ";
			depth--;
		}
		return str + "[S] ";
	}

	void Debug_Set(string key, int text)
	{
		m_Debug.Set(Debug_Prefix() + key, text);
	}

	void Debug_Set(string key, bool text)
	{
		m_Debug.Set(Debug_Prefix() + key, text);
	}

	void Debug_Set(string key, float text)
	{
		m_Debug.Set(Debug_Prefix() + key, text);
	}

	void Debug_Set(string key, vector text)
	{
		m_Debug.Set(Debug_Prefix() + key, text);
	}

	void Debug_Set(string key, Class text)
	{
		m_Debug.Set(Debug_Prefix() + key, text);
	}

	void Debug_Set(string key, string text)
	{
		m_Debug.Set(Debug_Prefix() + key, text);
	}

	int Debug_OnUpdate(Class dbg, int depth, float DeltaTime, int SimulationPrecision)
	{
		Class.CastTo(m_Debug, dbg);
		m_Depth = depth;

		Debug_Set("State", m_ClassName);
		int ret = OnUpdate(DeltaTime, SimulationPrecision);
		switch (ret)
		{
			case EXIT:
				Debug_Set("Status", "EXIT");
				break;
			case CONTINUE:
				Debug_Set("Status", "CONTINUE");
				break;
			default:
				Debug_Set("Status", "UNKNOWN");
				break;
		}
		return ret;
	}
	#else
	void Debug_Set(string key, int text)
	{
	}

	void Debug_Set(string key, bool text)
	{
	}

	void Debug_Set(string key, float text)
	{
	}

	void Debug_Set(string key, vector text)
	{
	}

	void Debug_Set(string key, Class text)
	{
	}

	void Debug_Set(string key, string text)
	{
	}

	int Debug_OnUpdate(Class dbg, int depth, float DeltaTime, int SimulationPrecision)
	{
		return OnUpdate(DeltaTime, SimulationPrecision);
	}
	#endif
};