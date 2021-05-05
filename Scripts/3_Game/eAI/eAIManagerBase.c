class eAIManagerBase
{
    void eAIManagerBase()
    {
        eAILogger.Init();

        // anything dependent on settings during init must be initialized first.
        eAISettings.Init();
    }

    void OnUpdate(bool doSim, float timeslice) {}
};