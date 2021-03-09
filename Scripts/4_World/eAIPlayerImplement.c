enum eAIBehaviorGlobal { // Global states that dictate each unit's actions
	BRAINDEAD, // Do absolutely nothing. For optimization of units outside a certain radius (or unconcious, etc.)
	RELAXED, // Idle and watch surroundings loosely. Will leisurely walk to waypoint.
	ALERT, // Scan surroundings carefully. Will run to waypoint.
	PANIC, // Unused, temp state if shot or injured from unknown direction
	COMBAT // Will engage the threats in the threat list in order. Will ignore waypoints to seek out threats.
}

#define INV_DEBUG


modded class WeaponManager {
	
	override void StartPendingAction()
	{		
		m_WeaponInHand = Weapon_Base.Cast(m_player.GetItemInHands());
		if(!m_WeaponInHand)
		{
			OnWeaponActionEnd();
			return;
		}
		Print("WM::StartPendingAction " + m_PendingWeaponAction.ToString() + " for unit " + m_player.ToString() + ", " + m_player.GetInstanceType().ToString());
		Print("PlayerInventoryInfo: " + m_player.GetDayZPlayerInventory().ToString() + " Type:" + m_player.GetDayZPlayerInventory().Type().ToString());
		switch (m_PendingWeaponAction)
		{
			case AT_WPN_ATTACH_MAGAZINE:
			{
				m_player.GetDayZPlayerInventory().PostWeaponEvent( new WeaponEventAttachMagazine(m_player, m_PendingTargetMagazine) );
				break;
			}
			case AT_WPN_SWAP_MAGAZINE:
			{
				m_player.GetDayZPlayerInventory().PostWeaponEvent( new WeaponEventSwapMagazine(m_player, m_PendingTargetMagazine) );
				break;
			}
			case AT_WPN_DETACH_MAGAZINE:
			{
				Magazine mag = Magazine.Cast(m_PendingInventoryLocation.GetItem());
				m_player.GetDayZPlayerInventory().PostWeaponEvent( new WeaponEventDetachMagazine(m_player, mag, m_PendingInventoryLocation) );
				break;
			}
			case AT_WPN_LOAD_BULLET:
			{
				m_WantContinue = false;
				m_player.GetDayZPlayerInventory().PostWeaponEvent( new WeaponEventLoad1Bullet(m_player, m_PendingTargetMagazine) );
				break;
			}
			case AT_WPN_LOAD_MULTI_BULLETS_START:
			{
				m_player.GetDayZPlayerInventory().PostWeaponEvent( new WeaponEventLoad1Bullet(m_player, m_PendingTargetMagazine) );
				break;
			}
			case AT_WPN_LOAD_MULTI_BULLETS_END:
			{
				m_player.GetDayZPlayerInventory().PostWeaponEvent( new WeaponEventContinuousLoadBulletEnd(m_player) );
				break;
			}
			case AT_WPN_UNJAM:
			{
				m_player.GetDayZPlayerInventory().PostWeaponEvent( new WeaponEventUnjam(m_player, NULL) );
				break;
			}
			case AT_WPN_EJECT_BULLET:
			{
				m_player.GetDayZPlayerInventory().PostWeaponEvent( new WeaponEventMechanism(m_player, NULL) );
				break;
			}
			case AT_WPN_SET_NEXT_MUZZLE_MODE:
			{
				m_player.GetDayZPlayerInventory().PostWeaponEvent( new WeaponEventSetNextMuzzleMode(m_player, NULL) );
				break;
			}
			default:
				m_InProgress = false;
				Error("unknown actionID=" + m_PendingWeaponAction);
		}	
		m_IsEventSended = true;
		m_canEnd = false;
	}
	
	override bool StartAction(int action, Magazine mag, InventoryLocation il, ActionBase control_action = NULL)
	{
		//if it is controled by action inventory reservation and synchronization provide action itself
		if(control_action) {
			m_ControlAction = ActionBase.Cast(control_action);
			Print("WeaponManager::StartAction control action " + m_ControlAction.GetID().ToString() + " triggered on " + m_player.ToString() + " (DayZPlayerInstanceType." + m_player.GetInstanceType().ToString() + ")");
			//Print(m_ControlAction.GetAdminLogMessage());
			m_PendingWeaponAction = action;
			m_InProgress = true;
			m_IsEventSended = false;
			m_PendingTargetMagazine = mag;
			m_PendingInventoryLocation = il;
			
			DumpStack();
			StartPendingAction();
			
			return true;
		}
		
		// Note that we added a condition to continue if server exec for an AI unit.
		if (GetGame().IsMultiplayer() && GetGame().IsServer() && !m_player.isAI())
			return false;
		
		if ( !ScriptInputUserData.CanStoreInputUserData() )
			return false;
		if ( !InventoryReservation(mag, il) )
			return false;

		m_PendingWeaponAction = action;
		m_InProgress = true;
		m_IsEventSended = false;
		
		if ( !GetGame().IsMultiplayer() ) {
			m_readyToStart = true;
		} else if (/*GetGame().IsServer() && */m_player.isAI()) {
			m_PendingTargetMagazine = mag;
			m_PendingInventoryLocation = il;
			SynchronizeServer(mag, il);
		} else {
			Synchronize();
		}
		
		return true;
	}
	
	// This is a meshing of the Synchronize() function and OnInputUserDataProcess() functions.
	// It is meant to do the job of the RPC sent by synchronize and the processing done by the latter function
	// since we are already on the server.
	// This function is only used by the WeaponManager of AI units and executes server-side.
	private bool SynchronizeServer(Magazine mag, InventoryLocation il){
		if(GetGame().IsServer() && m_player.isAI()) {
			Print("WeaponManager::SynchronizeServer for AI unit " + m_player.ToString() + " started.");
			
			m_PendingWeaponActionAcknowledgmentID = ++m_LastAcknowledgmentID;
			
			// When I was modifying this function , I realized you cannot re-instantiate a reference parameter even if it 
			// has been passed as null (as is the case for AT_WPN_ATTACH_MAGAZINE). So, I made a new InventoryLocation for this 
			// case only. I think it needs to be in-scope at the end of this function when the junctures are sent?
			InventoryLocation il2;
			
			Weapon_Base wpn;
			int mi;
			
			int slotID;
			bool accepted = false;
          //////////////////////////////////////////////////////////				

			m_InProgress = true;
			m_IsEventSended = false;
			
			Weapon_Base.CastTo( wpn, m_player.GetItemInHands() );
			if ( wpn )
				mi = wpn.GetCurrentMuzzle();
			
			switch (m_PendingWeaponAction)
			{
				case AT_WPN_ATTACH_MAGAZINE: {
					if ( !mag || !wpn )
						break;

					slotID = wpn.GetSlotFromMuzzleIndex(mi);
					il2 = new InventoryLocation;
					il2.SetAttachment(wpn,mag,slotID);
					if( GetGame().AddInventoryJuncture(m_player, mag, il2, false, 10000) )
						accepted = true;
					m_PendingTargetMagazine = mag;
					break;
				}
				case AT_WPN_SWAP_MAGAZINE: {
					if ( !mag || !wpn )
						break;

					if ( !wpn.GetMagazine(mi) )
						break;
					
					if ( GetGame().AddActionJuncture(m_player,mag,10000) )
						accepted = true;
					
					m_PendingTargetMagazine = mag;
					break;
				}
				case AT_WPN_DETACH_MAGAZINE:
				{
					if ( !il.IsValid() )
						break;
					
					if ( !wpn )
						break;
					
					Magazine det_mag = wpn.GetMagazine(mi);
					mag = Magazine.Cast(il.GetItem());
					if ( !det_mag || ( mag != det_mag) )
						break;
					
					if( GetGame().AddInventoryJuncture(m_player, il.GetItem(), il, true, 10000))
						accepted = true;
					
					m_PendingInventoryLocation = il;
					m_PendingTargetMagazine = mag;
					break;
				}
				case AT_WPN_LOAD_BULLET:
				{
					if ( !mag )
						break;
					
					if( GetGame().AddActionJuncture(m_player,mag,10000) )
						accepted = true;
					m_PendingTargetMagazine = mag;
					break;
				}
				case AT_WPN_LOAD_MULTI_BULLETS_START:
				{
					if ( !mag )
						break;
					
					if( GetGame().AddActionJuncture(m_player,mag,10000) )
						accepted = true;
					m_PendingTargetMagazine = mag;
					break;
				}
				case AT_WPN_UNJAM:
				{
					accepted = true;
					//Unjam();
					break;
				}
				case AT_WPN_EJECT_BULLET:
				{
					accepted = true;
					break;
				}
				case AT_WPN_SET_NEXT_MUZZLE_MODE:
				{
					accepted = true;
					break;
				}
				default:
					Error("unknown actionID=" + m_PendingWeaponAction);
					break;
			}
			// might be unnecessary - this is for the client which reloaded only?
			DayZPlayerSyncJunctures.SendWeaponActionAcknowledgment(m_player, m_PendingWeaponActionAcknowledgmentID, accepted);
			
			return accepted;
		}
		
		Print("WeaponManager::SynchronizeServer failed! IsServer? " + GetGame().IsServer().ToString() + ". IsAI? " + m_player.isAI().ToString());
		
		return false; // This case is only executed if the conditions are not met
	}
}


// Todo move this to a separate child class (like PlayerBaseClient)
modded class PlayerBase {
	ref array<Entity> threats = new array<Entity>();
	
	bool m_WantWeapRaise = false;
	
	Entity m_FollowOrders = null; // This is a reference to the entity we want to pathfind to.
	
	float m_FollowDistance = 2.0; // This is the distance that we consider a waypoint to be 'reached' at.
	
	int cur_waypoint_no = -1; // Next point to move to in the waypoints array. -1 for "none"
	ref TVectorArray waypoints = new TVectorArray(); // This is the list of waypoints the AI will follow. It is set by AIWorld.
												 // This could be used by players too, with some kind of UI "highlight waypoint" system.
	
	// This is checked every pathing update to ensure we are not running away from the waypoint.
	float lastDistToWP = -1; // If it is set to -1, the AI will not try to sprint on the first movement update.
	
	ref PGFilter pgFilter = new PGFilter(); // This is the bitmask used for pathfinding (i.e. whether we want a path that goes over a fence
	
	bool isAI = false;
	
	// IMPORTANT: Since this class is always constructed in engine, we need some other way to mark a unit as AI outside of the constructor.
	// As such, when spawning AI make sure you ALWAYS CALL MARKAI() ON THE UNIT AFTER TELLING THE ENGINE TO SPAWN THE UNIT.
	// This is used in many functions for workarounds for AI controlled players and it will break things if AI are not properly marked.
	bool isAI() {return isAI;}
	void markAIServer() {
		isAI = true;
		m_ActionManager = new ActionManagerAI(this);
		// Todo do we need to set instance type?
		
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
		
		newInv = new eAIDayZInventory(this);
		newInv.Init();
	}
	
	void markAIClient() {
		isAI = true;
		//m_ActionManager = new ActionManagerClient(this);
	}
	
	//Eventually we can use this instead of calling the action manager, needs more work though
	override void ReloadWeapon( EntityAI weapon, EntityAI magazine ) {
		// The only reason this is an override is because there is a client-only condition here that I have removed.
		// There is probably a better way to do this.
		Print(this.ToString() + "(DayZPlayerInstanceType." + GetInstanceType().ToString() + ") is trying to reload " + magazine.ToString() + " into " + weapon.ToString());
		ActionManagerClient mngr_client;
		CastTo(mngr_client, GetActionManager());
		
		if (mngr_client && FirearmActionLoadMultiBulletRadial.Cast(mngr_client.GetRunningAction()))
		{
			mngr_client.Interrupt();
		}
		else if ( GetHumanInventory().GetEntityInHands()!= magazine )
		{
			Weapon_Base wpn;
			Magazine mag;
			Class.CastTo( wpn,  weapon );
			Class.CastTo( mag,  magazine );
			if ( GetWeaponManager().CanUnjam(wpn) )
			{
				GetWeaponManager().Unjam();
			}
			else if ( GetWeaponManager().CanAttachMagazine( wpn, mag ) )
			{
				GetWeaponManager().AttachMagazine(mag);
			}
			else if ( GetWeaponManager().CanSwapMagazine( wpn, mag ) )
			{
				GetWeaponManager().SwapMagazine( mag );
			}
			else if ( GetWeaponManager().CanLoadBullet( wpn, mag ) )
			{
				//GetWeaponManager().LoadMultiBullet( mag );

				ActionTarget atrg = new ActionTarget(mag, this, -1, vector.Zero, -1.0);
				if ( mngr_client && !mngr_client.GetRunningAction() && mngr_client.GetAction(FirearmActionLoadMultiBulletRadial).Can(this, atrg, wpn) )
					mngr_client.PerformActionStart(mngr_client.GetAction(FirearmActionLoadMultiBulletRadial), atrg, wpn);
			}
		}
	}
	
	// We should integrate this into ReloadWeapon
	void ReloadWeaponAI( EntityAI weapon, EntityAI magazine ) {
		// The only reason this is an override is because there is a client-only condition here that I have removed.
		// There is probably a better way to do this.
		Print(this.ToString() + "(DayZPlayerInstanceType." + GetInstanceType().ToString() + ") is trying to reload " + magazine.ToString() + " into " + weapon.ToString());
		ActionManagerAI mngr_ai;
		CastTo(mngr_ai, GetActionManager());
		
		if (mngr_ai && FirearmActionLoadMultiBulletRadial.Cast(mngr_ai.GetRunningAction()))
		{
			mngr_ai.Interrupt();
		}
		else if ( GetHumanInventory().GetEntityInHands()!= magazine )
		{
			Weapon_Base wpn;
			Magazine mag;
			Class.CastTo( wpn,  weapon );
			Class.CastTo( mag,  magazine );
			if ( GetWeaponManager().CanUnjam(wpn) )
			{
				GetWeaponManager().Unjam();
			}
			else if ( GetWeaponManager().CanAttachMagazine( wpn, mag ) )
			{
				GetWeaponManager().AttachMagazine(mag);//, new FirearmActionAttachMagazineQuick() );
			}
			else if ( GetWeaponManager().CanSwapMagazine( wpn, mag ) )
			{
				GetWeaponManager().SwapMagazine( mag );
			}
			else if ( GetWeaponManager().CanLoadBullet( wpn, mag ) )
			{
				GetWeaponManager().LoadMultiBullet( mag );

				ActionTarget atrg = new ActionTarget(mag, this, -1, vector.Zero, -1.0);
				if ( mngr_ai && !mngr_ai.GetRunningAction() && mngr_ai.GetAction(FirearmActionLoadMultiBulletRadial).Can(this, atrg, wpn) )
					mngr_ai.PerformActionStart(mngr_ai.GetAction(FirearmActionLoadMultiBulletRadial), atrg, wpn);
			}
		}
	}
	
	override void QuickReloadWeapon( EntityAI weapon )
	{
		EntityAI magazine = GetMagazineToReload( weapon );
		if (isAI())
			ReloadWeaponAI( weapon, magazine );
		else
			ReloadWeapon( weapon, magazine );
	}
	
	ref DayZPlayerInventory newInv;
	
	//override GameInventory GetInventory() {
	//	if (isAI())
	//		return newInv;
	//	return super.GetInventory();
	//}
	
	/*override DayZPlayerInventory GetDayZPlayerInventory ()
	{
		DayZPlayerInventory inv;
		if (isAI())
			inv = DayZPlayerInventory.Cast(newInv);
		else
			inv = DayZPlayerInventory.Cast(GetInventory());
		return inv;
	}
	
	override HumanInventory GetHumanInventory ()
	{
		HumanInventory inv;
		if (isAI())
			inv = HumanInventory.Cast(newInv);
		else
			inv = HumanInventory.Cast(GetInventory());
		return inv;
	}*/

	void eAIFollow(DayZPlayer p, float distance = 5.0) {
		m_FollowOrders = p;
		m_FollowDistance = distance;
	}
	
	float targetAngle, heading, delta;
	
	void eAIDebugMovement() {
		GetWeaponManager().Fire(Weapon_Base.Cast(GetHumanInventory().GetEntityInHands()));
		GetInputController().OverrideAimChangeY(true, 0.0);
	}
	
	void ToggleWeaponRaise() {
		m_WantWeapRaise = !m_WantWeapRaise;
		GetInputController().OverrideRaise(true, m_WantWeapRaise);
		HumanCommandMove cm = GetCommand_Move();
		if (m_WantWeapRaise) {
			cm.ForceStance(DayZPlayerConstants.STANCEIDX_RAISEDERECT);
		} else {
			cm.ForceStance(DayZPlayerConstants.STANCEIDX_ERECT);
		}
	}
	
	// This returns true if the weapon should be raised.
	bool AIWeaponRaise() {
		return m_WantWeapRaise;
	}
	
	// As with many things we do, this is an almagomation of the client and server code
	override void CheckLiftWeapon()
	{
		if (isAI() && GetGame().IsServer()) {	
			bool state = false;
			Weapon_Base weap;
			if ( Weapon_Base.CastTo(weap, GetItemInHands()) )
			{
				state = m_LiftWeapon_player;
				bool limited = weap.LiftWeaponCheck(this);
				if (limited) //(limited && !m_WantWeapRaise)
					state = false;
			}
			
			// Now the server code
			ScriptJunctureData pCtx = new ScriptJunctureData;
			pCtx.Write(state);
			SendSyncJuncture(DayZPlayerSyncJunctures.SJ_WEAPON_LIFT, pCtx);
		}
		else super.CheckLiftWeapon();
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
	
	void eAIUpdateTargeting(float pDt) {
		// For now, just the logic for conrolling aim elevation
		//float quatHeadTrans[4];
		//int idx = GetBoneIndexByName("Head");
		//if (idx < 0)
		//	Error("I've lost my darn head!");
		//GetBoneRotationWS(idx, quatHeadTrans);
		//vector headTrans = Math3D.QuatToAngles(quatHeadTrans); //despite what it says in the doc, this goes <Yaw, Roll, Pitch> with Pitch measured from the +Y axis
		
		// This always returns 0
		//vector direction = GetInputController().GetHeadingAngle() * Math.RAD2DEG; 
		//Print(direction);

		float delta = -((GetAimingModel().getAimY()+10.0)*Math.DEG2RAD)/5.0;
		
		GetInputController().OverrideAimChangeY(true, delta);
		
		// Ignore the fact that this works. Do not ask why it works. Or how I found out that creating a faux recoil event
		// after updating the input controller updates the aim change. If you ask me about this code or why it is written
		// this way, I will deny its existance.
		//
		// Just kidding :-)   ... or am I?
		GetAimingModel().SetDummyRecoil(Weapon_Base.Cast(GetHumanInventory().GetEntityInHands()));
	}
	
	override bool AimingModel(float pDt, SDayZPlayerAimingModel pModel) {
		//if (isAI()) {
		//	GetInputController().OverrideAimChangeY(true, -0.25);
		//	GetAimingModel().SetRecoil(Weapon_Base.Cast(GetHumanInventory().GetEntityInHands()));
			//return true;
		//}
		
		// If we are not 
		return super.AimingModel(pDt, pModel);
	}	
	
	bool eAIUpdateMovement() {
		
		bool needsToRunAgain = false;
		
		// First, if we don't have any valid waypoints, make sure to stop the AI and quit prior to the movement logic.
		// Otherwise, the AimChange would be undefined in this case (leading to some hilarious behavior including disappearing heads)
		if (cur_waypoint_no < 0) {
			GetInputController().OverrideMovementSpeed(true, 0.0);
			GetInputController().OverrideAimChangeX(true, 0.0);
			return false;
		}
		
		targetAngle = vector.Direction(GetPosition(), waypoints[cur_waypoint_no]).VectorToAngles().GetRelAngles()[0];// * Math.DEG2RAD;
		//vector heading = MiscGameplayFunctions.GetHeadingVector(this);
		
		// This seems to be CCW is positive unlike relative angles.
		// ALSO THIS ISN'T CAPPED AT +-180. I HAVE SEEN IT IN THE THOUSANDS.
		heading = -GetInputController().GetHeadingAngle() * Math.RAD2DEG; 

		
		//heading = GetDirection()[0]*180 + 90; // The documentation does not cover this but the angle this spits out is between (-1..1). WHAT?! (also seems to be angle of feet)
		
		delta = Math.DiffAngle(targetAngle, heading);
		
		delta /= 500;
		delta = Math.Max(delta, -0.25);
		delta = Math.Min(delta, 0.25);
				
		// This is a capped PID controller, but using only the P component
		GetInputController().OverrideAimChangeX(true, delta);
		
		// Next, we handle logic flow for waypoints.
		float currDistToWP = vector.Distance(GetPosition(), waypoints[cur_waypoint_no]);
		bool gettingCloser = (lastDistToWP > currDistToWP); // If we are getting closer to the WP (a GOOD thing!) - otherwise we can only walk
		
		if (Math.AbsFloat(delta) > 0.24) {															// If we need to turn a lot, we don't want to start walking
			GetInputController().OverrideMovementSpeed(true, 0.0); 
		} else if (currDistToWP > 2 * m_FollowDistance && gettingCloser) { 							// If we have a WP but it is far away			
			GetInputController().OverrideMovementSpeed(true, 2.0);
		} else if (currDistToWP > m_FollowDistance) { 												// If we are getting close to a WP
			GetInputController().OverrideMovementSpeed(true, 1.0);
		} else { 																					// If we are 'at' a WP
			// We have reached our current waypoint
			if (!nextWaypoint())
				clearWaypoints();
			
			needsToRunAgain = true; // Run again, to either go to next WP or cancel all movement
		}
		
		lastDistToWP = currDistToWP;
		
		// Should this be moved to setup?
		GetInputController().OverrideMovementAngle(true, targetAngle*Math.DEG2RAD);//Math.PI/2.0);
		
		return needsToRunAgain;
		
		//m_MovementState.m_CommandTypeId = DayZPlayerConstants.COMMANDID_MOVE ;
		//m_MovementState.m_iMovement = 2;
	}
	
	// For debugging purposes, I am only updating this every 30 seconds to let the player to get some distance on the AI
	void eAIUpdateBrain() { // This update needs to be done way less frequent than Movement; Default is 1 every 10 update ticks.
		if (m_FollowOrders) { // If we have a reference to a player to follow, we refresh the waypoints.
			thread updateWaypoints(this);
		}
	}
	
	//DayZPlayerCameraResult m_pOutResult = new DayZPlayerCameraResult();
	
	override void CommandHandler(float pDt, int pCurrentCommandID, bool pCurrentCommandFinished)	
	{
		super.CommandHandler(pDt, pCurrentCommandID, pCurrentCommandFinished);
		if (isAI()) {
			//m_DirectionToCursor = m_CurrentCamera.GetBaseAngles();
			//if (m_CurrentCamera)
				//m_CurrentCamera.OnUpdate(pDt, m_pOutResult);
		}
	}
	
	override void OnCameraChanged(DayZPlayerCameraBase new_camera)
	{
		Print("Camera for " + this.ToString() + " changed. old:" + m_CurrentCamera.ToString() + " new:"+new_camera.ToString());
		m_CameraSwayModifier = new_camera.GetWeaponSwayModifier();
		m_CurrentCamera = new_camera;
	}

};