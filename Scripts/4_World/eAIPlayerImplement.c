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
	
	void eAIUpdateMovement() {
		//m_MovementState.m_CommandTypeId = DayZPlayerConstants.COMMANDID_MOVE ;
		//m_MovementState.m_iMovement = 2;
	}
	
	/*override HumanInputController GetInputController() {
		return m_eAIController;
	}*/
};