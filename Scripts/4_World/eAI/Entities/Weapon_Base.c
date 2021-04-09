// Copyright 2021 William Bowers
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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
	
	if (!pb) {
		Print("DayZPlayerInventory_OnEventForRemoteWeaponAI Callback for null PlayerBase! I am giving up, some inventory will be out of sync!");
		return false;
	}
	
	if (!pb.GetDayZPlayerInventory()) {
		Error("DayZPlayerInventory_OnEventForRemoteWeaponAI Callback for " + pb + " has a broken inventory reference!");
		return false;
	}
	
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
	
	// This is the latest updated aiming data for the weapon, on the server
	autoptr AimProfile aim = new AimProfile("0 0 0", "0 0 0");
	
	// For raycasting bullets in the navmesh
	autoptr PGFilter pgFilter = new PGFilter();
	
	EntityAI HitCast(out vector hitPosition, bool debugParticle = false) {
		// Get geometry info
		
		if (GetGame().GetTime() - aim.lastUpdated > 250)
			Print("Warning! Using old data for ballistics for weapon " + this.ToString());
		
		vector begin_point = aim.out_front;
		/*vector back = aim.out_back;
		
		vector aim_point = begin_point - back;*/
		vector dir = Vector(aim.InterpolationAzmuith, aim.InterpolationInclination, 0).AnglesToVector();

		vector end_point = (500*dir) + begin_point;
		
		// Use these to get  an idea of the direction for the raycast
		if (debugParticle) {
			GetRPCManager().SendRPC("eAI", "DebugParticle", new Param2<vector, vector>(begin_point + dir, vector.Zero));
			GetRPCManager().SendRPC("eAI", "DebugParticle", new Param2<vector, vector>(begin_point, vector.Zero));
		}
		
		// TODO: If we use Azumith and Inclination , we will get the benefits from server side interpolation.
		
		Print("Muzzle pos: " + begin_point.ToString() + " dir-pos: " + (end_point-begin_point).ToString());
		
		// Prep Raycast
		Object hitObject;
		vector hitNormal;
		float hitFraction;
		int contact_component = 0;
		DayZPhysics.RayCastBullet(begin_point, end_point, hit_mask, this, hitObject, hitPosition, hitNormal, hitFraction);
		//DayZPhysics.RaycastRV(begin_point, aim_point, hitPosition, hitNormal, contact_component, null, null, null, false, false, ObjIntersectFire);
		
		if (debugParticle)
			GetRPCManager().SendRPC("eAI", "DebugParticle", new Param2<vector, vector>(hitPosition, vector.Zero));
		
		Print("Raycast hitObject: " + hitObject.ToString() + " hitPosition-pos: " + (hitPosition-begin_point).ToString() + " hitNormal: " + hitNormal.ToString() + " hitFraction " + hitFraction.ToString());
		
		// So here is an interesting bug... hitObject is always still null even if the raycast succeeded
		// If it succeded then hitPosition, hitNormal, and hitFraction will be accurate
		if (hitFraction > 0.00001) {	
												
			array<Object> objects = new array<Object>();
			autoptr array<CargoBase> proxyCargos = new array<CargoBase>();
			Object closest = null;
			float dist = 1000000.0;
			float testDist;
		
			GetGame().GetObjectsAtPosition3D(hitPosition, 1.5, objects, proxyCargos);
			
			Print(objects);
		
			// not necessary since the ai aren't shooting themselves anymore?
			/*for (int i = 0; i < objects.Count(); i++)
				if (objects[i] == ignore)
					objects.Remove(i);*/
		
			for (int j = 0; j < objects.Count(); j++) {
				if (DayZInfected.Cast(objects[j]) || Man.Cast(objects[j])) {
					testDist = vector.Distance(objects[j].GetPosition(), hitPosition);
					if (testDist < dist) {
						closest = objects[j];
						dist = testDist;
					}
				}
			}
		
			// BUG: hitGround is sometimes still false even when we hit the top of an object.
			
			// As a quick workaround, do a raycast 5cm down from hitPosition
			// If we hit something other than object within 5cm, then we know we have hit the ground and we should not damage "closest."
			vector groundCheckDelta = hitPosition + "0 -0.05 0";
			vector groundCheckContactPos, groundCheckContactDir;
			int contactComponent;
		
			
			int allowFlags = 0;
			allowFlags |= PGPolyFlags.ALL;
			allowFlags |= PGPolyFlags.WALK;
			pgFilter.SetFlags(allowFlags, 0, 0);
			bool hitAnObject = GetGame().GetWorld().GetAIWorld().RaycastNavMesh(hitPosition, groundCheckDelta, pgFilter, groundCheckContactPos, groundCheckContactDir);
			GetRPCManager().SendRPC("eAI", "DebugParticle", new Param2<vector, vector>(groundCheckContactPos, vector.Zero));

			//DayZPhysics.RaycastRV(hitPosition, groundCheckDelta, groundCheckContactPos, groundCheckContactDir, contactComponent, null, null, closest);
			//bool hitGround = (vector.Distance(groundCheckDelta, groundCheckContactPos) > 0.01);
			
			Print("hitEnemy = " + closest.ToString());
			Print("Did we hit an inanimate object? = " + hitAnObject.ToString());
			//Print("hitGround = " + hitGround.ToString());
			
			if (closest && !hitAnObject)// && !hitGround)
				return closest;
		}
		return null;
	}
	
	/**@fn	ProcessWeaponEvent
	 * @brief	weapon's fsm handling of events
	 * @NOTE: warning: ProcessWeaponEvent can be called only within DayZPlayer::HandleWeapons (or ::CommandHandler)
	 **/
	override bool ProcessWeaponEvent (WeaponEventBase e)
	{
		if (GetGame().IsServer() && PlayerBase.Cast(e.m_player).IsAI()) {
			// Write the ctx that would normally be sent to the server... note we need to skip writing INPUT_UDT_WEAPON_REMOTE_EVENT
			// since this would normally be Read() and stripped away by the server before calling OnEventForRemoteWeapon
			
			// also wait, I think everyone is meant to execute this
			ScriptRemoteInputUserData ctx = new ScriptRemoteInputUserData;
			Print("Sending weapon event for " + e.GetEventID().ToString() + " player:" + e.m_player.ToString() + " mag:" + e.m_magazine.ToString());
			GetRPCManager().SendRPC("eAI", "DayZPlayerInventory_OnEventForRemoteWeaponAICallback", new Param3<int, DayZPlayer, Magazine>(e.GetPackedType(), e.m_player, e.m_magazine));
			
			// Now that the RPC is sent to the clients, we need to compute the ballistics data and see about a hit.
			// We miiight want to do this in another thread???
			if (CanFire() && e.GetEventID() == WeaponEventID.TRIGGER) {
				
				Print("Round fired by AI: " + e.m_player);
				
				// Get ballistics info
				float ammoDamage;
				string ammoTypeName;
				GetCartridgeInfo(GetCurrentMuzzle(), ammoDamage, ammoTypeName);
				
				// For now, we're just going to have a client handle this.
				// It will duplicate events if more than 1 person is on the server (not good)
				//array< PlayerIdentity > identities;
				//GetGame().GetPlayerIndentities(identities);
				PlayerBase p = PlayerBase.Cast(e.m_player);
				
				vector hitPos;
				EntityAI hitObject = HitCast(hitPos, true);
				if (hitObject)
					hitObject.ProcessDirectDamage(DT_FIRE_ARM, this, "Torso", ammoTypeName, hitObject.WorldToModel(hitPos), 1.0);
			}
			
			if (m_fsm.ProcessEvent(e) == ProcessEventResult.FSM_OK)
				return true;
				
			return false;
		
		// The rest of this is going to be client code. Also, for AI, clients should not sync events they receive to remote.
		} else if (!PlayerBase.Cast(e.m_player).IsAI())
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
		if (GetGame().IsServer() && PlayerBase.Cast(e.m_player).IsAI()) { // This is very hacky... we need to check if the unit is also AI
			if (m_fsm.ProcessEvent(e) == ProcessEventResult.FSM_OK)
				return true;

			return false;
		} else if (!PlayerBase.Cast(e.m_player).IsAI())
			SyncEventToRemote(e);
		
		ProcessEventResult aa;
		m_fsm.ProcessAbortEvent(e, aa);
		return aa == ProcessEventResult.FSM_OK;
	}
}