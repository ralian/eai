class eAICommandVehicle extends eAICommandBase
{
	const int STATE_UNKNOWN = -1;
	const int STATE_AWAIT = 1;
	const int STATE_GETTING_IN = 2;
	const int STATE_GETTING_OUT = 3;
	const int STATE_JUMPING_OUT = 4;
	const int STATE_JUMPED_OUT = 5;
	const int STATE_SWITCHING_SEAT = 6;
	const int STATE_FINISH = 7;

	const float TIME_GET_IN = 1.5;
	const float TIME_GET_OUT = 1.5;
	const float TIME_JUMP_OUT = 2.0;
	const float TIME_SWITCH_SEAT = 1.0;

	private Transport m_Vehicle;
	private Car m_Car;

	private int m_SeatIndex;
	private int m_SeatAnim;
	private int m_VehicleType;
    private bool m_FromUnconscious;

	private int m_State;
	private int m_PreviousState;

	private float m_Time;
	private float m_TimeMax;
	
	private float m_TranslationSpeed;

	private vector m_StartTransform[4];
	private vector m_TargetTransform[4];

	private bool m_GearChanged;
	private bool m_ClutchState;
	private bool m_LeftVehicle;

	private bool m_KeepInVehicleSpaceAfterLeave;

	private bool m_Look;
	private float m_LookLR;
	private float m_LookUD;

	void eAICommandVehicle(eAIBase unit, eAIAnimationST st, Transport vehicle, int seatIdx, int seat_anim, bool fromUnconscious)
	{
		m_Vehicle = vehicle;
		m_Car = Car.Cast(m_Vehicle);

		m_SeatIndex = seatIdx;
		m_SeatAnim = seat_anim;

        m_FromUnconscious = fromUnconscious;
		
		m_State = STATE_UNKNOWN;
		m_PreviousState = STATE_UNKNOWN;
	}

	override void SetLookDirection(vector pDirection)
	{
		vector angles = pDirection.VectorToAngles();
		m_LookLR = angles[0];
		m_LookUD = angles[1];
		if (m_LookLR > 180) m_LookLR = m_LookLR - 360;
		if (m_LookUD > 180) m_LookUD = m_LookUD - 360;
		m_Look = (Math.AbsFloat(m_LookLR) > 0.01) || (Math.AbsFloat(m_LookUD) > 0.01);
	}

	private void ChangeState(int pState, vector pNewTransform[4])
	{
		m_Time = 0;
		m_State = pState;
		m_Unit.GetTransformWS(m_StartTransform);

		m_TargetTransform[0] = pNewTransform[0];
		m_TargetTransform[1] = pNewTransform[1];
		m_TargetTransform[2] = pNewTransform[2];
		m_TargetTransform[3] = pNewTransform[3];
	}

	private void LeaveVehicle()
	{
		if (m_LeftVehicle)
			return;

		m_LeftVehicle = true;

		m_Table.SetVehicleType(this, -1);

		m_Vehicle.CrewGetOut(m_SeatIndex);

		dBodyActive(m_Unit, ActiveState.ALWAYS_ACTIVE);
		dBodyEnableGravity(m_Unit, true);
		
		if (!m_KeepInVehicleSpaceAfterLeave)
		{
			vector tmPlayer[4];
			vector tmVehicle[4];
			vector tmTarget[4];
			m_Unit.GetTransformWS(tmPlayer);
			m_Vehicle.GetTransform(tmVehicle);
			Math3D.MatrixMultiply4(tmVehicle, tmPlayer, tmTarget);
			
			m_Unit.UnlinkFromLocalSpace();
			m_Unit.SetTransform(tmTarget);
		}
	}

	override void OnActivate()
	{
		m_LeftVehicle = false;

		if (!m_FromUnconscious || m_Unit.GetParent() != m_Vehicle)
		{
			vector tmPlayer[4];
			vector tmTarget[4];
			vector tmLocal[4];
			m_Unit.GetTransformWS(tmPlayer);
			m_Vehicle.GetTransform(tmTarget);
			Math3D.MatrixInvMultiply4(tmTarget, tmPlayer, tmLocal);
			
			m_Unit.LinkToLocalSpaceOf(m_Vehicle, tmLocal);
		}

		m_Vehicle.CrewTransform(m_SeatIndex, m_TargetTransform);
		m_Vehicle.CrewGetIn(m_Unit, m_SeatIndex);

		m_Table.SetVehicleType(this, m_VehicleType);

		dBodyEnableGravity(m_Unit, false);
		dBodyActive(m_Unit, ActiveState.INACTIVE);

		m_Unit.OnCommandVehicleAIStart();

		ChangeState(STATE_GETTING_IN, m_TargetTransform);
	}

	override void OnDeactivate()
	{
		LeaveVehicle();

		m_Unit.OnCommandVehicleAIFinish();
	}

	override void PreAnimUpdate(float pDt)
	{
		PreAnim_SetFilteredHeading(0, 0.3, 180);

		m_Table.SetLook(this, m_Look);
		m_Table.SetLookDirX(this, m_LookLR);
		m_Table.SetLookDirY(this, m_LookUD);

		m_Table.SetVehicleType(this, m_VehicleType);
		m_Table.SetVehicleSteering(this, m_Car.GetController().GetSteering() + 0);
		m_Table.SetVehicleThrottle(this, m_Car.GetController().GetThrust() + 0);
		m_Table.SetVehicleClutch(this, m_ClutchState);
		m_Table.SetVehicleBrake(this, m_Car.GetController().GetBrake() != 0.0);
		
		switch (m_State)
		{
			case STATE_GETTING_IN:
				m_TimeMax = TIME_GET_IN;
				break;
			case STATE_GETTING_OUT:
				m_TimeMax = TIME_GET_OUT;
				break;
			case STATE_JUMPED_OUT:
			case STATE_JUMPING_OUT:
				m_TimeMax = TIME_JUMP_OUT;
				break;
			case STATE_SWITCHING_SEAT:
				m_TimeMax = TIME_SWITCH_SEAT;
				break;
		}

		if (m_State != m_PreviousState)
		{
			m_TranslationSpeed = vector.Distance(m_StartTransform[3], m_TargetTransform[3]) / m_TimeMax;
			
			switch (m_State)
			{
				case STATE_GETTING_IN:
					m_Table.CallVehicleGetIn(this, m_SeatAnim);
					break;
				case STATE_GETTING_OUT:
					m_Table.CallVehicleGetOut(this);
					break;
				case STATE_JUMPING_OUT:
					m_Table.CallVehicleJumpOut(this);
					break;
				case STATE_SWITCHING_SEAT:
					m_Table.CallVehicleSwitchSeat(this, m_SeatAnim);
					break;
			}
		}
	}

	override void PrePhysUpdate(float pDt)
	{
		if (m_State == STATE_JUMPED_OUT)
		{
			return;
		}
		
		vector translation;
		float rotation[4]
		
		if (m_State == STATE_JUMPING_OUT && m_Table.IsLeaveVehicle(this))
		{
			m_State = STATE_JUMPED_OUT;
			
			LeaveVehicle();
			
			return;
		}
		
		float speedT = m_TranslationSpeed;

		vector tmPlayer[4];
		m_Unit.GetTransformWS(tmPlayer);
		translation = vector.Direction(tmPlayer[3], m_TargetTransform[3]);
		translation = translation.InvMultiply3(m_TargetTransform);
		
		float lenT = translation.Normalize();
		if (lenT < pDt) speedT *= lenT;
		
		translation = translation * speedT * pDt;
		
		Math3D.MatrixToQuat(m_TargetTransform, rotation);
		
		PrePhys_SetRotation(rotation);
		PrePhys_SetTranslation(translation);
	}

	override bool PostPhysUpdate(float pDt)
	{
		m_Time += pDt;

		switch (m_State)
		{
			case STATE_GETTING_IN:
				if (m_Time > TIME_GET_IN)
					m_State = STATE_AWAIT;
				break;
			case STATE_GETTING_OUT:
				if (m_Time > TIME_GET_OUT)
					m_State = STATE_FINISH;
				break;
			case STATE_JUMPED_OUT:
			case STATE_JUMPING_OUT:
				if (m_Time > TIME_JUMP_OUT)
					m_State = STATE_FINISH;
				break;
			case STATE_SWITCHING_SEAT:
				if (m_Time > TIME_SWITCH_SEAT)
					m_State = STATE_AWAIT;
				break;
		}
		
		m_PreviousState = m_State;

		return m_State != STATE_FINISH;
	}

	void GetOutVehicle()
	{
		vector pos;
		vector dir;
		m_Vehicle.CrewEntry(m_SeatIndex, pos, dir);
		Math3D.DirectionAndUpMatrix(dir, vector.Up, m_TargetTransform);
		m_TargetTransform[3] = pos;

		ChangeState(STATE_GETTING_OUT, m_TargetTransform);
	}

	void JumpOutVehicle()
	{		
		vector pos;
		vector dir;
		m_Vehicle.CrewEntry(m_SeatIndex, pos, dir);
		Math3D.DirectionAndUpMatrix(dir, vector.Up, m_TargetTransform);
		m_TargetTransform[3] = pos;

		ChangeState(STATE_JUMPING_OUT, m_TargetTransform);
	}

	void SwitchSeat(int seatIdx, int seatAnimType)
	{
		m_SeatAnim = seatAnimType;

		m_Vehicle.CrewGetOut(m_SeatIndex);

		m_SeatIndex = seatIdx;

		m_Vehicle.CrewTransform(m_SeatIndex, m_TargetTransform);
		m_Vehicle.CrewGetIn(m_Unit, m_SeatIndex);

		ChangeState(STATE_SWITCHING_SEAT, m_TargetTransform);
	}

	bool IsGettingOut()
	{
		return m_State == STATE_GETTING_OUT || m_State == STATE_JUMPING_OUT;
	}

	bool IsGettingIn()
	{
		return m_State == STATE_GETTING_IN;
	}

	bool IsSwitchSeat()
	{
		return m_State == STATE_SWITCHING_SEAT;
	}

	Transport GetTransport()
	{
		return m_Vehicle;
	}

	int GetVehicleSeat()
	{
		return m_SeatIndex;
	}

	void SetVehicleType(int vehicleType)
	{
		m_VehicleType = vehicleType;
	}

	void KeepInVehicleSpaceAfterLeave(bool pState)
	{
		m_KeepInVehicleSpaceAfterLeave = pState;
	}

	void SignalGearChange()
	{
		m_GearChanged = true;
	}

	bool WasGearChange()
	{
		bool changed = m_GearChanged & !m_ClutchState;
		m_GearChanged = false;
		return changed;
	}

	void SetClutchState(bool pState)
	{
		m_ClutchState = pState;
	}
};