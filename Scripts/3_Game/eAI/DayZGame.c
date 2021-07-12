PlayerIdentity eAIGlobal_HeadlessClient = null;

modded class DayZGame
{
    private ref eAIManagerBase m_eAI_Manager;
	private ref eAICommandManager m_eAI_CommandManager;
	
	void DayZGame() {
		m_eAI_CommandManager = new eAICommandManagerClient();
	}

    void eAICreateManager()
    {
        // fancy way to spawn the manager from a submodule
        m_eAI_Manager = eAIManagerBase.Cast("eAIManager".ToType().Spawn());
    }

    eAIManagerBase eAIManagerGet()
    {
        return m_eAI_Manager;
    }
    
    override void OnUpdate(bool doSim, float timeslice)
    {
        super.OnUpdate(doSim, timeslice);
		
		if (m_eAI_Manager)
			m_eAI_Manager.OnUpdate(doSim, timeslice);
    }
	
	eAICommandManager GetEAICommandManager() {
		return m_eAI_CommandManager;
	}
	
	override void GlobalsInit()
    {
        super.GlobalsInit();

		string path = "cfgVehicles";
		string child_name = "";
		Print("eAI: Filtering " + m_CharClassNames.Count() + " classnames");
        for (int i = m_CharClassNames.Count() - 1; i >= 0; i--)
		{
            child_name = m_CharClassNames[i];
			Print("eAI: Reading config class " + child_name);
			if (child_name.Contains("eAI"))
            {
			    Print("eAI: Purged config class " + child_name);
				m_CharClassNames.RemoveOrdered(i);
            }
		}
    }
};