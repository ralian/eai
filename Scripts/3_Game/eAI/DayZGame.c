Man eAIGlobal_HeadlessClient = null;

modded class DayZGame
{
    private ref eAIManagerBase m_eAI_Manager;
	private ref eAICommandManager m_eAI_CommandManager;
	
	void DayZGame()
    {
		m_eAI_CommandManager = new eAICommandManagerClient();
	}

    void eAICreateManager()
    {
        // fancy way to spawn the manager from a submodule
        Class.CastTo(m_eAI_Manager, "eAIManager".ToType().Spawn());
    }

    eAIManagerBase eAIManagerGet()
    {
		//eAITrace trace(this, "eAIManagerGet");
        return m_eAI_Manager;
    }

    override void GlobalsInit()
    {
        super.GlobalsInit();

		string path = "cfgVehicles";
		string child_name = "";
        for (int i = m_CharClassNames.Count() - 1; i >= 0; i--)
		{
            child_name = m_CharClassNames[i];

			if (child_name.Contains("eAI"))
            {
			    m_CharClassNames.RemoveOrdered(i);
            }
		}
    }
    
    override void OnUpdate(bool doSim, float timeslice)
    {
        super.OnUpdate(doSim, timeslice);
		
		if (m_eAI_Manager)
			m_eAI_Manager.OnUpdate(doSim, timeslice);
    }
	
    //TODO: move to 5_Mission/eAIManager
	eAICommandManager GetEAICommandManager()
    {
		//eAITrace trace(this, "GetEAICommandManager");
		return m_eAI_CommandManager;
	}
};