Man eAIGlobal_HeadlessClient = null;

modded class DayZGame
{
    private ref eAIManagerBase m_eAI_Manager;

    void eAICreateManager()
    {
        // fancy way to spawn the manager from a submodule
        Class.CastTo(m_eAI_Manager, "eAIManager".ToType().Spawn());
    }

    eAIManagerBase eAIManagerGet()
    {
		//auto trace = CF_Trace_0(this, "eAIManagerGet");
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
};