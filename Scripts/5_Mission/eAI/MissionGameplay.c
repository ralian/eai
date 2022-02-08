modded class MissionGameplay
{
	UAInput m_eAIRadialKey;

	void MissionGameplay()
	{
		m_eAIRadialKey = GetUApi().GetInputByName("eAICommandMenu");

		GetDayZGame().eAICreateManager();

		CF_Log.Info("eAI - Loaded Client Mission");
	}

	override void OnMissionStart()
	{
		super.OnMissionStart();

		if (IsMissionOffline()) GetDayZGame().eAIManagerGet().SetAdmin(true);
	}

	override void OnMissionFinish()
	{
		super.OnMissionFinish();

		GetDayZGame().eAIManagerGet().SetAdmin(false);
	}

	override void OnUpdate(float timeslice)
	{
		super.OnUpdate(timeslice);

    	//TODO: move to 5_Mission/eAIManager

		DayZPlayerImplement player;
		Class.CastTo(player, GetGame().GetPlayer());

		// If we want to open the command menu, and nothing else is open
		if (m_eAIRadialKey.LocalPress() && !GetGame().GetUIManager().GetMenu())
		{
			if (GetDayZGame().eAIManagerGet().IsAdmin() || (player && player.GetGroup()))
			{
				if (!eAICommandMenu.instance) new eAICommandMenu();
				GetUIManager().ShowScriptedMenu(eAICommandMenu.instance, null);
			}
		}

		// If we want to close the command menu, and our menu is open
		if (m_eAIRadialKey.LocalRelease() && GetGame().GetUIManager().GetMenu() == eAICommandMenu.instance)
		{
			eAICommandMenu.instance.OnMenuRelease();
			GetUIManager().Back();
		}
	}
};