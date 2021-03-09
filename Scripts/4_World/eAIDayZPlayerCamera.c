// *************************************************************************************
// ! DayZPlayerCamera1stPerson - Looking control for bots
// *************************************************************************************
class eAIDayZPlayerCamera extends DayZPlayerCameraBase
{
	static const float 	CONST_UD_MIN	= -85.0;		//!< down limit
	static const float 	CONST_UD_MAX	= 85.0;			//!< up limit

	static const float 	CONST_LR_MIN	= -160.0;		//!< down limit
	static const float 	CONST_LR_MAX	= 160.0;		//!< up limit
	
	override float UpdateUDAngle(out float pAngle, out float pAngleAdd, float pMin, float pMax, float pDt)
	{
		pAngle = 0;
		pAngleAdd = 0;
		return 0;
		
		
		/*{
			float change = m_pInput.GetAimChange()[1] * Math.RAD2DEG;

			Print ("Script: Camera ud angle: " +  pAngle.ToString() + " change: " + change.ToString() );
		}*/		

		//return Limit(pAngle + pAngleAdd, pMin, pMax);
	}


	void 	DayZPlayerCamera1stPerson(DayZPlayer pPlayer, HumanInputController pInput)
	{
		m_iBoneIndex = pPlayer.GetBoneIndexByName("Head");
		if (m_iBoneIndex == -1)
		{
			Print("DayZPlayerCamera1stPerson: main bone not found");
		}
		
		m_OffsetLS = "0.04 0.04 0"; //x - up, y - forward
	}
	

	//
	override void 		OnActivate (DayZPlayerCamera pPrevCamera, DayZPlayerCameraResult pPrevCameraResult)
	{
		super.OnActivate(pPrevCamera, pPrevCameraResult);
		if (pPrevCamera)
		{
			vector 	f = pPrevCamera.GetBaseAngles();
			m_fUpDownAngle		= f[0]; 
			m_fLeftRightAngle	= f[1]; 
			m_fUpDownAngleAdd	= f[2];
		}
	}

	//	we may be able to safely skip all of this
	override void 		OnUpdate(float pDt, out DayZPlayerCameraResult pOutResult)
	{
		Print("Updating camera!");
		//! update angles from input 
		float 	udAngle 	= UpdateUDAngle(m_fUpDownAngle, m_fUpDownAngleAdd, CONST_UD_MIN, CONST_UD_MAX, pDt);
		m_CurrentCameraPitch = udAngle;
		m_fLeftRightAngle 	= UpdateLRAngle(m_fLeftRightAngle, CONST_LR_MIN, CONST_LR_MAX, pDt);

		
		//! 
		vector rot;
		rot[0] = m_fLeftRightAngle;
		rot[1] = udAngle;
		rot[2] = 0;
		
		Math3D.YawPitchRollMatrix(rot, pOutResult.m_CameraTM);
		pOutResult.m_CameraTM[3]		= m_OffsetLS;

		pOutResult.m_iDirectBone 		= m_iBoneIndex;
		
		// Here we also want to overwrite the rotation of the head bone.
		pOutResult.m_iDirectBoneMode 	= 3;
		
		pOutResult.m_fUseHeading 		= 1.0;
		pOutResult.m_fInsideCamera 		= 1.0;
		
		// Shoot from camera optic instead of barrel as a workaround?
		pOutResult.m_fShootFromCamera = 1;

		//super.OnUpdate(pDt, pOutResult);
		
		pOutResult.m_fNearPlane = 0.04; //0.07 default
		if(m_CameraShake)
		{
			float x,y;
			m_CameraShake.Update(pDt, x, y);
			//m_fLeftRightAngle += x;
			//m_fUpDownAngleAdd += y;
			//Print(x);
		}
		//Print("1st person near plane = " + pOutResult.m_fNearPlane);
	}

	//
	
	override vector GetBaseAngles()
	{
		vector a;
		a[0] = m_fUpDownAngle;
		a[1] = m_fLeftRightAngle;
		a[2] = m_fUpDownAngleAdd;
		return a;
	}
	
	override string GetCameraName()
	{
		return "DayZPlayerCamera1stPerson";
	}
	
	protected	int 	m_iBoneIndex;		//!< main bone
	protected	vector	m_OffsetLS;			//!< position offset 

	protected 	float 	m_fUpDownAngle;		//!< up down angle in rad
	protected 	float 	m_fUpDownAngleAdd;	//!< up down angle in rad
	protected 	float 	m_fLeftRightAngle;	//!< left right angle in rad (in freelook only)
};

// *************************************************************************************
// ! DayZPlayerCamera1stPersonUnconscious - first person only unconscious
// *************************************************************************************
class eAIDayZPlayerCamera1stPersonUnconscious extends DayZPlayerCamera1stPerson
{
	void DayZPlayerCamera1stPersonUnconscious(DayZPlayer pPlayer, HumanInputController pInput)
	{
		m_iBoneIndex		= pPlayer.GetBoneIndexByName ("Head");
	}
	
	override void OnUpdate(float pDt, out DayZPlayerCameraResult pOutResult)
	{
		super.OnUpdate(pDt, pOutResult);

		pOutResult.m_iDirectBone 		= m_iBoneIndex;
		pOutResult.m_iDirectBoneMode 	= 4;
		pOutResult.m_fUseHeading 		= 0.0;
		
		vector rot;
		rot[0] = 0;
		rot[1] = 90;
		rot[2] = 0;
		
		Math3D.YawPitchRollMatrix(rot, pOutResult.m_CameraTM);
	}
};
