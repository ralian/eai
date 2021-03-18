//todo: handle weapon firing from this command. In vanilla dayz, weapon firing doesn't work outside of HumanCommandMove anyways so there is no loss of functionality.

class eAICommandMove extends eAICommandBase
{
	private DayZPlayerImplement m_Entity;
    private eAIPlayerHandler m_Handler;
	private eAIAnimationST m_ST;

	private int m_PreviousInteractionLayer;
	
	private float m_PathAngle;
	
	private float m_TurnSpeed;
	private float m_TurnTargetSpeed;
	private float m_MaxTurnSpeed;
	private float m_MaxTurnAcceleration;
	
	private float m_Speed;
	private float m_TargetSpeed;
	private float m_SpeedMapping[8];

	void eAICommandMove(DayZPlayerImplement ai, eAIPlayerHandler handler, eAIAnimationST st)
	{
		m_Entity = ai;
        m_Handler = handler;
		m_ST = st;
	}
	
	void ~eAICommandMove()
	{
	}

	override void OnActivate()
	{
		m_PreviousInteractionLayer = dBodyGetInteractionLayer(m_Entity);
		dBodySetInteractionLayer(m_Entity, PhxInteractionLayers.AI);
	}

	override void OnDeactivate()
	{
		dBodySetInteractionLayer(m_Entity, m_PreviousInteractionLayer);
	}

	void SetSpeedMapping(int i, float speedMS)
	{
		int index = i * 2;
		m_SpeedMapping[ index ]		= i;
		m_SpeedMapping[ index + 1 ]	= speedMS;
	}

	float GetSpeedMS(float speedIdx)
	{
		int index = Math.Floor(speedIdx);
		if (index >= 3)
			return m_SpeedMapping[ 7 ];

		float a = m_SpeedMapping[ (index * 2) + 1 ];
		float b = m_SpeedMapping[ ((index + 1) * 2) + 1 ];

		return Math.Lerp(a, b, speedIdx - index);
	}

	float GetSpeedIndex(float speedMs)
	{
		int i = 0;
		while (i < 4)
		{
			if (m_SpeedMapping[ i * 2 + 1 ] > speedMs)
				break;

			++i;
		}

		if (i >= 3)
			return m_SpeedMapping[ 6 ];

		float a = m_SpeedMapping[ i * 2 + 1 ];
		float b = m_SpeedMapping[ (i + 1) * 2 + 1 ];

		return i + ((a - speedMs) / (a - b));
	}
	
	float ShortestAngle(float a, float b)
	{
		int aa = a;
		int bb = b;
		
		int phi = Math.AbsInt(aa - bb) % 360; 
		if (phi > 180) phi = 360 - phi;
		
		if ((a - b >= 0 && a - b <= 180) || (a - b <=-180 && a- b>= -360))
		{
			return phi;
		}
		
		return -phi;
	}

	override void PreAnimUpdate(float pDt)
	{
		SetSpeedMapping(0, 0.0);
		SetSpeedMapping(1, 2.0);
		SetSpeedMapping(2, 4.0);
		SetSpeedMapping(3, 7.0);

		m_ST.SetMovementSpeed(this, m_Speed);
		m_ST.SetMovementDirection(this, 0);
		
		m_ST.SetRaised(this, false);
	}

	override void PrePhysUpdate(float pDt)
	{
		if (m_Handler.PathCount() == 0)
		{
			m_Speed = 0;
			
			PrePhys_SetAngles(vector.Zero);
			PrePhys_SetTranslation(vector.Zero);
			
			return;
		}
		
		m_MaxTurnSpeed = 25.0;

		m_MaxTurnAcceleration = 10.0;

		vector transform[4];
		m_Entity.GetTransform(transform);

		vector movementDirection = "0 0 1";

		float wayPointDistance;
		int wayPointIndex = m_Handler.FindNext(m_Entity.GetPosition(), wayPointDistance);
		vector wayPoint = m_Handler.PathGet(wayPointIndex);
		wayPointDistance = vector.Distance(m_Entity.GetPosition(), wayPoint);
		
		m_PathAngle = Math.NormalizeAngle(m_Handler.AngleBetweenPoints(m_Entity.GetPosition(), wayPoint));
		float currentYaw = Math.NormalizeAngle(m_Entity.GetOrientation()[0]);
		if (m_PathAngle > 180.0) m_PathAngle = m_PathAngle - 360.0;
		if (currentYaw > 180.0) currentYaw = currentYaw - 360.0;
		
		float pathAngleDiff = ShortestAngle(m_PathAngle, currentYaw);
		
		m_TurnSpeed = Math.Clamp(pathAngleDiff, -m_MaxTurnSpeed, m_MaxTurnSpeed);
		
		float angleDt = m_TurnSpeed * pDt;
		
		if (Math.AbsFloat(pathAngleDiff) > 10.0)
		{
			m_TargetSpeed = 0.0;
			
			angleDt *= 40.0;
		} else
		{
			m_TargetSpeed = 3;
			
			angleDt *= 0.5;
		}
		
		if (wayPointDistance < 0.5) m_TargetSpeed = 0;
		
		float acceleration = Math.Clamp((m_TargetSpeed - m_Speed), -3.0 * 40.0, 1.0) * pDt;
		m_Speed = Math.Clamp(m_Speed + acceleration, 0.0, 3.0);
		
		float speedMs = GetSpeedMS(m_Speed);
		
		PrePhys_SetAngles(Vector(angleDt, 0, 0));
		PrePhys_SetTranslation(movementDirection * speedMs * pDt);
	}

	override bool PostPhysUpdate(float pDt)
	{
		return true;
	}
};