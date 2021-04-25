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
			eAIGroup.OnHeadlessClientConnect(identity);
		}

		eAIGroup.GetGroupByLeader(player);
	}
};