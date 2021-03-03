/*modded class WeaponAttachMagazine extends WeaponStateBase {
	override void OnEntry (WeaponEventBase e)
	{
		Print("WeaponAttachMagazine::OnEntry called");
		super.OnEntry(e);
	}
};

modded class WeaponAttachMagazine extends WeaponStateBase {
	override void OnEntry (WeaponEventBase e)
	{
		Print("WeaponAttachMagazine::OnEntry called for event " + e.ToString());
		super.OnEntry(e);
	}
};

modded class MagazineShow extends WeaponStateBase
{
	override void OnEntry (WeaponEventBase e)
	{
		Print("MagazineShow::OnEntry called for event " + e.ToString());
		super.OnEntry(e);
	}
};*/



/// creates weapon fsm events
/*override WeaponEventBase WeaponEventFactory (WeaponEventID id, int aetype, DayZPlayer p = NULL, Magazine m = NULL)
{
	Print("WeaponEventFactory called, id:" + id.ToString());
	switch (id)
	{
		case WeaponEventID.UNKNOWN: return null;
		case WeaponEventID.MECHANISM: return new WeaponEventMechanism(p, m);
		case WeaponEventID.TRIGGER: return new WeaponEventTrigger(p, m);
		case WeaponEventID.TRIGGER_JAM: return new WeaponEventTriggerToJam(p, m);
		case WeaponEventID.LOAD1_BULLET: return new WeaponEventLoad1Bullet(p, m);
		case WeaponEventID.CONTINUOUS_LOADBULLET_START: return new WeaponEventContinuousLoadBulletStart(p, m);
		case WeaponEventID.CONTINUOUS_LOADBULLET_END: return new WeaponEventContinuousLoadBulletEnd(p, m);
		case WeaponEventID.UNJAM: return new WeaponEventUnjam(p, m);
		case WeaponEventID.ATTACH_MAGAZINE: return new WeaponEventAttachMagazine(p, m);
		case WeaponEventID.UNJAMMING_FAILED_TIMEOUT: return new WeaponEventUnjammingFailedTimeout(p, m);
		case WeaponEventID.UNJAMMING_TIMEOUT: return new WeaponEventUnjammingTimeout(p, m);
		case WeaponEventID.DETACH_MAGAZINE: return new WeaponEventDetachMagazine(p, m);
		case WeaponEventID.SWAP_MAGAZINE: return new WeaponEventSwapMagazine(p, m);
		case WeaponEventID.HUMANCOMMAND_ACTION_FINISHED: return new WeaponEventHumanCommandActionFinished(p, m);
		case WeaponEventID.HUMANCOMMAND_ACTION_ABORTED: return new WeaponEventHumanCommandActionAborted(p, m);
		case WeaponEventID.RELOAD_TIMEOUT: return new WeaponEventReloadTimeout(p, m);
		case WeaponEventID.DRY_FIRE_TIMEOUT: return new WeaponEventDryFireTimeout(p, m);
		case WeaponEventID.SET_NEXT_MUZZLE_MODE: return new WeaponEventSetNextMuzzleMode(p, m);
		case WeaponEventID.ANIMATION_EVENT: return WeaponAnimEventFactory(aetype, p, m);
	}
	return NULL;
}

/// creates animation system events
override WeaponEventBase WeaponAnimEventFactory (WeaponEvents type, DayZPlayer p = NULL, Magazine m = NULL)
{
	Print("WeaponAnimEventFactory called, type:" + type.ToString());
	if (p && p.isAI() && !m) // if we are an AI but have no target mag
		m = p.GetMagazineToReload(weapon);
	switch (type)
	{
		case WeaponEvents.ATTACHMENT_HIDE: return new WeaponEventAnimAttachmentHide(p, m);
		case WeaponEvents.ATTACHMENT_SHOW: return new WeaponEventAnimAttachmentShow(p, m);
		case WeaponEvents.BULLET_EJECT: return new WeaponEventAnimBulletEject(p, m);
		case WeaponEvents.BULLET_HIDE: return new WeaponEventAnimBulletHide(p, m);
		case WeaponEvents.BULLET_HIDE2: return new WeaponEventAnimBulletHide2(p, m);
		case WeaponEvents.BULLET_IN_CHAMBER: return new WeaponEventAnimBulletInChamber(p, m);
		case WeaponEvents.BULLET_IN_MAGAZINE: return new WeaponEventAnimBulletInMagazine(p, m);
		case WeaponEvents.BULLET_SHOW: return new WeaponEventAnimBulletShow(p, m);
		case WeaponEvents.BULLET_SHOW2: return new WeaponEventAnimBulletShow2(p, m);
		case WeaponEvents.CANUNJAM_END: return new WeaponEventAnimCanUnjamEnd(p, m);
		case WeaponEvents.CANUNJAM_START: return new WeaponEventAnimCanUnjamStart(p, m);
		case WeaponEvents.COCKED: return new WeaponEventAnimCocked(p, m);
		case WeaponEvents.MAGAZINE_ATTACHED: return new WeaponEventAnimMagazineAttached(p, m);
		case WeaponEvents.MAGAZINE_DETACHED: return new WeaponEventAnimMagazineDetached(p, m);
		case WeaponEvents.MAGAZINE_HIDE: return new WeaponEventAnimMagazineHide(p, m);
		case WeaponEvents.MAGAZINE_SHOW: return new WeaponEventAnimMagazineShow(p, m);
		case WeaponEvents.SLIDER_OPEN: return new WeaponEventAnimSliderOpen(p, m);
		case WeaponEvents.UNJAMMED: return new WeaponEventAnimUnjammed(p, m);
		case WeaponEvents.HAMMER_UNCOCKED: return new WeaponEventAnimHammerUncocked(p, m);
		case WeaponEvents.HAMMER_COCKED: return new WeaponEventAnimHammerCocked(p, m);
		case WeaponEvents.CYLINDER_ROTATE: return new WeaponEventCylinderRotate(p, m);
		//case WeaponEvents.: return new WeaponEventAnim(p, m);
	}
	return NULL;
}*/

/*override WeaponEventBase CreateWeaponEventFromContext (ParamsReadContext ctx)
{
	int packedType;
	ctx.Read(packedType);
	DayZPlayer player;
	ctx.Read(player);
	Magazine magazine;
	ctx.Read(magazine);
	if (GetDayZPlayerOwner().isAI())
		magazine = player.GetMagazineToReload(weapon);
	
	Print("We got here!");

	WeaponEventID eventID = packedType >> 16;
	WeaponEvents animEvent = packedType & 0x0000ffff;
	WeaponEventBase b = WeaponEventFactory(eventID, animEvent, player, magazine);
	b.ReadFromContext(ctx);
	return b;
}*/


class eAIDayZInventory : DayZPlayerInventory {
	
	PlayerBase p;
	
	override DayZPlayer GetDayZPlayerOwner() {return DayZPlayer.Cast(p);}
	
	void eAIDayZInventory(PlayerBase player) {
		p = player;
	}
	
	override void PostWeaponEvent (WeaponEventBase e)
	{
		Print("Weapon Event posting...");
		if (m_DeferredWeaponEvent == NULL)
		{
			m_DeferredWeaponEvent = e;
			wpnDebugPrint("[wpnfsm] " + Object.GetDebugName(GetInventoryOwner()) + " Posted event m_DeferredWeaponEvent=" + m_DeferredWeaponEvent.DumpToString());
		}
		else
			Error("[wpnfsm] " + Object.GetDebugName(GetInventoryOwner()) + " warning - pending event already posted, curr_event=" + m_DeferredWeaponEvent.DumpToString() + " new_event=" + e.DumpToString());
	}

	override void HandleWeaponEvents (float dt, out bool exitIronSights)
	{
		Print("debug1");
		HumanCommandWeapons hcw = GetDayZPlayerOwner().GetCommandModifier_Weapons();

		Weapon_Base weapon;
		Class.CastTo(weapon, GetEntityInHands());
		
		if (hcw && weapon && weapon.CanProcessWeaponEvents())
		{
			weapon.GetCurrentState().OnUpdate(dt);

			wpnDebugSpamALot("[wpnfsm] " + Object.GetDebugName(weapon) + " HCW: playing A=" + typename.EnumToString(WeaponActions, hcw.GetRunningAction()) + " AT=" + WeaponActionTypeToString(hcw.GetRunningAction(), hcw.GetRunningActionType()) + " fini=" + hcw.IsActionFinished());

			if (!weapon.IsIdle())
			{
				while (true)
				{
					int weaponEventId = hcw.IsEvent();
					if (weaponEventId == -1)
					{
						break;
					}

					if (weaponEventId == WeaponEvents.CHANGE_HIDE)
					{
						break;
					}
					
					Print("We got here!3");
					
					WeaponEventBase anim_event;
					// This is the only addition to DayZPlayerInventory
					//if (GetDayZPlayerOwner().isAI())
						anim_event = WeaponAnimEventFactory(weaponEventId, GetDayZPlayerOwner(), PlayerBase.Cast(GetDayZPlayerOwner()).GetMagazineToReload(weapon));
					//else
						//anim_event = WeaponAnimEventFactory(weaponEventId, GetDayZPlayerOwner(), NULL);
					
					wpnDebugPrint("[wpnfsm] " + Object.GetDebugName(weapon) + " HandleWeapons: event arrived " + typename.EnumToString(WeaponEvents, weaponEventId) + "(" + weaponEventId + ")  fsm_ev=" + anim_event.ToString());
					if (anim_event != NULL)
					{
						weapon.ProcessWeaponEvent(anim_event);
					}
				}

				if (hcw.IsActionFinished())
				{
					if (weapon.IsWaitingForActionFinish())
					{
						wpnDebugPrint("[wpnfsm] " + Object.GetDebugName(weapon) + " Weapon event: finished! notifying waiting state=" + weapon.GetCurrentState());
						weapon.ProcessWeaponEvent(new WeaponEventHumanCommandActionFinished(GetDayZPlayerOwner()));
					}
					else
					{
						wpnDebugPrint("[wpnfsm] " + Object.GetDebugName(weapon) + " Weapon event: ABORT! notifying running state=" + weapon.GetCurrentState());
						weapon.ProcessWeaponAbortEvent(new WeaponEventHumanCommandActionAborted(GetDayZPlayerOwner()));
					}
				}
			}

			if (m_DeferredWeaponEvent)
			{
				wpnDebugPrint("[wpnfsm] " + Object.GetDebugName(weapon) + " Weapon event: deferred " + m_DeferredWeaponEvent.DumpToString());
				if (weapon.ProcessWeaponEvent(m_DeferredWeaponEvent))
				{
					exitIronSights = true;
					fsmDebugSpam("[wpnfsm] " + Object.GetDebugName(weapon) + " Weapon event: resetting deferred event" + m_DeferredWeaponEvent.DumpToString());
					m_DeferredWeaponEvent = NULL;
					m_DeferredWeaponTimer.Stop();
				}
				else if (!m_DeferredWeaponTimer.IsRunning())
				{
					m_DeferredWeaponTimer.Run(3, this, "DeferredWeaponFailed");
				}
			}
		}
	}
	
	override bool OnEventForRemoteWeapon (ParamsReadContext ctx)
	{
		Print("This should work... probably");
		if (GetEntityInHands())
		{
			Weapon_Base wpn = Weapon_Base.Cast(GetEntityInHands());
			if (wpn)
			{
				PlayerBase pb = PlayerBase.Cast(GetDayZPlayerOwner());

				WeaponEventBase e = CreateWeaponEventFromContext(ctx);
				if (pb && e)
				{
					pb.GetWeaponManager().SetRunning(true);
		
					fsmDebugSpam("[wpnfsm] " + Object.GetDebugName(wpn) + " recv event from remote: created event=" + e);
					if (e.GetEventID() == WeaponEventID.HUMANCOMMAND_ACTION_ABORTED)
					{
						wpn.ProcessWeaponAbortEvent(e);
					}
					else
					{
						wpn.ProcessWeaponEvent(e);
					}
					pb.GetWeaponManager().SetRunning(false);
				}
			}
			else
				Error("OnEventForRemoteWeapon - entity in hands, but not weapon. item=" + GetEntityInHands());
		}
		else
			Error("OnEventForRemoteWeapon - no entity in hands");
		return true;
	}
	
	override void HandleTakeToDst( DeferredEvent deferred_event )
	{
		Print("This should work... probably2");
		DeferredTakeToDst deferred_take_to_dst = DeferredTakeToDst.Cast(deferred_event);
		if( deferred_take_to_dst )
		{
			inventoryDebugPrint("[inv] I::Take2Dst(" + typename.EnumToString(InventoryMode, deferred_take_to_dst.m_mode) + ") src=" + InventoryLocation.DumpToStringNullSafe(deferred_take_to_dst.m_src) + " dst=" + InventoryLocation.DumpToStringNullSafe(deferred_take_to_dst.m_dst));

			switch (deferred_take_to_dst.m_mode)
			{
				case InventoryMode.PREDICTIVE:
					InventoryInputUserData.SendInputUserDataMove(InventoryCommandType.SYNC_MOVE, deferred_take_to_dst.m_src, deferred_take_to_dst.m_dst);
					ClearInventoryReservation(deferred_take_to_dst.m_dst.GetItem(),deferred_take_to_dst.m_dst);
					LocationSyncMoveEntity(deferred_take_to_dst.m_src, deferred_take_to_dst.m_dst);
					break;
				case InventoryMode.JUNCTURE:
					DayZPlayer player = GetGame().GetPlayer();
					player.GetHumanInventory().AddInventoryReservation(deferred_take_to_dst.m_dst.GetItem(), deferred_take_to_dst.m_dst, GameInventory.c_InventoryReservationTimeoutShortMS);
					EnableMovableOverride(deferred_take_to_dst.m_dst.GetItem());	
					InventoryInputUserData.SendInputUserDataMove(InventoryCommandType.SYNC_MOVE, deferred_take_to_dst.m_src, deferred_take_to_dst.m_dst);
					break;
				case InventoryMode.LOCAL:
					break;
				case InventoryMode.SERVER:
					break;

					break;
				default:
					Error("HandEvent - Invalid mode");
			}
		}
	}
};