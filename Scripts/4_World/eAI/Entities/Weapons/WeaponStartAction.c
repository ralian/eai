modded class WeaponStartAction
{
	void WeaponStartAction_OnEntry(WeaponEventBase e)
	{
		// WeaponStateBase::OnEntry
		if (HasFSM() && !m_fsm.IsRunning())
		{
			wpnDebugPrint("[wpnfsm] " + Object.GetDebugName(m_weapon) + " { " + this.Type().ToString() + "  Has Sub-FSM! Starting submachine...");
			m_fsm.Start(e);
		}
		else
			wpnDebugPrint("[wpnfsm] " + Object.GetDebugName(m_weapon) + " { " + this.Type().ToString());

		// WeaponStartAction::OnEntry
		// super call was here -> added above
		if (e)
		{
			if (e.m_player)
			{
				HumanCommandWeapons hcw = e.m_player.GetCommandModifier_Weapons();
				if (hcw)
				{
					hcw.StartAction(m_action, m_actionType);
		
					if (hcw.GetRunningAction() == m_action && hcw.GetRunningActionType() == m_actionType)
						wpnDebugPrint("HCW: playing A=" + typename.EnumToString(WeaponActions, m_action) + " AT=" + WeaponActionTypeToString(m_action, m_actionType) + " fini=" + hcw.IsActionFinished());
					else
						Error("HCW: NOT playing A=" + typename.EnumToString(WeaponActions, m_action) + " AT=" + WeaponActionTypeToString(m_action, m_actionType) + " fini=" + hcw.IsActionFinished());
				}
				else
					wpnDebugPrint("---: remote playing A=" + typename.EnumToString(WeaponActions, m_action) + " AT=" + WeaponActionTypeToString(m_action, m_actionType));
			}
			else
			{
				wpnDebugPrint("---: warning, no player wants to play A=" + typename.EnumToString(WeaponActions, m_action) + " AT=" + WeaponActionTypeToString(m_action, m_actionType));
			}
		}
	}

	void eAI_Fire_OnEntry(WeaponEventBase e)
	{
		float ammoDamage;
		string ammoTypeName;
		m_weapon.GetCartridgeInfo(m_weapon.GetCurrentMuzzle(), ammoDamage, ammoTypeName);
		
		vector hitPos;
		EntityAI hitObject = m_weapon.HitCast(hitPos);
		if (hitObject) hitObject.ProcessDirectDamage(DT_FIRE_ARM, m_weapon, "Torso", ammoTypeName, hitObject.WorldToModel(hitPos), 1.0);
	}
};