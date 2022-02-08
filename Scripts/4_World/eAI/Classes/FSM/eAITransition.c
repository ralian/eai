class eAITransition
{
	static const int FAIL = 0;
	static const int SUCCESS = 1; 

	protected string m_ClassName;
	
	/* STATE VARIABLES */
	protected eAIBase unit;

	void eAITransition(eAIFSM _fsm, eAIBase _unit)
	{
		unit = _unit;
	}

	eAIState GetSource()
	{
		return null;
	}

	eAIState GetDestination()
	{
		return null;
	}

	string GetEvent()
	{
		return "";
	}

	int Guard()
	{
		return SUCCESS;
	}

	#ifdef EAI_DEBUG_FSM
	//! Current active debug block
	CF_DebugUI_Block m_Debug;
	int m_Depth;
	int m_Index;

	private string Debug_Prefix()
	{
		int depth = m_Depth;
		string str = "";
		while (depth > 0)
		{
			str += " ";
			depth--;
		}

		str += "[T_" + m_Index + "]";
		if (m_Index < 10) depth = 2;
		else if (m_Index < 100) depth = 1;

		while (depth > 0)
		{
			str += " ";
			depth--;
		}
		return str;
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

	int Debug_Guard(Class dbg, int depth, int index)
	{
		Class.CastTo(m_Debug, dbg);
		m_Depth = depth;
		m_Index = index;

		Debug_Set("Source", GetSource());
		Debug_Set("Destination", GetDestination());
		int ret = Guard();
		switch (ret)
		{
			case FAIL:
				Debug_Set("Status", "FAIL");
				break;
			case SUCCESS:
				Debug_Set("Status", "SUCCESS");
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

	int Debug_Guard(Class dbg, int depth, int index)
	{
		return Guard();
	}
	#endif
};