WeaponEventBase CreateWeaponEventFromContextAI (int packedType, DayZPlayer player, Magazine magazine)
{
	WeaponEventID eventID = packedType >> 16;
	WeaponEvents animEvent = packedType & 0x0000ffff;
	WeaponEventBase b = WeaponEventFactory(eventID, animEvent, player, magazine);
	//b.ReadFromContext(ctx); // currently does nothing in the bI code anyways
	return b;
}

bool DayZPlayerInventory_OnEventForRemoteWeaponAI (int packedType, DayZPlayer player, Magazine magazine)
{
	PlayerBase pb = PlayerBase.Cast(player);
	if (pb.GetDayZPlayerInventory().GetEntityInHands())
	{
		Weapon_Base wpn = Weapon_Base.Cast(pb.GetDayZPlayerInventory().GetEntityInHands());
		if (wpn)
		{
			

			WeaponEventBase e = CreateWeaponEventFromContextAI(packedType, player, magazine);
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
			Error("OnEventForRemoteWeapon - entity in hands, but not weapon. item=" + pb.GetDayZPlayerInventory().GetEntityInHands());
	}
	else
		Error("OnEventForRemoteWeapon - no entity in hands");
	return true;
}

modded class Weapon_Base {
	override bool LiftWeaponCheck (PlayerBase player)
	{
		int idx;
		float distance;
		float hit_fraction; //junk
		vector start, end;
		vector direction;
		vector usti_hlavne_position;
		vector trigger_axis_position;
		vector hit_pos, hit_normal; //junk
		Object obj;
		ItemBase attachment;
		
		m_LiftWeapon = false;
		// not a gun, no weap.raise for now
		if ( HasSelection("Usti hlavne") )
			return false;
		
		if (!player)
		{
			Print("Error: No weapon owner, returning");
			return false;
		}
		
		// add a separate check here since AI cannot use that part of the HIC
		if ( !player.isAI() && player.GetInputController() && !player.GetInputController().IsWeaponRaised() )
			return false;
		
		// if the AI is not targeting, we don't want a raise anyways
		if ( player.isAI() && !player.AIWeaponRaise() )
			return false;
		
		usti_hlavne_position = GetSelectionPositionLS( "Usti hlavne" ); 	// Usti hlavne
		trigger_axis_position = GetSelectionPositionLS("trigger_axis");
		
		// freelook raycast
		if (player.GetInputController().CameraIsFreeLook())
		{
			if (player.m_DirectionToCursor != vector.Zero)
			{
				direction = player.m_DirectionToCursor;
			}
			// if player raises weapon in freelook
			else
			{
				direction = MiscGameplayFunctions.GetHeadingVector(player);
			}
		}
		else
		{
			direction = GetGame().GetCurrentCameraDirection(); // exception for freelook. Much better this way!
		}
		
		idx = player.GetBoneIndexByName("Neck"); //RightHandIndex1
		if ( idx == -1 )
			{ start = player.GetPosition()[1] + 1.5; }
		else
			{ start = player.GetBonePositionWS(idx); }
		
		//! snippet below measures distance from "RightHandIndex1" bone for lifting calibration
		/*usti_hlavne_position = ModelToWorld(usti_hlavne_position);
		distance = vector.Distance(start,usti_hlavne_position);*/
		distance = m_WeaponLength;// - 0.05; //adjusted raycast length

		// if weapon has battel attachment, does longer cast
		if (ItemBase.CastTo(attachment,FindAttachmentBySlotName("weaponBayonet")) || ItemBase.CastTo(attachment,FindAttachmentBySlotName("weaponBayonetAK")) || ItemBase.CastTo(attachment,FindAttachmentBySlotName("weaponBayonetMosin")) || ItemBase.CastTo(attachment,FindAttachmentBySlotName("weaponBayonetSKS")) || ItemBase.CastTo(attachment,GetAttachedSuppressor()))
		{
			distance += attachment.m_ItemModelLength;
		}
		end = start + (direction * distance);
		DayZPhysics.RayCastBullet(start, end, hit_mask, player, obj, hit_pos, hit_normal, hit_fraction);
		
		// something is hit
		if (hit_pos != vector.Zero)
		{
			//Print(distance);
			m_LiftWeapon = true;
			return true;
		}
		return false;
	}
	
	
	/**@fn	ProcessWeaponEvent
	 * @brief	weapon's fsm handling of events
	 * @NOTE: warning: ProcessWeaponEvent can be called only within DayZPlayer::HandleWeapons (or ::CommandHandler)
	 **/
	override bool ProcessWeaponEvent (WeaponEventBase e)
	{
		if (GetGame().IsServer() && PlayerBase.Cast(e.m_player).isAI()) {
			// Write the ctx that would normally be sent to the server... note we need to skip writing INPUT_UDT_WEAPON_REMOTE_EVENT
			// since this would normally be Read() and stripped away by the server before calling OnEventForRemoteWeapon
			
			// also wait, I think everyone is meant to execute this
			ScriptRemoteInputUserData ctx = new ScriptRemoteInputUserData;
			Print("Sending weapon event for " + e.GetPackedType().ToString() + " player:" + e.m_player.ToString() + " mag:" + e.m_magazine.ToString());
			GetRPCManager().SendRPC("eAI", "DayZPlayerInventory_OnEventForRemoteWeaponAICallback", new Param3<int, DayZPlayer, Magazine>(e.GetPackedType(), e.m_player, e.m_magazine));
			if (m_fsm.ProcessEvent(e) == ProcessEventResult.FSM_OK)
				return true;
			
			/*if (e.GetEventID() == WeaponEventID.TRIGGER) {
				vector pos, dir;
				GetCameraPoint(GetCurrentMuzzle(), pos, dir);
				vector speed;
				for (int i = 0; i < 3; i++)
					speed[i] = dir[i]*910;
				Fire(GetCurrentMuzzle(), pos, dir, speed);
				//TryFireWeapon(this, GetCurrentMuzzle());
			}*/
				
			return false;
		} else if (!PlayerBase.Cast(e.m_player).isAI())
			SyncEventToRemote(e);
		
		// @NOTE: synchronous events not handled by fsm
		if (e.GetEventID() == WeaponEventID.SET_NEXT_MUZZLE_MODE)
		{
			SetNextMuzzleMode(GetCurrentMuzzle());
			return true;
		}

		if (m_fsm.ProcessEvent(e) == ProcessEventResult.FSM_OK)
			return true;

		//wpnDebugPrint("FSM refused to process event (no transition): src=" + GetCurrentState().ToString() + " event=" + e.ToString());
		return false;
	}
	/**@fn	ProcessWeaponAbortEvent
	 * @NOTE: warning: ProcessWeaponEvent can be called only within DayZPlayer::HandleWeapons (or ::CommandHandler)
	 **/
	override bool ProcessWeaponAbortEvent (WeaponEventBase e)
	{
		if (GetGame().IsServer() && PlayerBase.Cast(e.m_player).isAI()) { // This is very hacky... we need to check if the unit is also AI
			// Write the ctx that would normally be sent to the server... note we need to skip writing INPUT_UDT_WEAPON_REMOTE_EVENT
			// since this would normally be Read() and stripped away by the server before calling OnEventForRemoteWeapon
			ScriptRemoteInputUserData ctx = new ScriptRemoteInputUserData;
			GetRPCManager().SendRPC("eAI", "ToggleWeaponRaise", new Param3<int, DayZPlayer, Magazine>(e.GetPackedType(), e.m_player, e.m_magazine)); // This might need a separate RPC?
			if (m_fsm.ProcessEvent(e) == ProcessEventResult.FSM_OK)
				return true;
			return false;
		} else if (!PlayerBase.Cast(e.m_player).isAI())
			SyncEventToRemote(e);
		
		ProcessEventResult aa;
		m_fsm.ProcessAbortEvent(e, aa);
		return aa == ProcessEventResult.FSM_OK;
	}
}