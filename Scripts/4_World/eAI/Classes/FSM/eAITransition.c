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
};