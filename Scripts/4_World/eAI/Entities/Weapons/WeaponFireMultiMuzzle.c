modded class WeaponFireMultiMuzzle
{
	override void OnEntry (WeaponEventBase e)
	{
		eAIBase ai;
		if (e && Class.CastTo(ai, e.m_player))
		{
			m_dtAccumulator = 0;
	
			wpnPrint("[wpnfsm] " + Object.GetDebugName(m_weapon) + " WeaponFire bang bang!");
			//m_weapon.Fire();
			int mi = m_weapon.GetCurrentMuzzle();
			int b = m_weapon.GetCurrentModeBurstSize(mi);
			if(b > 1 )
			{
				
				for (int i = 0; i < b; i++)
				{
					if (TryFireWeapon(m_weapon, i))
					{
						eAI_Fire_OnEntry(e);
						DayZPlayerImplement p1;
						if (Class.CastTo(p1, e.m_player))
						p1.GetAimingModel().SetRecoil(m_weapon);
						m_weapon.OnFire(i);
					}
				}
			}
			else
			{
				//int mi = m_weapon.GetCurrentMuzzle();
				if (TryFireWeapon(m_weapon, mi))
				{
					eAI_Fire_OnEntry(e);
					DayZPlayerImplement p;
					if (Class.CastTo(p, e.m_player))
						p.GetAimingModel().SetRecoil(m_weapon);
					m_weapon.OnFire(mi);
				}
			}
			if(mi >= m_weapon.GetMuzzleCount() - 1 )
				m_weapon.SetCurrentMuzzle(0);
			else
				m_weapon.SetCurrentMuzzle(mi + 1);
				
			WeaponStartAction_OnEntry(e);
			return;
		}

		super.OnEntry(e);
	}
};