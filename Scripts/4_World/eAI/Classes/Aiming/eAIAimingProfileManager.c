class eAIAimingProfileManager
{
	private autoptr array<eAIBase> m_AIs = new array<eAIBase>(); 

	private float m_Time;

	void eAIAimingProfileManager()
	{
		GetRPCManager().AddRPC("eAIAimingProfileManager", "OnStart", this, SingeplayerExecutionType.Client);
		GetRPCManager().AddRPC("eAIAimingProfileManager", "OnEnd", this, SingeplayerExecutionType.Client);

		GetRPCManager().AddRPC("eAIAimingProfileManager", "OnSync", this, SingeplayerExecutionType.Server);
	}

	void AddAI(eAIBase ai)
	{
		if (GetGame().IsServer()) return;

		int idx = m_AIs.Find(ai);
		if (idx == -1) m_AIs.Insert(ai);

		//Print("Started arbitrating for " + ai);

		ai.CreateAimingProfile();
	}

	void RemoveAI(eAIBase ai)
	{
		if (GetGame().IsServer()) return;

		int idx = m_AIs.Find(ai);
		if (idx != -1) m_AIs.RemoveOrdered(idx);

		//Print("Stopped arbitrating for " + ai);

		ai.DestroyAimingProfile();
	}

	void Update(float pDt)
	{
		m_Time += pDt;
		//if (m_Time < 0.5) return;

		m_Time = 0;

		for (int i = m_AIs.Count() - 1; i >= 0; i--)
		{
			if (!m_AIs[i]) // if the AI is null, prematurely remove it
			{
				m_AIs.RemoveOrdered(i);
				continue;
			}

			eAIAimingProfile profile = m_AIs[i].GetAimingProfile();

			profile.Update();
			profile.SyncToServer();
		}
	}

	void OnStart(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		eAIBase ai;
		if (!Class.CastTo(ai, target)) return;

		if (GetGame().IsServer()) return;

		AddAI(ai);
	}

	void OnEnd(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		eAIBase ai;
		if (!Class.CastTo(ai, target)) return;

		if (GetGame().IsServer()) return;

		RemoveAI(ai);
	}

	void OnSync(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		eAIBase ai;
		if (!Class.CastTo(ai, target)) return;

		if (GetGame().IsClient()) return;

		eAIAimingProfile profile = ai.GetAimingProfile();
		Man arbiter = profile.m_Arbiter;

		//TODO: figure out why 'sender' and 'arbiter' differ when there should only be 1 PlayerIdentity on the server...
		//if (!arbiter || arbiter && arbiter.GetIdentity() != sender) return;

		eAIAimingProfile_SyncParams param;
		if (!ctx.Read(param)) return;

		profile.m_LastUpdated = param.param1;
		if (param.param2) Class.CastTo(profile.m_Hands, GetGame().GetObjectByNetworkId(param.param3, param.param4));

		profile.m_Position = param.param5;
		profile.m_Direction = param.param6;
	}
};