class eAIAimingProfileManager
{
	private autoptr array<eAIBase> m_AIs = new array<eAIBase>(); 

	void eAIAimingProfileManager()
	{
		GetRPCManager().AddRPC("eAIAimingProfileManager", "OnStart", this, SingeplayerExecutionType.Client);
		GetRPCManager().AddRPC("eAIAimingProfileManager", "OnEnd", this, SingeplayerExecutionType.Client);

		GetRPCManager().AddRPC("eAIAimingProfileManager", "OnSync", this, SingeplayerExecutionType.Server);
	}

	void AddAI(eAIBase ai)
	{
		int idx = m_AIs.Find(ai);
		if (idx == -1) m_AIs.Insert(ai);

		ai.CreateAimingProfile();
	}

	void RemoveAI(eAIBase ai)
	{
		int idx = m_AIs.Find(ai);
		if (idx != -1) m_AIs.RemoveOrdered(idx);

		ai.DestroyAimingProfile();
	}

	void Update()
	{
		for (int i = m_AIs.Count() - 1; i >= 0; i--)
		{
			if (!m_AIs[i]) // if the AI is null, prematurely remove it
			{
				m_AIs.RemoveOrdered(i);
				continue;
			}

			m_AIs[i].GetAimingProfile().Update();
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

		if (!GetGame().IsServer()) return;

		ai.GetAimingProfile().Deserialize_Params(ctx);
	}
};