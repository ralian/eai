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
	ref PGFilter m_PathFilter;
	autoptr array<Shape> m_DebugShapeArray = new array<Shape>();
	
	// Formation following logic
	DayZPlayer m_FollowOrders = null;
	vector m_FormationOffset = "0 0 0";
	
	// Data for the heading computation
	float targetAngle, heading, delta;
	
	// Logic for waypoints
	autoptr TVectorArray m_Path = new TVectorArray(); // This is the list of waypoints the AI will follow. It is set by AIWorld.
	
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
		
		m_PathFilter = new PGFilter();

		int inFlags = PGPolyFlags.WALK | PGPolyFlags.DOOR | PGPolyFlags.INSIDE | PGPolyFlags.JUMP_OVER;
		int exFlags = PGPolyFlags.DISABLED | PGPolyFlags.SWIM | PGPolyFlags.SWIM_SEA | PGPolyFlags.JUMP | PGPolyFlags.CLIMB | PGPolyFlags.CRAWL | PGPolyFlags.CROUCH;

		m_PathFilter.SetFlags( inFlags, exFlags, PGPolyFlags.NONE );
		m_PathFilter.SetCost( PGAreaType.JUMP, 0.0 );
		m_PathFilter.SetCost( PGAreaType.FENCE_WALL, 0.0 );
		m_PathFilter.SetCost( PGAreaType.WATER, 1.0 );
		
		// This is a separate PGFilter for raycasting what we can "See"
		// Todo move this to a global filter?
		int allowFlags2 = 0;
		allowFlags2 |= PGPolyFlags.ALL;
		allowFlags2 |= PGPolyFlags.WALK;
		occlusionPGFilter.SetFlags(allowFlags2, 0, 0);
	}
	
	// Set a player to follow.
	void Follow(DayZPlayer p, vector formationPos, float distance = 2.0) {
		m_FormationOffset = formationPos;
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
		//if (m_WantWeapRaise) // double check that this hasn't been reset
		//	unit.eAI_Use_ADS_Tracking = true;
	}
	
	// todo fix the name of this
	void RaiseWeapon(bool up) {
		m_WantWeapRaise = up;

		HumanCommandMove cm = unit.GetCommand_Move();
		if (!cm) return;

		unit.GetInputController().OverrideRaise(true, m_WantWeapRaise);
		if (m_WantWeapRaise) {
			cm.ForceStance(DayZPlayerConstants.STANCEIDX_RAISEDERECT);
		} else {
			cm.ForceStance(DayZPlayerConstants.STANCEIDX_ERECT);
		}
		
		// Now, we need to kick off the client weapon aim arbitration.
		if (up) {
			if (m_FollowOrders && m_FollowOrders.GetIdentity()) {
				// Start the client arbiter for this AI's weapon. The arbiter must already be init'ed
				GetRPCManager().SendRPC("eAI", "eAIAimArbiterStart", new Param2<Weapon_Base, int>(Weapon_Base.Cast(unit.GetHumanInventory().GetEntityInHands()), 250), false, m_FollowOrders.GetIdentity());
				// Use ADS instead of view for targeting, but we have to wait until our data from the client is valid
				GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.EnableADSTracking, 1000, false);
				Print(this.ToString() + " entering ADS");
			} else {
				Error("Tried entering ADS, but no available aim arbiter was available!");
			}
		} else {
			GetRPCManager().SendRPC("eAI", "eAIAimArbiterStop", new Param1<Weapon_Base>(Weapon_Base.Cast(unit.GetHumanInventory().GetEntityInHands())), false, m_FollowOrders.GetIdentity());
			//unit.eAI_Use_ADS_Tracking = false;
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
	
#ifndef SERVER
	void ClearDebugShapes()
	{
		for (int i = m_DebugShapeArray.Count() - 1; i >= 0; i--)
			m_DebugShapeArray[i].Destroy();
		m_DebugShapeArray.Clear();
	}
	
	void AddShape(Shape shape)
	{
		m_DebugShapeArray.Insert(shape);
	}
#endif	

	float Distance( int index, vector position )
	{
		vector b = m_Path[index];
		vector e = m_Path[index + 1] - b;
		vector p = position - b;
		float eSize2 = e.LengthSq();
		if ( eSize2 > 0 )
		{
			float t = ( e * p ) / eSize2;
			vector nearest = b + Math.Clamp( t, 0, 1 ) * e;
			return vector.DistanceSq( nearest, position );
		} else
		{
			return vector.DistanceSq( b, position );
		}
	}

	void PathClear()
	{
		m_Path.Clear();
	}
	
	int PathCount()
	{
		return m_Path.Count();
	}
	
	vector PathGet( int index )
	{
		if ( index < 0 || index >= m_Path.Count() )
			return "0 0 0";
		
		return m_Path[ index ];
	}

	int FindNext( vector position )
	{
		float dist;
		return FindNext( position, dist );
	}
	
	// Checks to see if the path between the start and end is blocked
	bool PathBlocked( vector start, vector end )
	{
		vector hitPos;
		vector hitNormal;
		
		AIWorld world = GetGame().GetWorld().GetAIWorld();
		
		bool blocked = world.RaycastNavMesh( start, end, m_PathFilter, hitPos, hitNormal );

#ifndef SERVER
		int debugColour = 0xFF00FF00;
		if (blocked) debugColour = 0xFFFF0000;
		vector points[2];
		points[0] = start;
		points[1] = end;
		if (blocked) points[1] = hitPos;
		m_DebugShapeArray.Insert(Shape.CreateLines(debugColour, ShapeFlags.NOZBUFFER, points, 2));
#endif

		return blocked;
	}
	
	// Checks to see if the path between the start and end is blocked
	bool PathBlocked( vector start, vector end, out vector hitPos, out vector hitNormal )
	{
		AIWorld world = GetGame().GetWorld().GetAIWorld();
		
		bool blocked = world.RaycastNavMesh( start, end, m_PathFilter, hitPos, hitNormal );

#ifndef SERVER
		int debugColour = 0xFF00FF00;
		if (blocked) debugColour = 0xFFFF0000;
		vector points[2];
		points[0] = start;
		points[1] = end;
		if (blocked) points[1] = hitPos;
		m_DebugShapeArray.Insert(Shape.CreateLines(debugColour, ShapeFlags.NOZBUFFER, points, 2));
#endif

		return blocked;
	}

	int FindNext( vector position, out float minDist )
	{
		int index = 0;

		minDist = 1000000000.0;

		float epsilon = 0.1;
		for ( int i = 0; i < m_Path.Count() - 1; ++i )
		{
			float dist = Distance( i, position );
			
			if ( minDist >= dist - epsilon )
			{
				if ( !PathBlocked( position, m_Path[i + 1] ) )
				{
					minDist = dist;
					index = i;
				}
			}
		}

		return index + 1;
	}

	float AngleBetweenPoints( vector pos1, vector pos2 )
	{
		return vector.Direction( pos1, pos2 ).Normalized().VectorToAngles()[0];
	}

	void UpdatePath()
	{
		if ( !m_PathFilter || !m_FollowOrders )
		{
			PathClear();
			return;
		}
		
		m_Path.Clear();
		
		AIWorld world = GetGame().GetWorld().GetAIWorld();
		
		vector outPos = m_FollowOrders.GetPosition() + m_FormationOffset;
		vector outNormal;
		PathBlocked(m_FollowOrders.GetPosition(), outPos, outPos, outNormal);

		world.FindPath( unit.GetPosition(), outPos, m_PathFilter, m_Path );
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

		PathClear();	

		Weapon_Base wpn = Weapon_Base.Cast(unit.GetDayZPlayerInventory().GetEntityInHands());
		if (wpn) {
			unit.lookAt = threats[0];
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
				unit.lookAt = threats[0];
				HasAShot = false;
				
				// Also check if we need to exit combat
				if (threats.Count() < 1) {
					// Similarly, this will crash a unit which exits combat after emptying last round
					RaiseWeapon(false);
					unit.lookAt = null;
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