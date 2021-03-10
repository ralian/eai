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
			
			if (CanFire() && e.GetEventID() == WeaponEventID.TRIGGER) {

				
					
				
				// Get ballistics info
				float ammoDamage;
				string ammoTypeName;
				GetCartridgeInfo(GetCurrentMuzzle(), ammoDamage, ammoTypeName);
				
				Print("AI ROUND FIRED, BEGIN DEBUG INFO FOR UNIT: " + e.m_player);
				Print("AmmoType: " + ammoTypeName + " ammoDamage: " + ammoDamage.ToString());
				
				// Get Position Info
				// This is the same code in GetProjectedCursorPos3d(), but we need to do our own raycast logic
				vector usti_hlavne_position = GetSelectionPositionMS( "usti hlavne" );
				vector konec_hlavne_position = GetSelectionPositionMS( "konec hlavne" );
				
				
				
				//usti_hlavne_position = GetParent().VectorToParent(VectorToParent(usti_hlavne_position));
				//konec_hlavne_position = VectorToParent(konec_hlavne_position);
				
				//HumanItemAccessor hia = e.m_player.GetItemAccessor();
				vector aimingMatTM[4];
				//hia.WeaponGetAimingModelDirTm(aimingMatTM);
				
				float quatHeadTrans[4];
				int idx = e.m_player.GetBoneIndexByName("Head");
				if (idx < 0)
					Error("I've lost my darn head!");
				e.m_player.GetBoneRotationWS(idx, quatHeadTrans);
				vector headTrans = Math3D.QuatToAngles(quatHeadTrans); //despite what it says in the doc, this goes <Yaw, Roll, Pitch> with Pitch measured from the +Y axis
				
				vector weaponPos = e.m_player.GetBonePositionWS(idx);
				vector begin_point = weaponPos;
				vector aim_point;
				aim_point[1] = Math.Sin(DayZPlayerImplement.Cast(e.m_player).GetAimingModel().getAimY() * Math.DEG2RAD);
				aim_point[0] = -Math.Cos(headTrans[0] * Math.DEG2RAD);
				aim_point[2] = Math.Sin(headTrans[0] * Math.DEG2RAD);
				vector end_point = (500*aim_point) + begin_point;
				
				// Use these to get  an idea of the direction for the raycast
				//GetRPCManager().SendRPC("eAI", "DebugParticle", new Param2<vector, vector>(begin_point, vector.Zero));
				//GetRPCManager().SendRPC("eAI", "DebugParticle", new Param2<vector, vector>((begin_point+aim_point), vector.Zero));
				
				Print("Muzzle pos: " + begin_point.ToString() + " dir-pos: " + (end_point-begin_point).ToString());
				
				// Prep Raycast
				Object hitObject;
				vector hitPosition, hitNormal;
				float hitFraction;
				int contact_component = 0;
				DayZPhysics.RayCastBullet(begin_point, end_point, hit_mask, this, hitObject, hitPosition, hitNormal, hitFraction);
				//DayZPhysics.RaycastRV(begin_point, aim_point, hitPosition, hitNormal, contact_component, null, null, null, false, false, ObjIntersectFire);
				
				GetRPCManager().SendRPC("eAI", "DebugParticle", new Param2<vector, vector>(hitPosition, vector.Zero));
				
				Print("Raycast hitObject: " + hitObject.ToString() + " hitPosition-pos: " + (hitPosition-begin_point).ToString() + " hitNormal: " + hitNormal.ToString() + " hitFraction " + hitFraction.ToString());
				
				//if (hitPosition && hitNormal) {
				//	Particle p = Particle.PlayInWorld(ParticleList.DEBUG_DOT, hitPosition);
				//	p.SetOrientation(hitNormal);
					
				//}
				
				// So here is an interesting bug... hitObject is always still null even if the raycast succeeded
				// If it succeded then hitPosition, hitNormal, and hitFraction will be accurate
				if (hitFraction > 0.00001) {
					// this could be useful in the future
					//ref array<Object> nearest_objects = new array<Object>;
					//ref array<CargoBase> proxy_cargos = new array<CargoBase>;
					//GetGame().GetObjectsAtPosition ( hitPosition, 1.0, nearest_objects, proxy_cargos );
					
									
					array<Object> objects = new array<Object>();
					ref array<CargoBase> proxyCargos = new array<CargoBase>();
					Object closest = null;
					float dist = 1000000.0;
					float testDist;
					GetGame().GetObjectsAtPosition3D(hitPosition, 1.5, objects, proxyCargos);
					for (int i = 0; i < objects.Count(); i++) {
						if (DayZInfected.Cast(objects[i]) || Man.Cast(objects[i])) {
							testDist = vector.Distance(objects[i].GetPosition(), hitPosition);
							if (testDist < dist) {
								closest = objects[i];
								dist = testDist;
							}
						}
					}
					if (closest)
						closest.ProcessDirectDamage(DT_FIRE_ARM, e.m_player, "Torso", ammoTypeName, closest.WorldToModel(hitPosition), 1.0);
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