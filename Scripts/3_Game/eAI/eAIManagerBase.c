class eAIManagerBase
{
    private bool m_IsAdmin;

    void eAIManagerBase()
    {
        eAILogger.Init();

        // anything dependent on settings during init must be initialized first.
        eAISettings.Init();

        SetAdmin(false);
		GetRPCManager().AddRPC("eAI", "RPC_SetAdmin", this, SingeplayerExecutionType.Client);
    }

    bool IsAdmin()
    {
        return m_IsAdmin;
    }

    void SetAdmin(bool admin)
    {
        m_IsAdmin = admin;
    }
    
    void RPC_SetAdmin(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		//eAITrace trace(this, "RPC_SetAdmin");

		Param1<bool> data;
        if (!ctx.Read(data)) return;
		
		if (GetGame().IsServer()) return;

        SetAdmin(data.param1);
	}

    void OnUpdate(bool doSim, float timeslice) {}

	void InvokeOnConnect(DayZPlayer player, PlayerIdentity identity)
	{
		//eAITrace trace(this, "InvokeOnConnect");

        string guid = identity.GetPlainId();
        int idx = eAISettings.GetAdmins().Find(guid);

        GetRPCManager().SendRPC("eAI", "RPC_SetAdmin", new Param1<bool>(idx != -1), true, identity);
    }
};