modded class WeaponFireToJam
{
	override void OnEntry (WeaponEventBase e)
	{
		eAIBase ai;
		if (e && Class.CastTo(ai, e.m_player))
		{
			m_dtAccumulator = 0;

			int mi = m_weapon.GetCurrentMuzzle();
			if (TryFireWeapon(m_weapon, mi))
			{
				eAI_Fire_OnEntry(e);

				m_weapon.SetJammed(true);
				DayZPlayerImplement p;
				if (Class.CastTo(p, e.m_player))
					p.GetAimingModel().SetRecoil(m_weapon);
			}
			
			WeaponStartAction_OnEntry(e);
			return;
		}

		super.OnEntry(e);
	}
};