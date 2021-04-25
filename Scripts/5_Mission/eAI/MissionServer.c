modded class MissionServer
{
	static string HeadlessClientSteamID = "REDACTED (PUT STEAMID HERE)";

	void MissionServer()
	{
		GetDayZGame().eAICreateManager();

		Print("eAI - Loaded Server Mission");
	}

	override void OnUpdate(float timeslice)
	{
		super.OnUpdate(timeslice);
	}

	override void InvokeOnConnect(PlayerBase player, PlayerIdentity identity)
	{
		super.InvokeOnConnect(player, identity);

		if (identity && identity.GetId() == HeadlessClientSteamID)
		{
			eAIGlobal_HeadlessClient = player;
			foreach (eAIGroup g : eAIGroup.GROUPS)
			{
				for (int i = 0; i < g.Count(); i++)
				{
					eAIBase ai = g.GetMember(i);
					if (ai && ai.IsAI() && ai.IsAlive())
						GetRPCManager().SendRPC("eAI", "HCLinkObject", new Param1<PlayerBase>(ai), false, identity);
				}
			}
		}

		eAIGroup.GetGroupByLeader(player);
	}
};