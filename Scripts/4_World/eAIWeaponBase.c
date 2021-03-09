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
			Print("Sending weapon event for " + e.GetEventID().ToString() + " player:" + e.m_player.ToString() + " mag:" + e.m_magazine.ToString());
			GetRPCManager().SendRPC("eAI", "DayZPlayerInventory_OnEventForRemoteWeaponAICallback", new Param3<int, DayZPlayer, Magazine>(e.GetPackedType(), e.m_player, e.m_magazine));
			
			// Now that the RPC is sent to the clients, we need to compute the ballistics data and see about a hit.
			// We miiight want to do this in another thread???
			
			if (e.GetEventID() == WeaponEventID.TRIGGER) {

				
					
				
				// Get ballistics info
				float ammoDamage;
				string ammoTypeName;
				GetCartridgeInfo(GetCurrentMuzzle(), ammoDamage, ammoTypeName);
				
				Print("AI ROUND FIRED, BEGIN DEBUG INFO FOR UNIT: " + e.m_player);
				Print("AmmoType: " + ammoTypeName + " ammoDamage: " + ammoDamage.ToString());
				
				// Get Position Info
				// This is the same code in GetProjectedCursorPos3d(), but we need to do our own raycast logic
				vector usti_hlavne_position = GetSelectionPositionWS( "usti hlavne" );
				vector konec_hlavne_position = GetSelectionPositionWS( "konec hlavne" );
				
				//usti_hlavne_position = VectorToParent(usti_hlavne_position);
				konec_hlavne_position = VectorToParent(konec_hlavne_position);
				
				//HumanItemAccessor hia = e.m_player.GetItemAccessor();
				vector aimingMatTM[4];
				//hia.WeaponGetAimingModelDirTm(aimingMatTM);
				
				vector begin_point = GetPosition() + usti_hlavne_position;
				vector end_point = GetPosition() + konec_hlavne_position;

				//vector end_point = e.m_player.ModelToWorld(VectorToParent(usti_hlavne_position));// + GetWorldPosition();//ModelToWorld(usti_hlavne_position);
				//vector begin_point = konec_hlavne_position + GetWorldPosition(); //ModelToWorld(konec_hlavne_position);
				vector contact_dir;
				
				GetRPCManager().SendRPC("eAI", "DebugParticle", new Param2<vector, vector>(end_point, vector.Zero));
				GetRPCManager().SendRPC("eAI", "DebugParticle", new Param2<vector, vector>(begin_point, vector.Zero));
				
				vector aim_point = end_point - begin_point;
				
				//aim_point[2] = temp;
				//aim_point[1] = -aim_point[1];
				aim_point = aim_point.Normalized() * 500; // 500 meters for now
				aim_point = aim_point + end_point;
				Print("Muzzle pos: " + begin_point.ToString() + " dir-pos: " + (aim_point-begin_point).ToString());
				
				// Prep Raycast
				Object hitObject;
				vector hitPosition, hitNormal;
				float hitFraction;
				int contact_component = 0;
				DayZPhysics.RayCastBullet(begin_point, aim_point, hit_mask, this, hitObject, hitPosition, hitNormal, hitFraction);
				//DayZPhysics.RaycastRV(begin_point, aim_point, hitPosition, hitNormal, contact_component, null, null, null, false, false, ObjIntersectFire);
				
				Print("Raycast hitObject: " + hitObject.ToString() + " hitPosition-pos: " + (hitPosition-begin_point).ToString() + " hitNormal: " + hitNormal.ToString() + " hitFraction " + hitFraction.ToString());
				
				//if (hitPosition && hitNormal) {
				//	Particle p = Particle.PlayInWorld(ParticleList.DEBUG_DOT, hitPosition);
				//	p.SetOrientation(hitNormal);
					
				//}
				
				// So here is an interesting bug... hitObject is always still null even if the raycast succeeded
				// If it succeded then hitPosition, hitNormal, and hitFraction will be accurate
				if (hitFraction > 0.01) {
					// this could be useful in the future
					//ref array<Object> nearest_objects = new array<Object>;
					//ref array<CargoBase> proxy_cargos = new array<CargoBase>;
					//GetGame().GetObjectsAtPosition ( hitPosition, 1.0, nearest_objects, proxy_cargos );
					
									
					array<Man> players = new array<Man>();
					GetGame().GetPlayers(players);
					//GetGame().Get
					for (int i = 0; i < players.Count(); i++) {
						// todo more sophistocated logic for hitting each part
						if (vector.Distance(players[i].GetPosition(), hitPosition) < 1.5) {
							Print("We have a winner! unit: " + players[i].ToString() + " position: " + players[i].GetPosition().ToString());
							// I have an idea here but no time to implement it. We could pick the hit part of the player randomly using the hitPos's height off
							// the ground... eg higher than 1.5m translates to "head", then "torso", then below 1m is "legs"
							// Of course, this will not work if the player is crouched or prone, or on something like a building
							players[i].ProcessDirectDamage(DT_FIRE_ARM, e.m_player, "Torso", ammoTypeName, "0 0 0", 1.0);
							break;
						}
					}
				}
				
				// Check if a player was hit
				// unused for now because of the aforementioned bug
				if (hitObject) {
					hitObject.ProcessDirectDamage(DT_FIRE_ARM, e.m_player, "Head", ammoTypeName, "0 0 0", 1.0);
				}
			}
			
			if (m_fsm.ProcessEvent(e) == ProcessEventResult.FSM_OK)
				return true;
				
			return false;
		
		// The rest of this is going to be client code. Also, for AI, clients should not sync events they receive to remote.
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