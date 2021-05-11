modded class MissionGameplay
{
	UAInput m_eAIRadialKey;

	void MissionGameplay()
	{
		m_eAIRadialKey = GetUApi().GetInputByName("eAICommandMenu");

		GetDayZGame().eAICreateManager();

		eAILogger.Info("eAI - Loaded Client Mission");
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