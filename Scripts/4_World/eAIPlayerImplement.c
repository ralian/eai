class eAIMovementState : HumanMovementState {
	void eAIMovementState() {
		Print("eAIMovementState: Init");
		//m_CommandTypeId = DayZPlayerConstants.COMMANDID_MOVE;
		//m_iMovement = 2;
	}
};


modded class DayZPlayerImplement {
	ref HumanMovementState m_MovementState = new eAIMovementState();
	//ref eAIHumanInputController m_eAIController = new eAIHumanInputController();
	
	Entity m_FollowOrders = null; // If this player id is > 0, they will follow.
	float m_FollowDistance = 5.0;
	vector m_NextWaypoint = "0 0 0"; // otherwise, default to waypoint.
	
	void DayZPlayerImplement() {
		Print("eAI DayZPlayerImplement: Init");
		m_IsFireWeaponRaised = false;
		m_SprintFull = false;
		m_SprintedTime = 0;
		m_AimingModel = new DayZPlayerImplementAiming(this);
		m_MeleeCombat = new DayZPlayerImplementMeleeCombat(this);
		m_MeleeFightLogic = new DayZPlayerMeleeFightLogic_LightHeavy(this);
		m_Swimming = new DayZPlayerImplementSwimming(this);		
		m_Throwing = new DayZPlayerImplementThrowing(this);
		m_JumpClimb = new DayZPlayerImplementJumpClimb(this);
		m_FallDamage = new DayZPlayerImplementFallDamage(this);
		m_bADS = false;
		m_WeaponRaiseCompleted = false;
		m_CameraEyeZoom = false;
		m_CameraOptics = false;
		m_IsShootingFromCamera = true;
		m_ProcessFirearmMeleeHit = false;
		m_ContinueFirearmMelee = false;
		m_WasIronsight = true;
		#ifdef PLATFORM_CONSOLE
		m_Camera3rdPerson = !GetGame().GetWorld().Is3rdPersonDisabled();
		#endif
		m_LastSurfaceUnderHash = ("cp_gravel").Hash();
		m_NextVoNNoiseTime = 0;
	}
	
	void eAIFollow(DayZPlayer p, float distance = 5.0) {
		m_FollowOrders = p;
		m_FollowDistance = distance;
	}
	
	float targetAngle, heading, delta;
	
	void eAIDebugMovement() {
		Print("targetAngle: " + targetAngle);
		Print("heading: " + heading);
		Print("delta: " + delta);
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
		
		GetInputController().OverrideAimChangeX(true, delta/3000.0);
		
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