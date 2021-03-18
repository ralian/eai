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

enum eAIBehaviorGlobal { // Global states that dictate each unit's actions
	BRAINDEAD, 	// Do absolutely nothing. For optimization of units outside a certain radius (or unconcious, etc.)
	RELAXED, 	// Idle and watch surroundings loosely. Will leisurely walk to waypoint.
	ALERT, 		// Scan surroundings carefully. Will run to waypoint.
	PANIC, 		// Unused, temp state if shot or injured from unknown direction
	COMBAT 		// Will engage the threats in the threat list in order. Will ignore waypoints to seek out threats.
};

// Combat behavior flags... mostly unused at the moment
enum eAICombatPriority {
	ELIMINATE_TARGET,	// Kill the first target in the target array.
	FIND_COVER,			// Find cover before engaging
	COVERING_FIRE,		// Used to protect squadmates while they move
	IGNORE,				// Ignore the combat and move to waypoint
	RELOADING,
	OPPORTUNITY_SHOT	// The enemy does not see us (such as an infected) and we should only shoot if we have to.
};

enum eAIActionInterlock { // To be implemented later, so that only one anim runs at a time
	NONE,
	WEAPON_UPDOWN,
	RELOADING,
	FIRING
};

// This class is assigned to a single PlayerBase to manage its behavior.
class eAIPlayerHandler {
	PlayerBase unit;
	
	ref eAIDayZPlayerCamera cam;
	
	// Brain data
	ref array<Object> threats = new array<Object>();
	eAIBehaviorGlobal state;
	eAICombatPriority combatState;
	bool m_WantWeapRaise = false;
	
	// This is the bitmask used for pathfinding (i.e. whether we want a path that goes over a fence
	ref PGFilter pgFilter = new PGFilter();
	
	// Formation following logic
	DayZPlayer m_FollowOrders = null;
	vector formationOffset = "0 0 0";
	vector m_FormDir = "1 0 0";
	vector m_FormLastPos = "0 0 0";
	
	// Data for the heading computation
	float targetAngle, heading, delta;
	
	// Logic for waypoints
	ref TVectorArray waypoints = new TVectorArray(); // This is the list of waypoints the AI will follow. It is set by AIWorld.
	int cur_waypoint_no = -1; // Next point to move to in the waypoints array. -1 for "none"
	float arrival_radius = 2.0; // This is the distance that we consider a waypoint to be 'reached' at.
	float lastDistToWP = -1; // If it is set to -1, the AI will not try to sprint on the first movement update.
	
	void eAIPlayerHandler(notnull PlayerBase p) {
		if (!GetGame().IsServer())
			Error("AI may only be managed server side!");
				
		if (p.isAI())
			unit = p;
		else
			Error("Attempting to call eAIPlayerHandler() on " + p.ToString() + ", which is not marked as AI.");
		
		state = eAIBehaviorGlobal.RELAXED;
		combatState = eAICombatPriority.ELIMINATE_TARGET;
		
		cam = new eAIDayZPlayerCamera(unit, unit.GetInputController());
		cam.InitCameraOnPlayer(true); // force the camera active
		
		// Configure the pathfinding filter
		//pgFilter.SetCost(PGAreaType.NONE, 1.0f); // not sure what kind of behaivor this causes
		pgFilter.SetCost(PGAreaType.TERRAIN, 1.0);
		pgFilter.SetCost(PGAreaType.WATER, 15.0);
		pgFilter.SetCost(PGAreaType.WATER_DEEP, 15.0);
		pgFilter.SetCost(PGAreaType.WATER_SEA, 15.0);
		pgFilter.SetCost(PGAreaType.OBJECTS_NOFFCON, 1000000.0); // imma be real with you chief idk what this is
		pgFilter.SetCost(PGAreaType.OBJECTS, 20.0);
		pgFilter.SetCost(PGAreaType.BUILDING, 20.0);
		pgFilter.SetCost(PGAreaType.ROADWAY, 0.2);
		pgFilter.SetCost(PGAreaType.TREE, 15.0);
		pgFilter.SetCost(PGAreaType.ROADWAY_BUILDING, 20.0);
		pgFilter.SetCost(PGAreaType.DOOR_OPENED, 1.0);
		pgFilter.SetCost(PGAreaType.DOOR_CLOSED, 1000000.0); // not yet implemented :)
		pgFilter.SetCost(PGAreaType.LADDER, 1000000.0);
		pgFilter.SetCost(PGAreaType.CRAWL, 1000000.0);
		pgFilter.SetCost(PGAreaType.CROUCH, 1000000.0);
		pgFilter.SetCost(PGAreaType.FENCE_WALL, 1000000.0);
		pgFilter.SetCost(PGAreaType.JUMP, 1000000.0);
		
		int allowFlags = 0;
		allowFlags |= PGPolyFlags.WALK;
		allowFlags |= PGPolyFlags.CRAWL;
		allowFlags |= PGPolyFlags.CROUCH;
		
		int disallowFlags = 0;
		disallowFlags |= PGPolyFlags.JUMP_OVER;
		disallowFlags |= PGPolyFlags.JUMP_DOWN;
		disallowFlags |= PGPolyFlags.UNREACHABLE;
		disallowFlags |= PGPolyFlags.JUMP;
		
		pgFilter.SetFlags(allowFlags, disallowFlags, 0);
		
		// This is a separate PGFilter for raycasting what we can "See"
		// Todo move this to a global filter?
		int allowFlags2 = 0;
		allowFlags2 |= PGPolyFlags.ALL;
		allowFlags2 |= PGPolyFlags.WALK;
		occlusionPGFilter.SetFlags(allowFlags2, 0, 0);
	}
	
	/*void ~eAIPlayerHandler() {
		delete unit;
		delete cam;
		delete threats;
		delete pgFilter;
		delete waypoints;
	}*/
	
	// Set a player to follow.
	void Follow(DayZPlayer p, vector formationPos, float distance = 2.0) {
		formationOffset = formationPos;
		m_FollowOrders = p;
		arrival_radius = distance;
		if (m_FollowOrders.GetIdentity()) {
			// This is unsafe, we need to do it on weapon swap as well
			GetRPCManager().SendRPC("eAI", "eAIAimArbiterSetup", new Param1<Weapon_Base>(Weapon_Base.Cast(unit.GetHumanInventory().GetEntityInHands())), false, m_FollowOrders.GetIdentity());
		}
	}
	
	void FireHeldWeapon() {
		unit.GetWeaponManager().Fire(Weapon_Base.Cast(unit.GetHumanInventory().GetEntityInHands()));
		unit.GetInputController().OverrideAimChangeY(true, 0.0);
	}
	
	void EnableADSTracking() {
		if (m_WantWeapRaise) // double check that this hasn't been reset
			unit.eAI_Use_ADS_Tracking = true;
	}
	
	// todo fix the name of this
	void RaiseWeapon(bool up) {
		m_WantWeapRaise = up;
		unit.GetInputController().OverrideRaise(true, m_WantWeapRaise);
		HumanCommandMove cm = unit.GetCommand_Move();
		if (m_WantWeapRaise) {
			cm.ForceStance(DayZPlayerConstants.STANCEIDX_RAISEDERECT);
		} else {
			cm.ForceStance(DayZPlayerConstants.STANCEIDX_ERECT);
		}
		
		// Now, we need to kick off the client weapon aim arbitration.
		if (up) {
			if (m_FollowOrders.GetIdentity()) {
				// Start the client arbiter for this AI's weapon. The arbiter must already be init'ed
				GetRPCManager().SendRPC("eAI", "eAIAimArbiterStart", new Param2<Weapon_Base, int>(Weapon_Base.Cast(unit.GetHumanInventory().GetEntityInHands()), 250), false, m_FollowOrders.GetIdentity());
				// Use ADS instead of view for targeting, but we have to wait until our data from the client is valid
				GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.EnableADSTracking, 1000, false);
				Print(this.ToString() + " entering ADS");
			} 
		} else {
			GetRPCManager().SendRPC("eAI", "eAIAimArbiterStop", new Param1<Weapon_Base>(Weapon_Base.Cast(unit.GetHumanInventory().GetEntityInHands())), false, m_FollowOrders.GetIdentity());
			unit.eAI_Use_ADS_Tracking = false;
			Print(this.ToString() + " exiting ADS");
		}
	}
	
	void ToggleWeaponRaise() {
		RaiseWeapon(!m_WantWeapRaise);
	}
	
	// This returns true if the weapon should be raised.
	// I need another function to do this with a delay.
	bool WantsWeaponUp() {
		return m_WantWeapRaise;
	}
	
	//--------------------------------------------------------------------------------------------------------------------------
	// CODE AND HELPER FUNCS FOR PATHING
	//--------------------------------------------------------------------------------------------------------------------------
	
	//! Clear the array of waypoints, setting cur_waypoint_no to -1
	void clearWaypoints() {
		cur_waypoint_no = -1;
		waypoints.Clear(); // Now we need a clean array!
		lastDistToWP = -1; // If this is set to -1, the AI will not try to sprint on the first movement update.
	}
	
	//! Skip to the next waypoint (returns true, unless there is no future waypoint.)
	//! If there is not a next waypoint, returns false and clears the list of waypoints.
	bool nextWaypoint() {
		if (++cur_waypoint_no < waypoints.Count())
			return true;
		
		// executes if there is no future waypoint
		clearWaypoints();
		return false;
	}
	
	// Add a waypoint, return the index of the added wp
	int addWaypoint(vector pos) {
		waypoints.Insert(pos);
		return ++cur_waypoint_no;
	}
	
	// Update our heading and speed to the next waypoint; if we reach a waypoint, do more wacky logic.
	bool UpdateMovement() {
		bool needsToRunAgain = false;
		
		/*if (state == eAIBehaviorGlobal.COMBAT) {
			vector myPos = unit.GetPosition();
			vector threatPos = threats[0].GetPosition();
			vector aimPoint = "0 0 0";
			Weapon_Base weap = Weapon_Base.Cast(unit.GetHumanInventory().GetEntityInHands());
			
			if (weap && weap.whereIAmAimedAt != "0 0 0" && WantsWeaponUp()) {
				// Aiming logic based on where the barrel is aimed, won't work if the weapon is not yet up
				aimPoint = weap.whereIAmAimedAt - myPos;
				threatPos = threatPos - myPos;
				targetAngle = threatPos.VectorToAngles().GetRelAngles()[0];
				heading = aimPoint.VectorToAngles().GetRelAngles()[0];
				
				HasAShot = true;
				
			}
			
			delta = Math.DiffAngle(targetAngle, heading);
			
			// Do the logic for aiming along the x axis...
			// Todo make it so the x direction is calculated from barrell
			unit.GetInputController().OverrideMovementSpeed(true, 0.0);
			
			
			delta /= 500;
			delta = Math.Max(delta, -0.25);
			delta = Math.Min(delta, 0.25);
			unit.GetInputController().OverrideAimChangeX(true, delta);
			//unit.GetInputController().OverrideAimChangeY(true, 0.0);
			
			Print("UpdateMovement() - targetAngle: " + targetAngle + " heading: " + heading + " delta: " + delta);
			
			// Now, do the logic for aiming along the y axis.
			float gunHeight = 1.5 + myPos[1]; 			// Todo get the actual world gun height.
			float targetHeight = 1.0 + threatPos[1]; 	// Todo get actual threat height
			float aimAngle = Math.Atan2(targetHeight - gunHeight, vector.Distance(myPos, threatPos));
			unit.targetAngle = aimAngle * Math.RAD2DEG;
			
			// Make sure the AI is aimed at the target and not at anything else
			if (HasAShot) {
				array<Object> objects = new array<Object>();
				ref array<CargoBase> proxyCargos = new array<CargoBase>();
				
				bool aimedAtTheEnemy = false;
				
				// Here we use 2.5 meters instead of 1.5, to allow a small threshhold for AI to miss a shot
				GetGame().GetObjectsAtPosition3D(weap.whereIAmAimedAt, 2.5, objects, proxyCargos);
				
				for (int i = 0; i < objects.Count(); i++) {
					if (objects[i] == threats[0])
						aimedAtTheEnemy = true;
					if (objects[i] == m_FollowOrders) // quick and dirty check for shooting at a friendly
						HasAShot = false;
				}
				
				HasAShot = (HasAShot && aimedAtTheEnemy);
			}			
			
			return false;
		}*/
		
		if (m_WantWeapRaise) {
			
		}
		
		// First, if we don't have any valid waypoints, make sure to stop the AI and quit prior to the movement logic.
		// Otherwise, the AimChange would be undefined in this case (leading to some hilarious behavior including disappearing heads)
		if (cur_waypoint_no < 0) {
			unit.GetInputController().OverrideMovementSpeed(true, 0.0);
			unit.GetInputController().OverrideAimChangeX(true, 0.0);
			return false;
		}
		
		/*if (m_FollowOrders) {
			cur_waypoint_no = -1; // can't use clearWaypoints() here because we want to preserve the distance
			waypoints.Clear();
			addWaypoint(m_FollowOrders.GetPosition());
		};*/
		
		//unit.headingTarget = vector.Direction(GetPosition(), waypoints[cur_waypoint_no]).VectorToAngles().GetRelAngles()[0];
		
		/*targetAngle = vector.Direction(unit.GetPosition(), waypoints[cur_waypoint_no]).VectorToAngles().GetRelAngles()[0];// * Math.DEG2RAD;
		//vector heading = MiscGameplayFunctions.GetHeadingVector(this);
		
		// This seems to be CCW is positive unlike relative angles.
		// ALSO THIS ISN'T CAPPED AT +-180. I HAVE SEEN IT IN THE THOUSANDS.
		heading = -(unit.GetInputController().GetHeadingAngle() * Math.RAD2DEG); 

		
		//heading = GetDirection()[0]*180 + 90; // The documentation does not cover this but the angle this spits out is between (-1..1). WHAT?! (also seems to be angle of feet)
		
		delta = Math.DiffAngle(targetAngle, heading);
		
		delta /= 500;
		delta = Math.Max(delta, -0.25);
		delta = Math.Min(delta, 0.25);
				
		// This is a capped PID controller, but using only the P component
		unit.GetInputController().OverrideAimChangeX(true, delta);*/
		
		// Next, we handle logic flow for waypoints.
		float currDistToWP = vector.Distance(unit.GetPosition(), waypoints[cur_waypoint_no]);
		bool gettingCloser = (lastDistToWP > currDistToWP); // If we are getting closer to the WP (a GOOD thing!) - otherwise we can only walk
		
		if (Math.AbsFloat(delta) > 0.24) {															// If we need to turn a lot, we don't want to start walking
			unit.GetInputController().OverrideMovementSpeed(true, 0.0); 
		} else if (currDistToWP > 2 * arrival_radius) { //&& gettingCloser) { 							// If we have a WP but it is far away			
			unit.GetInputController().OverrideMovementSpeed(true, 2.0);
		} else if (currDistToWP > arrival_radius) { 												// If we are getting close to a WP
			unit.GetInputController().OverrideMovementSpeed(true, 1.0);
		} else { 																					// If we are 'at' a WP
			// We have reached our current waypoint
			if (!nextWaypoint())
				clearWaypoints();
			
			needsToRunAgain = true; // Run again, to either go to next WP or cancel all movement
		}
		
		lastDistToWP = currDistToWP;
		
		// Should this be moved to setup?
		unit.GetInputController().OverrideMovementAngle(true, targetAngle*Math.DEG2RAD);//Math.PI/2.0);
		
		return needsToRunAgain;
		
		//m_MovementState.m_CommandTypeId = DayZPlayerConstants.COMMANDID_MOVE ;
		//m_MovementState.m_iMovement = 2;
	}

	void UpdatePathing() { // This update needs to be done way less frequent than Movement; Default is 1 every 10 update ticks.
		
		
		
		if (m_FollowOrders) {
			vector fop = m_FollowOrders.GetPosition();
			
			vector delta = (fop - m_FormLastPos);
			if (delta.Length() > 0.2)
				m_FormDir = delta.Normalized();
			
			// Now we need to transform to the basis of the formation dir
			// Because the perpend function looks weird, we have to do it this way
			vector finalPos = fop + (m_FormDir * formationOffset[2]) + (m_FormDir.Perpend() * formationOffset[0]);
			
			// disabling this temporarily for performance
			//thread updateWaypoints(this, finalPos);
			
			 // this is the temporary workaround
			cur_waypoint_no = -1; // can't use clearWaypoints() here because we want to preserve the distance
			waypoints.Clear();
			addWaypoint(finalPos);
			
			m_FormLastPos = fop;
		} else {
			clearWaypoints();
		}
		
	}
	
	//--------------------------------------------------------------------------------------------------------------------------
	// HELPER FUNCS FOR TARGETING
	//--------------------------------------------------------------------------------------------------------------------------
	
	ref PGFilter occlusionPGFilter = new PGFilter();
	bool IsViewOccluded(vector pos, float tolerance = 0.05) {
		vector hitPos, hitDir;
		return GetGame().GetWorld().GetAIWorld().RaycastNavMesh(unit.GetPosition() + "0 1.5 0", pos, occlusionPGFilter, hitPos, hitDir);
		//return (vector.Distance(hitPos, pos) > tolerance);
	}
	
	void RecalcThreatList(vector center = "0 0 0") {
		
		if (vector.Distance(center, "0 0 0") < 1.0)
			center = unit.GetPosition();
		
		// Leave threats in that don't need cleaning
		for (int j = 0; j < threats.Count(); j++)
			if (!threats[j] || !threats[j].IsAlive() || vector.Distance(unit.GetPosition(), threats[j].GetPosition()) < 30.0)
				threats.Remove(j);
		
		array<Object> newThreats = new array<Object>();
		GetGame().GetObjectsAtPosition(center, 30.0, newThreats, proxyCargos);
		
		// Todo find a faster way to do this... like a linked list?
		int i = 0;
		float minDistance = 1000000.0, temp;
		while (i < newThreats.Count()) {
			DayZInfected infected = DayZInfected.Cast(newThreats[i]);
			PlayerBase player = PlayerBase.Cast(newThreats[i]);
			if (infected && infected.IsAlive() && !IsViewOccluded(infected.GetPosition() + "0 1.5 0")) {
				// It's an infected, add it to teh threates array
				temp = vector.Distance(newThreats[i].GetPosition(), unit.GetPosition());
				if (temp < minDistance) {
					threats.InsertAt(infected, 0);
					minDistance = temp;
				} else threats.Insert(infected);
				
			} // this would make them shoot at all AI
			 /*else if (player && player != m_FollowOrders && (!player.parent || (player.parent.m_FollowOrders != m_FollowOrders)) && player.IsAlive() && !IsViewOccluded(player.GetPosition() + "0 1.5 0")) {
				// If it's an enemy player
				temp = vector.Distance(newThreats[i].GetPosition(), unit.GetPosition());
				if (temp < minDistance) {
					threats.InsertAt(player, 0);
					minDistance = temp;
				} else threats.Insert(player);
			}*/
			i++;
		}
	}
	
	bool dead;
	void markDead() {dead = true;}
	bool isDead() {return dead;}
	
	//--------------------------------------------------------------------------------------------------------------------------
	// BEGIN CODE FOR ACTION INTERLOCKS
	//--------------------------------------------------------------------------------------------------------------------------
	
	float interlockTimeout = 0.0;
	
	//todo
	bool TryRaiseWeapon(bool up) {
	
	}
	
	bool TryReload(bool up) {
	
	}
	
	//--------------------------------------------------------------------------------------------------------------------------
	// BEGIN CODE FOR FSM
	//--------------------------------------------------------------------------------------------------------------------------
	
	// This is used in combat state, is true when the target is sufficiently being aimed at
	bool HasAShot = false;
	
	protected void EnterCombat() {
		
		// WARNING. I THINK THIS WILL CRASH A UNIT WHICH TRIES TO ENTER COMBAT AND RELOAD AT THE SAME TIME.
		// WE NEED TO IMPLEMENT THE INTERLOCK BEFORE THIS WILL ALWAYS WORK
		RaiseWeapon(true);
		
		state = eAIBehaviorGlobal.COMBAT;
		clearWaypoints();	
		Weapon_Base wpn = Weapon_Base.Cast(unit.GetDayZPlayerInventory().GetEntityInHands());
		if (wpn) {
			//unit.lookAt = threats[0];
			if (wpn.CanFire()) {
				combatState = eAICombatPriority.ELIMINATE_TARGET;
			} else {
				combatState = eAICombatPriority.RELOADING;
				unit.QuickReloadWeapon(wpn);
			}
		} else {
			// If we don't have a weapon in hands, we need to switch weapons. But I haven't added this logic yet.
			combatState = eAICombatPriority.FIND_COVER;
		}
	}
	
	protected void UpdateCombatState() {
		Weapon_Base wpn = Weapon_Base.Cast(unit.GetDayZPlayerInventory().GetEntityInHands());
		
		// THIS WILL BREAK IF THE PLAYER DOESN'T HAVE AN OWNER
		GetRPCManager().SendRPC("eAI", "ClientWeaponDataWithCallback", new Param2<Weapon_Base, string>(wpn, "ServerWeaponAimCheck"), false, m_FollowOrders.GetIdentity());
		
		if (threats.Count() == 0 || !threats[0]) {
			state = eAIBehaviorGlobal.RELAXED;	
			return;
		}
		
		if (wpn.CanFire() && combatState == eAICombatPriority.RELOADING) {
			// we are done reloading, we can continue
			combatState = eAICombatPriority.ELIMINATE_TARGET;
		} else if (!wpn.CanFire() && combatState != eAICombatPriority.RELOADING) {
			// need to reload again
			combatState = eAICombatPriority.RELOADING;
			unit.QuickReloadWeapon(wpn);
			return;
		}
		
		
		if (combatState == eAICombatPriority.ELIMINATE_TARGET) {
			
			if (!threats[0] || !threats[0].IsAlive()) {
				RecalcThreatList();
				HasAShot = false;
				
				// Also check if we need to exit combat
				if (threats.Count() < 1) {
					// Similarly, this will crash a unit which exits combat after emptying last round
					RaiseWeapon(false);
					//unit.lookAt = m_FollowOrders;
					state = eAIBehaviorGlobal.RELAXED;
				}
			}
			
			if (wpn.CanFire() && HasAShot) {
				FireHeldWeapon();
			}
		}
	}
	
	ref array<CargoBase> proxyCargos = new array<CargoBase>();// not sure what this is for yet, it is returned by GetObjectsAtPosition
	
	void UpdateState() {
		Weapon_Base wpn = Weapon_Base.Cast(unit.GetDayZPlayerInventory().GetEntityInHands());
		
		if (state == eAIBehaviorGlobal.COMBAT)
			UpdateCombatState();
		
		if (state == eAIBehaviorGlobal.RELAXED) {
			// If we are not reloading but need to, then reload
			if (!wpn.CanFire() && combatState != eAICombatPriority.RELOADING) {
				unit.QuickReloadWeapon(wpn);
				combatState = eAICombatPriority.RELOADING;
				return;
			}			
			
			// Otherwise, if we are currently reloading
			if (combatState == eAICombatPriority.RELOADING) {
				if (!wpn.CanFire())
					return; // continue reloading
				else
					combatState = eAICombatPriority.ELIMINATE_TARGET; // done reloading but not in combat
			}
			
			// maybe do this in another thread
			// also maybe do it less often when relaxed
			RecalcThreatList();
			if (threats.Count() > 0)
				EnterCombat();
		}
	}

};