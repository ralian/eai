enum eAITargetOverriding
{
	NONE,
	POSITION,
	PATH
};

class eAIPathFinding
{
	private eAIBase m_Unit;

	private float m_Time;

	private bool m_InVehicle;
	private bool m_IsDriving;

	private ref PGFilter m_PathFilter;
	private ref array<vector> m_Path;
	private vector m_TargetPosition;
	private eAITargetOverriding m_TargetOverriding;

	private AIWorld m_AIWorld;
	private eAIRoadNetwork m_RoadNetwork;

	void eAIPathFinding()
	{
		m_Path = new array<vector>();
		m_PathFilter = new PGFilter();

		m_AIWorld = GetGame().GetWorld().GetAIWorld();
		m_RoadNetwork = eAIManagerImplement.Get().GetRoadNetwork();

		SetPathFilter();
	}

	private void SetPathFilter()
	{
		int inFlags = PGPolyFlags.WALK | PGPolyFlags.DOOR | PGPolyFlags.INSIDE | PGPolyFlags.JUMP_OVER;
		int exFlags = PGPolyFlags.DISABLED | PGPolyFlags.SWIM | PGPolyFlags.SWIM_SEA | PGPolyFlags.SPECIAL | PGPolyFlags.JUMP | PGPolyFlags.CLIMB | PGPolyFlags.CRAWL | PGPolyFlags.CROUCH;

		m_PathFilter.SetFlags(inFlags, exFlags, PGPolyFlags.NONE);
	}

	void OnUpdate(float pDt, int pSimulationPrecision)
	{
		m_Time += pDt;

		float reqTime = pSimulationPrecision * (pSimulationPrecision + 1) * 0.025;
		if (m_Time < reqTime) return;
		m_Time = 0;

		SetPathFilter();

		if (m_TargetOverriding != eAITargetOverriding.PATH)
		{
			m_Path.Clear();
			
			if (m_PathFilter)
			{
				if (m_TargetOverriding != eAITargetOverriding.POSITION && m_eAI_Targets.Count() > 0)
				{
					eAITarget target = m_eAI_Targets[0];
					if (target.HasInfo()) 
						m_TargetPosition = target.GetPosition(this);
				}

				vector modifiedTargetPosition = m_TargetPosition;
				if (vector.DistanceSq(GetPosition(), m_TargetPosition) > (50 * 50))
				{
					m_RoadNetwork.Find(GetPosition(), m_TargetPosition, m_Path);
					vector start = m_Path[0];
					vector end = m_Path[m_Path.Count() - 1];
					world.FindPath(GetPosition(), start, m_PathFilter, m_Path);
					world.FindPath(end, m_TargetPosition, m_PathFilter, m_Path);
				}
				else
				{
					world.FindPath(GetPosition(), modifiedTargetPosition, m_PathFilter, m_Path);
				}
			}
		}
	}

	vector GetTargetPosition()
	{
		return m_TargetPosition;
	}

	void OverridePath()
	{
		//eAITrace trace(this, "OverridePath");

		m_TargetOverriding = eAITargetOverriding.PATH;
		m_Path.Clear();
	}

	void OverridePath(array<vector> pPath)
	{
		//eAITrace trace(this, "OverridePath", pPath.ToString());

		m_TargetOverriding = eAITargetOverriding.PATH;
		pPath.Copy(m_Path);
	}

	void OverridePosition(vector pPosition)
	{
		//eAITrace trace(this, "OverridePosition", pPosition.ToString());
		
		m_TargetOverriding = eAITargetOverriding.POSITION;
		m_TargetPosition = pPosition;
	}
	
	float Distance(int index, vector position)
	{
		vector begin = m_Path[index];
		vector end = m_Path[index + 1] - begin;
		vector relative = position - begin;
		float eSize2 = end.LengthSq();
		if (eSize2 > 0)
		{
			float time = (end * relative) / eSize2;
			vector nearest = begin + Math.Clamp(time, 0, 1) * end;
			return vector.DistanceSq(nearest, position);
		}

		return vector.DistanceSq(begin, position);
	}

	void Clear()
	{
		m_Path.Clear();
	}
	
	int Count()
	{
		return m_Path.Count();
	}
	
	vector Get(int index)
	{		
		return m_Path[index];
	}
	
	bool IsBlocked(vector start, vector end)
	{
		vector hitPos;
		vector hitNormal;
		
		return m_AIWorld.RaycastNavMesh(start, end, m_PathFilter, hitPos, hitNormal);
	}
	
	bool IsBlocked(vector start, vector end, out vector hitPos, out vector hitNormal)
	{
		return m_AIWorld.RaycastNavMesh(start, end, m_PathFilter, hitPos, hitNormal);
	}

	int Next(vector position)
	{
		int index = 0;
		float minDist = 1000000000.0;

		float epsilon = -0.5;
		for (int i = 0; i < m_Path.Count() - 1; ++i)
		{
			float dist = Distance(i, position);
			
			if (minDist >= dist - epsilon)
			{
				if (!IsBlocked(position, m_Path[i + 1]))
				{
					minDist = dist;
					index = i;
				}
			}
		}

		return index + 1;
	}
};