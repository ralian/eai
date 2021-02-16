class eAIMovementState : HumanMovementState {
	bool m_weaponRaised = false;
	
	void eAIMovementState() {
		Print("eAIMovementState: Init");
		//m_CommandTypeId = DayZPlayerConstants.COMMANDID_MOVE;
		//m_iMovement = 2;
	}
	
	void toggleWeapon() {m_weaponRaised = !m_weaponRaised;}
};

modded class WeaponManager {
	override bool StartAction(int action, Magazine mag, InventoryLocation il, ActionBase control_action = NULL)
	{
		//if it is controled by action inventory reservation and synchronization provide action itself
		if(control_action)
		{
			m_ControlAction = ActionBase.Cast(control_action);
			m_PendingWeaponAction = action;
			m_InProgress = true;
			m_IsEventSended = false;
			m_PendingTargetMagazine = mag;
			m_PendingInventoryLocation = il;
			StartPendingAction();
			
			return true;
		}
		
		if ( !ScriptInputUserData.CanStoreInputUserData() )
			return false;
		if ( !InventoryReservation(mag, il) )
			return false;

		m_PendingWeaponAction = action;
		m_InProgress = true;
		m_IsEventSended = false;
		
		if ( !GetGame().IsMultiplayer() )
			m_readyToStart = true;
		else
			Synchronize();
		
		// Okay. So. I don't completely understand the client/server interaction here, but this bit is an override
		// (replacing BI code which would kick a server out of this function) to force server-local AI to kick into the queued
		// reloading action. For some reason however the correct animation triggers, but the magazine is invisible and ends up 
		// getting bugged.
		if (GetGame().IsMultiplayer() && GetGame().IsServer()) {
			m_PendingTargetMagazine = mag;
			m_PendingInventoryLocation = il;
			StartPendingAction();
		}
		
		return true;
	}
}


modded class PlayerBase {
	ref HumanMovementState m_MovementState = new eAIMovementState();
	//ref eAIHumanInputController m_eAIController = new eAIHumanInputController();
	
	Entity m_FollowOrders = null; // If this player id is > 0, they will follow.
	float m_FollowDistance = 5.0;
	vector m_NextWaypoint = "0 0 0"; // otherwise, default to waypoint.
	
	bool isAI = false;
	
	// IMPORTANT: Since this class is always constructed in engine, we need some other way to mark a unit as AI outside of the constructor.
	// As such, when spawning AI make sure you ALWAYS CALL MARKAI() ON THE UNIT AFTER TELLING THE ENGINE TO SPAWN THE UNIT.
	// This is used in many functions for workarounds for AI controlled players and it will break things if AI are not properly marked.
	void markAI() {isAI = true;}
	bool isAI() {return isAI;}
	
	//Reload weapon with given magazine
	override void ReloadWeapon( EntityAI weapon, EntityAI magazine ) {
		Print(this.ToString() + " is trying to reload " + magazine.ToString() + " into " + weapon.ToString());
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
				GetWeaponManager().AttachMagazine( mag );
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
	
	void eAIFollow(DayZPlayer p, float distance = 5.0) {
		m_FollowOrders = p;
		m_FollowDistance = distance;
	}
	
	float targetAngle, heading, delta;
	
	void eAIDebugMovement() {
		//Print("targetAngle: " + targetAngle);
		//Print("heading: " + heading);
		//Print("delta: " + delta);
		
		//GetCommandModifier_Weapons().StartAction(WeaponActions.RELOAD, WeaponActionReloadTypes.RELOADRIFLE_NOMAGAZINE_NOBULLET); // Need to use wpn UIManager
		//WeaponManager wm = GetWeaponManager();
		//if (wm.CanAttachMagazine(GetHumanInventory().GetEntityInHands(), wm.GetPreparedMagazine())) {
		//	wm.AttachMagazine(wm.GetPreparedMagazine());
		//}
		
		QuickReloadWeapon(GetHumanInventory().GetEntityInHands());
		
		//GetWeaponManager().Fire(Weapon_Base.CastTo(GetHumanInventory().GetEntityInHands())); // Needs to reload first
		//GetCommandModifier_Weapons().SetADS(true); // Use M to toggle ADS as a test
	}
	
	void eAIUpdateMovement() {
		
		if (m_FollowOrders) {m_NextWaypoint = m_FollowOrders.GetPosition();} // move to brain
		
		targetAngle = vector.Direction(GetPosition(), m_NextWaypoint).VectorToAngles().GetRelAngles()[0];// * Math.DEG2RAD;
		heading = -GetInputController().GetHeadingAngle() / Math.DEG2RAD; // This seems to be CCW is positive unlike relative angles.
														// ALSO THIS ISN'T CAPPED AT +-180. I HAVE SEEN IT IN THE THOUSANDS. WHY BOHEMIA?
		//heading = GetDirection()[0]*180 + 90; // The documentation does not cover this but the angle this spits out is between (-1..1). WHAT?! (also seems to be angle of feet)
		//while (heading > 180) {heading -= 360;} // Also, the basis seems to be off by 90 degrees
		//if (heading < 0) {heading = 360+heading;}
		
		delta = targetAngle - heading;
		while (delta > 180) {delta -= 360;} // There's no remainder function so I had to do this retarded BS
		while (delta < -180) {delta += 360;}
		
		GetInputController().OverrideAimChangeX(true, delta/1000.0);
		
		if (vector.Distance(GetPosition(), m_NextWaypoint) >  2 * m_FollowDistance) {
			GetInputController().OverrideMovementSpeed(true, 2.0);
		} else if (vector.Distance(GetPosition(), m_NextWaypoint) > m_FollowDistance) {
			GetInputController().OverrideMovementSpeed(true, 1.0);
		} else {
			GetInputController().OverrideMovementSpeed(true, 0.0);
		}
		
		GetInputController().OverrideMovementAngle(true, targetAngle*Math.DEG2RAD);//Math.PI/2.0);
		//m_MovementState.m_CommandTypeId = DayZPlayerConstants.COMMANDID_MOVE ;
		//m_MovementState.m_iMovement = 2;
	}
	
	void eAIUpdateBrain() { // This update needs to be done way less frequent than Movement; Default is 1 every 10 update ticks.
		// Todo we need to stagger this so not all AI are updated at once
	}
	
	/*override HumanInputController GetInputController() {
		return m_eAIController;
	}*/
};