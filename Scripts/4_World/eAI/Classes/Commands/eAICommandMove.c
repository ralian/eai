class eAICommandMove extends eAICommandBase
{
	private int m_PreviousInteractionLayer;
	
	private float m_PathAngle;
	
	private float m_TurnSpeed;
	private float m_TurnTargetSpeed;
	private float m_MaxTurnSpeed;
	private float m_MaxTurnAcceleration;

	private vector m_Direction;
	private float m_MovementDirection;
	private float m_TargetMovementDirection;

	private bool m_Look;
	private float m_LookLR;
	private float m_LookUD;
	
	private bool m_Raised;
	
	private float m_Speed;
	private float m_PhysicsSpeed;
	private float m_TargetSpeed;
	private float m_SpeedLimit;
	
	private float m_LastSpeedChangeTime;
	
	private const float WALK_SPEED = 2.0;
	private const float RUN_SPEED = 4.0;
	private const float SPRINT_SPEED = 7.0;
	
	private int m_ChangeCounter = 0;

	private vector m_Transform[4];

	void eAICommandMove(eAIBase unit, eAIAnimationST st)
	{
	}
	
	void ~eAICommandMove()
	{
	}

	override void OnActivate()
	{
		SetSpeedLimit(-1);
		
		m_Unit.GetTransform(m_Transform);
		
		dBodySetInteractionLayer(m_Unit, PhxInteractionLayers.CHARACTER | PhxInteractionLayers.BUILDING | PhxInteractionLayers.DOOR | PhxInteractionLayers.VEHICLE | PhxInteractionLayers.ITEM_LARGE | PhxInteractionLayers.FENCE | PhxInteractionLayers.AI);
	}

	override void OnDeactivate()
	{
	}

	override void SetLookDirection(vector direction)
	{
		vector angles = direction.VectorToAngles();
		m_LookLR = angles[0];
		m_LookUD = angles[1];
		if (m_LookLR > 180) m_LookLR = m_LookLR - 360;
		if (m_LookUD > 180) m_LookUD = m_LookUD - 360;
		m_Look = (Math.AbsFloat(m_LookLR) > 0.01) || (Math.AbsFloat(m_LookUD) > 0.01);
	}

	void SetSpeedLimit(float speedIdx)
	{
		m_SpeedLimit = speedIdx;

		if (m_SpeedLimit < 0 || m_SpeedLimit > 3) m_SpeedLimit = 3;
	}
	
	void SetTargetSpeed(float target)
	{
		if (m_LastSpeedChangeTime < 0.5) return;
		
		m_LastSpeedChangeTime = 0;
		m_TargetSpeed = target;
	}
	
	void SetRaised(bool raised)
	{
		m_Raised = raised;
	}
	
	float ShortestAngle(float a, float b)
	{
		if (a - b > 0) return a - b;
		return b - a;
		
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
		m_Table.SetMovementSpeed(this, m_Speed);
		
		m_TargetMovementDirection = Math.NormalizeAngle(m_Direction.VectorToAngles()[0]);
		if (m_TargetMovementDirection > 180.0) m_TargetMovementDirection = m_TargetMovementDirection - 360.0;

		m_Table.SetMovementDirection(this, m_MovementDirection);
		
		//m_ST.SetAimX(this, false);
		//if (m_Raised)
		//	m_ST.SetAimY(this, -15.0);
	}

	override void PrePhysUpdate(float pDt)
	{
		m_LastSpeedChangeTime += pDt;

		m_MaxTurnSpeed = 25.0;
		m_MaxTurnAcceleration = 10.0;
		
		vector expectedPosition = m_Unit.ModelToWorld(m_Direction * m_PhysicsSpeed * pDt);
		
		// First, if we are in combat, just do turn
		float currentYaw, pathAngleDiff;
		if (m_Raised && m_Unit.threats.Count() > 0 && m_Unit.threats[0])
		{
			m_PathAngle = Math.NormalizeAngle(m_Unit.AngleBetweenPoints(expectedPosition, m_Unit.threats[0].GetPosition()));
			
			currentYaw = Math.NormalizeAngle(m_Unit.GetOrientation()[0]);
			if (m_PathAngle > 180.0) m_PathAngle = m_PathAngle - 360.0;
			if (currentYaw > 180.0) currentYaw = currentYaw - 360.0;
			
			pathAngleDiff = ShortestAngle(m_PathAngle, currentYaw);
			m_TurnSpeed = Math.Clamp(pathAngleDiff, -m_MaxTurnSpeed, m_MaxTurnSpeed);
			
			PrePhys_SetAngles(Vector(m_TurnSpeed * pDt * 10.0, 0, 0));
			
			m_Speed = 0;
			PrePhys_SetTranslation(vector.Zero);
			
			//Print(pathAngleDiff.ToString() + " " + m_TurnSpeed.ToString());
			return;
		}
		
		if (m_Unit.PathCount() == 0)
		{
			m_Speed = 0;
			
			PrePhys_SetAngles(vector.Zero);
			PrePhys_SetTranslation(vector.Zero);
			
			return;
		}

		float wayPointDistance;
		int wayPointIndex = m_Unit.FindNext(expectedPosition, wayPointDistance);
		vector wayPoint = m_Unit.PathGet(wayPointIndex);
		wayPointDistance = Math.Pow(wayPoint[0]-expectedPosition[0],2) + Math.Pow(wayPoint[2]-expectedPosition[2],2);

		bool isFinal = wayPointIndex == m_Unit.PathCount() - 1;
		
		if (!isFinal || (isFinal && wayPointDistance > 0.5))
		{			
			vector pathDir = m_Unit.WorldToModel(wayPoint).Normalized();
			pathAngleDiff = pathDir.VectorToAngles()[0];
			if (pathAngleDiff > 180.0) pathAngleDiff = pathAngleDiff - 360.0;
			
			m_TurnSpeed = Math.Clamp(pathAngleDiff, -m_MaxTurnSpeed, m_MaxTurnSpeed);
			m_PathAngle = m_Unit.GetOrientation()[0] + m_TurnSpeed;
		}
		else
		{
			m_PathAngle = m_Unit.GetOrientation()[0];
			m_TurnSpeed = 0;
		}
		
		Vector(m_PathAngle, 0, 0).RotationMatrixFromAngles(m_Transform);
		m_Transform[3] = m_Unit.GetPosition();
		
		float angleDt = m_TurnSpeed * pDt * 10.0;
		
		
		if (Math.AbsFloat(angleDt) < 0.125 * m_PhysicsSpeed)
		{			
			if (isFinal)
			{
				if (wayPointDistance < 1.0)
					SetTargetSpeed(0.0);
				else if (wayPointDistance < 8.0)
					SetTargetSpeed(1.0);
				else if (wayPointDistance < 20.0)
					SetTargetSpeed(2.0);
				else 
					SetTargetSpeed(3.0);
			}
			else
				SetTargetSpeed(3.0);
			
			angleDt = angleDt / 10.0;
		}
		else
		{
			//! turn in place - todo: perform animation
			SetTargetSpeed(0.0);
		}
		
		if (m_Raised) SetSpeedLimit(1);
		
		
		vector leftPos;
		vector rightPos;
		vector forwardPos;
		vector outNormal;
			
		bool blockedForward = m_Unit.PathBlocked(m_Transform[3], m_Transform[3] + (7.0 * pDt * m_Transform[2]), forwardPos, outNormal); // check forward
		float hitFraction = CheckPhysicsInFront();
		float forwardBlocking = hitFraction;
		if (blockedForward)
		{
			forwardBlocking *= vector.DistanceSq(m_Transform[3], forwardPos) / (49.0 * pDt);
		}
		
		m_Unit.PathBlocked(m_Transform[3], m_Transform[3] + (-5.0 * m_Transform[0]), leftPos, outNormal); // check the left
		m_Unit.PathBlocked(m_Transform[3], m_Transform[3] + (5.0 * m_Transform[0]), rightPos, outNormal); // check the right

		float leftDist = vector.DistanceSq(m_Transform[3], leftPos) / 25;
		float rightDist = vector.DistanceSq(m_Transform[3], rightPos) / 25;
		float minLRDist = Math.Min(leftDist, rightDist);
		float maxLRDist = Math.Max(leftDist, rightDist);

		m_TargetMovementDirection = 0.0;
		
		if (forwardBlocking < 0.9)
		{
			if (rightDist > leftDist)
			{
				m_TargetMovementDirection = 90.0;
			}
			else if (leftDist < rightDist)
			{
				m_TargetMovementDirection = -90.0;
			}
		}
		
		m_MovementDirection += Math.Clamp((m_TargetMovementDirection - m_MovementDirection) * pDt, -180.0, 180.0);
				
		float movementDirectionCorrected = m_MovementDirection;
		if (movementDirectionCorrected < 0) movementDirectionCorrected = 360.0 - movementDirectionCorrected;
		m_Direction = Vector(movementDirectionCorrected, 0, 0).AnglesToVector();

#ifndef SERVER
		m_Unit.AddShape(Shape.CreateSphere(0xFF0000FF, ShapeFlags.NOZBUFFER | ShapeFlags.WIREFRAME, m_Unit.ModelToWorld(m_Direction), 0.05));
		m_Unit.AddShape(Shape.CreateSphere(0xFFFF0000, ShapeFlags.NOZBUFFER | ShapeFlags.WIREFRAME, m_Unit.GetPosition() + m_Transform[2], 0.05));
		m_Unit.AddShape(Shape.CreateSphere(0xFF00FF00, ShapeFlags.NOZBUFFER | ShapeFlags.WIREFRAME, wayPoint, 0.05));
#endif
		float speedCheck = m_TargetSpeed;
		if (speedCheck > m_SpeedLimit && m_SpeedLimit != -1) speedCheck = m_SpeedLimit;
		
		m_PhysicsSpeed = 0.0;
		m_Speed = 0;
		
		if (speedCheck == 1)
		{
			m_Speed = 1;
			m_PhysicsSpeed = WALK_SPEED;
		} else if (speedCheck == 2)
		{
			m_Speed = 2;
			m_PhysicsSpeed = RUN_SPEED;
		} else if (speedCheck > 2)
		{
			m_Speed = Math.Clamp(speedCheck, 2, 3);
			m_PhysicsSpeed = Math.Lerp(RUN_SPEED, SPRINT_SPEED, m_Speed - 2);
		}
						
		PrePhys_SetAngles(Vector(angleDt, 0, 0));
		PrePhys_SetTranslation(m_Direction * m_PhysicsSpeed * pDt);
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
		bool hit = DayZPhysics.SphereCastBullet(m_Transform[3] + CHECK_MIN_HEIGHT, m_Transform[3] + (1.0 * m_Transform[2]) + CHECK_MIN_HEIGHT, 0.5, hit_mask, m_Unit, hitObject, hitPosition, hitNormal, hitFraction);
		hitFraction = 1.0 - hitFraction;
			
#ifndef SERVER
		int debugColour = 0xFF00AAFF;
		if (hit) debugColour = 0xFFAA00FF;
		vector points2[2];
		points2[0] = m_Transform[3] + CHECK_MIN_HEIGHT;
		points2[1] = m_Transform[3] + (1.0 * m_Transform[2]) + CHECK_MIN_HEIGHT;
		if (hit) points2[1] = hitPosition;
		m_Unit.AddShape(Shape.CreateLines(debugColour, ShapeFlags.NOZBUFFER, points2, 2));
#endif

		return hitFraction;
	}
};