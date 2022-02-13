modded class MissionServer
{
	void MissionServer()
	{
        MakeDirectory("$profile:eAI/");
        MakeDirectory("$profile:eAI/Loadout/");

		GetDayZGame().eAICreateManager();

		CF_Log.Info("eAI - Loaded Server Mission");
	}

	override void OnUpdate(float timeslice)
	{
		super.OnUpdate(timeslice);
	}

	override void InvokeOnConnect(PlayerBase player, PlayerIdentity identity)
	{
		super.InvokeOnConnect(player, identity);

		GetDayZGame().eAIManagerGet().InvokeOnConnect(player, identity);
	}
};