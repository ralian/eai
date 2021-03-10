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
	Entity m_FollowOrders = null;
	
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
	}
	
	/*void ~eAIPlayerHandler() {
		delete unit;
		delete cam;
		delete threats;
		delete pgFilter;
		delete waypoints;
	}*/
	
	// Set a player to follow.
	void Follow(DayZPlayer p, float distance = 5.0) {
		m_FollowOrders = p;
		arrival_radius = distance;
	}
	
	void FireHeldWeapon() {
		unit.GetWeaponManager().Fire(Weapon_Base.Cast(unit.GetHumanInventory().GetEntityInHands()));
		unit.GetInputController().OverrideAimChangeY(true, 0.0);
	}
	
	void WeaponRaise(bool up) {
		m_WantWeapRaise = up;
		unit.GetInputController().OverrideRaise(true, m_WantWeapRaise);
		HumanCommandMove cm = unit.GetCommand_Move();
		if (m_WantWeapRaise) {
			cm.ForceStance(DayZPlayerConstants.STANCEIDX_RAISEDERECT);
		} else {
			cm.ForceStance(DayZPlayerConstants.STANCEIDX_ERECT);
		}
	}
	
	void ToggleWeaponRaise() {
		WeaponRaise(!m_WantWeapRaise);
	}
	
	// This returns true if the weapon should be raised.
	bool WantsWeaponUp() {
		return m_WantWeapRaise;
	}
	
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
		
		if (state == eAIBehaviorGlobal.COMBAT) {
			vector myPos = unit.GetPosition();
			vector threatPos = threats[0].GetPosition();
			
			// Do the logic for aiming along the x axis...
			// Todo make it so the x direction is calculated from barrell
			unit.GetInputController().OverrideMovementSpeed(true, 0.0);
			targetAngle = vector.Direction(myPos, threatPos).VectorToAngles().GetRelAngles()[0];
			heading = -(unit.GetInputController().GetHeadingAngle() * Math.RAD2DEG); 
			delta = Math.DiffAngle(targetAngle, heading);
			delta /= 500;
			delta = Math.Max(delta, -0.25);
			delta = Math.Min(delta, 0.25);
			unit.GetInputController().OverrideAimChangeX(true, delta);
			
			// Now, do the logic for aiming along the y axis.
			float gunHeight = 1.5 + myPos[1]; 			// Todo get the actual world gun height.
			float targetHeight = 1.0 + threatPos[1]; 	// Todo get actual threat height
			float aimAngle = Math.Atan2(targetHeight - gunHeight, vector.Distance(myPos, threatPos));
			unit.targetAngle = aimAngle * Math.RAD2DEG;
			
			// Enable firing if the AI is with a threshold of 
			HasAShot = (delta < 0.01);
			return false;
		}
		
		// First, if we don't have any valid waypoints, make sure to stop the AI and quit prior to the movement logic.
		// Otherwise, the AimChange would be undefined in this case (leading to some hilarious behavior including disappearing heads)
		if (cur_waypoint_no < 0) {
			unit.GetInputController().OverrideMovementSpeed(true, 0.0);
			unit.GetInputController().OverrideAimChangeX(true, 0.0);
			return false;
		}
		
		if (m_FollowOrders) {
			cur_waypoint_no = -1; // can't use clearWaypoints() here because we want to preserve the distance
			waypoints.Clear();
			addWaypoint(m_FollowOrders.GetPosition());
		};
		
		targetAngle = vector.Direction(unit.GetPosition(), waypoints[cur_waypoint_no]).VectorToAngles().GetRelAngles()[0];// * Math.DEG2RAD;
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
		unit.GetInputController().OverrideAimChangeX(true, delta);
		
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
	
	// For debugging purposes, I am only updating this every 30 seconds to let the player to get some distance on the AI
	void UpdatePathing() { // This update needs to be done way less frequent than Movement; Default is 1 every 10 update ticks.
		if (m_FollowOrders) { // If we have a reference to a player to follow, we refresh the waypoints.
			thread updateWaypoints(this);
		}
	}
	
	void RecalcThreatList() {
		GetGame().GetObjectsAtPosition(unit.GetPosition(), 10.0, threats, proxyCargos);
		
		// Todo find a faster way to do this... like a linked list?
		int i = 0;
		float minDistance = 1000000.0, temp;
		while (i < threats.Count()) {
			if (DayZInfected.Cast(threats[i]) && threats[i].IsAlive()) {
				temp = vector.Distance(threats[i].GetPosition(), unit.GetPosition());
				// See if this index is the biggest threat, if so swap it to front
				if (temp < minDistance && i > 1) {
					threats.SwapItems(0, i);
					minDistance = temp;
				}
				i++;
			} else
				threats.RemoveOrdered(i);
		}
	}
	
	bool dead;
	void markDead() {dead = true;}
	bool isDead() {return dead;}
	
	//--------------------------------------------------------------------------------------------------------------------------
	// BEGIN CODE FOR FSM
	//--------------------------------------------------------------------------------------------------------------------------
	
	// This is used in combat state, is true when the target is sufficiently being aimed at
	bool HasAShot = false;
	
	protected void EnterCombat() {
		state = eAIBehaviorGlobal.COMBAT;
		clearWaypoints();	
		Weapon_Base wpn = Weapon_Base.Cast(unit.GetDayZPlayerInventory().GetEntityInHands());
		if (wpn) {
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
				if (threats.Count() < 1)
					state = eAIBehaviorGlobal.RELAXED;
			}
			
			if (wpn.CanFire() && HasAShot) {
				FireHeldWeapon();
			}
		}
	}
	
	ref array<CargoBase> proxyCargos = new array<CargoBase>();// not sure what this is for yet, it is returned by GetObjectsAtPosition
	
	void UpdateState() {
		if (state == eAIBehaviorGlobal.COMBAT)
			UpdateCombatState();
		
		if (state == eAIBehaviorGlobal.RELAXED) {
			// maybe do this in another thread
			// also maybe do it less often when relaxed
			RecalcThreatList();
			if (threats.Count() > 0)
				EnterCombat();
		}
	}

};