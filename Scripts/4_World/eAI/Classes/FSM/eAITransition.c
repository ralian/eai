class eAITransition
{
    static const int FAIL = 0;
    static const int SUCCESS = 1; 

    protected string m_ClassName;
	
	protected eAIFSM m_FSM;

    /* STATE VARIABLES */
    protected eAIBase unit;

    void eAITransition(eAIFSM _fsm, eAIBase _unit)
    {
        unit = _unit;
		m_FSM = _fsm;
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
};