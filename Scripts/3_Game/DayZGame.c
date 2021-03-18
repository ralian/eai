modded class DayZGame
{
    private ref eAIManagerBase m_eAI_Manager;

    void eAICreateManager()
    {
        // fancy way to spawn the manager from a submodule
        m_eAI_Manager = eAIManagerBase.Cast("eAIManager".ToType().Spawn());
    }

    ref eAIManagerBase eAIManagerGet()
    {
        return m_eAI_Manager;
    }
    
    override void OnUpdate(bool doSim, float timeslice)
    {
        super.OnUpdate(doSim, timeslice);

        m_eAI_Manager.OnUpdate(doSim, timeslice);
    }
};