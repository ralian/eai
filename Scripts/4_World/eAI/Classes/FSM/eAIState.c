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
};
