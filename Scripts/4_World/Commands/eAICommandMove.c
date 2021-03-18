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

	private vector m_Direction;
	private float m_MovementDirection;
	private float m_TargetMovementDirection;
	
	private float m_Speed;
	private float m_TargetSpeed;
	private float m_SpeedMapping[8];

	private vector m_Transform[4];

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
		m_Entity.GetTransform(m_Transform);
		
		m_PreviousInteractionLayer = dBodyGetInteractionLayer(m_Entity);
		dBodySetInteractionLayer(m_Entity, PhxInteractionLayers.CHARACTER | PhxInteractionLayers.BUILDING | PhxInteractionLayers.DOOR | PhxInteractionLayers.VEHICLE | PhxInteractionLayers.ITEM_LARGE | PhxInteractionLayers.FENCE | PhxInteractionLayers.AI);
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
		//if (a - b > 0) return a - b;
		//return b - a;
		
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
		
		m_TargetMovementDirection = Math.NormalizeAngle(m_Direction.VectorToAngles()[0]);
		if (m_TargetMovementDirection > 180.0) m_TargetMovementDirection = m_TargetMovementDirection - 360.0;

		m_ST.SetMovementDirection(this, m_MovementDirection);
		
		m_ST.SetRaised(this, false);
	}

	override void PrePhysUpdate(float pDt)
	{
#ifndef SERVER
		m_Handler.ClearDebugShapes();
#endif

		if (m_Handler.PathCount() == 0)
		{
			m_Speed = 0;
			
			PrePhys_SetAngles(vector.Zero);
			PrePhys_SetTranslation(vector.Zero);
			
			return;
		}
		
		m_MaxTurnSpeed = 25.0;
		m_MaxTurnAcceleration = 10.0;
		
		vector expectedPosition = m_Entity.GetPosition() + (m_Direction * GetSpeedMS(m_Speed) * pDt);

		float wayPointDistance;
		int wayPointIndex = m_Handler.FindNext(expectedPosition, wayPointDistance);
		vector wayPoint = m_Handler.PathGet(wayPointIndex);
		wayPointDistance = vector.DistanceSq(Vector(expectedPosition[0], 0, expectedPosition[2]), Vector(wayPoint[0], 0, wayPoint[2]));

		bool isFinal = wayPointIndex == m_Handler.PathCount() - 1;
		
		if (!isFinal || (isFinal && wayPointDistance > 0.5))
		{
			m_PathAngle = Math.NormalizeAngle(m_Handler.AngleBetweenPoints(expectedPosition, wayPoint));
			
			float currentYaw = Math.NormalizeAngle(m_Entity.GetOrientation()[0]);
			if (m_PathAngle > 180.0) m_PathAngle = m_PathAngle - 360.0;
			if (currentYaw > 180.0) currentYaw = currentYaw - 360.0;
			
			float pathAngleDiff = ShortestAngle(m_PathAngle, currentYaw);
			m_TurnSpeed = Math.Clamp(pathAngleDiff, -m_MaxTurnSpeed, m_MaxTurnSpeed);
		} else
		{
			m_PathAngle = m_Entity.GetOrientation()[0];
			m_TurnSpeed = 0;
		}
		
		Vector(m_PathAngle, 0, 0).RotationMatrixFromAngles(m_Transform);
		m_Transform[3] = m_Entity.GetPosition();
		
		float angleDt = m_TurnSpeed * pDt * 10.0;
		
		m_TargetSpeed = 0.0;
		
		if (Math.AbsFloat(angleDt) < 0.125)
		{
			m_TargetSpeed = 3.0;
			angleDt = angleDt / 10.0;
		} else
		{
			m_TargetSpeed = 0;
		}
		
		if (isFinal && wayPointDistance < 0.5)
			m_TargetSpeed = 0.0;

		
		vector leftPos;
		vector rightPos;
		vector forwardPos;
		vector outNormal;
			
		bool blockedForward = m_Handler.PathBlocked(m_Transform[3], m_Transform[3] + (7.0 * pDt * m_Transform[2]), forwardPos, outNormal); // check forward
		float hitFraction = CheckPhysicsInFront();
		float forwardBlocking = hitFraction;
		if (blockedForward)
		{
			forwardBlocking *= vector.DistanceSq(m_Transform[3], forwardPos) / (49.0 * pDt);
		}
		
		m_Handler.PathBlocked(m_Transform[3], m_Transform[3] + (-5.0 * m_Transform[0]), leftPos, outNormal); // check the left
		m_Handler.PathBlocked(m_Transform[3], m_Transform[3] + (5.0 * m_Transform[0]), rightPos, outNormal); // check the right

		float leftDist = vector.DistanceSq(m_Transform[3], leftPos) / 25;
		float rightDist = vector.DistanceSq(m_Transform[3], rightPos) / 25;
		float minLRDist = Math.Min(leftDist, rightDist);
		float maxLRDist = Math.Max(leftDist, rightDist);

		m_TargetMovementDirection = 0.0;
		
		if (rightDist > leftDist && forwardBlocking < 0.9)
		{
			m_TargetMovementDirection = 90.0;
		}
		else if (leftDist < rightDist && forwardBlocking < 0.9)
		{
			m_TargetMovementDirection = -90.0;
		}
		
		m_MovementDirection += Math.Clamp((m_TargetMovementDirection - m_MovementDirection) * pDt, -180.0, 180.0);
		
		if (maxLRDist < 0.1)
		{
		//	m_TargetSpeed = 0.0;
		}
		
		float movementDirectionCorrected = m_MovementDirection;
		if (movementDirectionCorrected < 0) movementDirectionCorrected = 360.0 - movementDirectionCorrected;
		m_Direction = Vector(movementDirectionCorrected, 0, 0).AnglesToVector();

#ifndef SERVER
		m_Handler.AddShape(Shape.CreateSphere(0xFF0000FF, ShapeFlags.NOZBUFFER | ShapeFlags.WIREFRAME, m_Entity.ModelToWorld(m_Direction), 0.05));
		m_Handler.AddShape(Shape.CreateSphere(0xFFFF0000, ShapeFlags.NOZBUFFER | ShapeFlags.WIREFRAME, m_Entity.GetPosition() + m_Transform[2], 0.05));
		m_Handler.AddShape(Shape.CreateSphere(0xFF00FF00, ShapeFlags.NOZBUFFER | ShapeFlags.WIREFRAME, wayPoint, 0.05));
#endif
		
		float animationIndexAcceleration = Math.Clamp((m_TargetSpeed - m_Speed), -1000.0, 1.0) * pDt;
		m_Speed = Math.Clamp(m_Speed + animationIndexAcceleration, 0.0, 3.0);
		
		PrePhys_SetAngles(Vector(angleDt, 0, 0));
		PrePhys_SetTranslation(m_Direction * GetSpeedMS(m_Speed) * pDt);
	}

	override bool PostPhysUpdate(float pDt)
	{
		return true;
	}

	private float CheckPhysicsInFront()
	{
		vector CHECK_MIN_HEIGHT = "0 1.25 0";

		Object hitObject;
		vector hitPosition, hitNormal;
		float hitFraction;
		PhxInteractionLayers hit_mask = PhxInteractionLayers.CHARACTER | PhxInteractionLayers.DOOR | PhxInteractionLayers.VEHICLE | PhxInteractionLayers.ITEM_LARGE | PhxInteractionLayers.FENCE | PhxInteractionLayers.AI;
		bool hit = DayZPhysics.SphereCastBullet(m_Transform[3] + CHECK_MIN_HEIGHT, m_Transform[3] + (1.0 * m_Transform[2]) + CHECK_MIN_HEIGHT, 0.5, hit_mask, m_Entity, hitObject, hitPosition, hitNormal, hitFraction);
		hitFraction = 1.0 - hitFraction;
			
#ifndef SERVER
		int debugColour = 0xFF00AAFF;
		if (hit) debugColour = 0xFFAA00FF;
		vector points2[2];
		points2[0] = m_Transform[3] + CHECK_MIN_HEIGHT;
		points2[1] = m_Transform[3] + (1.0 * m_Transform[2]) + CHECK_MIN_HEIGHT;
		if (hit) points2[1] = hitPosition;
		m_Handler.AddShape(Shape.CreateLines(debugColour, ShapeFlags.NOZBUFFER, points2, 2));
#endif

		return hitFraction;
	}
};