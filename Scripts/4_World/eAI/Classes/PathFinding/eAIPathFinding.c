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

	private ref array<vector> m_RoadPath;
	private ref array<vector> m_RoadPathSwap;
	private bool m_RoadPathReady;
	private bool m_IsFindingRoadPath;

	private vector m_Position;

	private vector m_OverridePosition;
	private eAITargetOverriding m_Overriding;

	private AIWorld m_AIWorld;
	private eAIRoadNetwork m_RoadNetwork;

	void eAIPathFinding(eAIBase unit)
	{
		m_Unit = unit;
		
		m_Path = new array<vector>();
		m_PathFilter = new PGFilter();

		m_RoadPath = new array<vector>();
		m_RoadPathSwap = new array<vector>();

		m_AIWorld = GetGame().GetWorld().GetAIWorld();
		m_RoadNetwork = eAIManagerImplement.Get4().GetRoadNetwork();

		SetPathFilter();
	}

	private void SetPathFilter()
	{
		int inFlags = PGPolyFlags.WALK | PGPolyFlags.DOOR | PGPolyFlags.INSIDE | PGPolyFlags.JUMP_OVER;
		int exFlags = PGPolyFlags.DISABLED | PGPolyFlags.SWIM | PGPolyFlags.SWIM_SEA | PGPolyFlags.SPECIAL | PGPolyFlags.JUMP | PGPolyFlags.CLIMB | PGPolyFlags.CRAWL | PGPolyFlags.CROUCH;

		m_PathFilter.SetFlags(inFlags, exFlags, PGPolyFlags.NONE);
		m_PathFilter.SetCost(PGAreaType.OBJECTS, 1.0);
		m_PathFilter.SetCost(PGAreaType.TERRAIN, 1.0);
		m_PathFilter.SetCost(PGAreaType.BUILDING, 1.0);
	}

	void StartRoadPath(out array<vector> path)
	{
		//Print("StartRoadPath");
		//m_RoadPathSwap.Clear();
		//path = m_RoadPathSwap;
		//m_IsFindingRoadPath = true;
	}

	//! temp var
	//ref array<PathNode> m_RoadNodes = new array<PathNode>();
	void FinishRoadPath(array<vector> path, array<PathNode> nodes)
	{		
	//	//Print("FinishRoadPath");
	//	m_RoadPathSwap = path;
	//	m_RoadPathReady = true;
	//	m_IsFindingRoadPath = false;
	//	
	//	m_RoadNodes.Clear();
	//	m_RoadNodes.Copy(nodes);
	}

	void OnDebug()
	{
		#ifdef EAI_DEBUG_PATH
		//CF_DebugUI_Block dbg;
		//Class.CastTo(dbg, CF.DebugUI.Get("PATH", m_Unit));
		//dbg.Clear();
		
		int i;
		vector points[2];
		
		for (i = 0; i < m_Path.Count(); i++)
		{
			//dbg.Set("[" + i + "]", m_Path[i]);

			if (i != m_Path.Count() - 1)
			{
				#ifndef SERVER
				points[0] = m_Path[i];
				points[1] = m_Path[i + 1];
				m_Unit.AddShape(Shape.CreateLines(0xFF00FF00, ShapeFlags.VISIBLE | ShapeFlags.NOZBUFFER, points, 2));
				#endif
			}
		}
		for (i = 0; i < m_RoadNodes.Count(); i++)
		{
			//dbg.Set("[" + i + "]", m_Path[i]);

			if (i != m_RoadNodes.Count() - 1)
			{
				#ifndef SERVER
				points[0] = m_RoadNodes[i].m_Position + "0 0.5 0";
				points[1] = m_RoadNodes[i + 1].m_Position + "0 0.5 0";
				m_Unit.AddShape(Shape.CreateLines(0xFF00FFFF, ShapeFlags.VISIBLE | ShapeFlags.NOZBUFFER, points, 2));
				#endif
			}
		}
		#endif
	}
	
	void OnUpdate(float pDt, int pSimulationPrecision)
	{
		OnDebug();
		
		m_Time += pDt;

		//if (m_RoadPathReady)
		//{
		//	m_RoadPath.Clear();
		//	m_RoadPath.Copy(m_RoadPathSwap);
		//	m_RoadPathSwap.Clear();
		//	m_RoadPathReady = false;
		//}

		float reqTime = pSimulationPrecision * (pSimulationPrecision + 1) * 0.025;
		if (m_Time < reqTime) return;
		m_Time = 0;

		//SetPathFilter();

		if (m_Overriding != eAITargetOverriding.PATH)
		{
			m_Path.Clear();
			
			if (m_PathFilter)
			{
				if (m_Overriding == eAITargetOverriding.POSITION) m_Position = m_OverridePosition;

				m_AIWorld.FindPath(m_Unit.GetPosition(), m_Position, m_PathFilter, m_Path);

/*
				bool generatedPath = false;
				const float maxNavMeshFindDistance = 50.0;
				if (vector.DistanceSq(m_Unit.GetPosition(), m_Position) > (maxNavMeshFindDistance * maxNavMeshFindDistance))
				{
					//Print("+eAIPathFinding::OnUpdate");
					if (!m_IsFindingRoadPath) m_RoadNetwork.FindPath(m_Unit.GetPosition(), m_Position, this);
					//Print("-eAIPathFinding::OnUpdate");

					if (m_RoadPath.Count() >= 2)
					{
						generatedPath = true;

						vector end = m_RoadPath[m_RoadPath.Count() - 1];
						vector start = m_RoadPath[1];

						//m_AIWorld.FindPath(m_Unit.GetPosition(), start, m_PathFilter, m_Path);
						for (int j = 0; j < m_RoadPath.Count() - 1; j++)
						{
							m_Path.Insert(m_RoadPath[j]);
						}
						//m_AIWorld.FindPath(end, m_Position, m_PathFilter, m_Path);
					}

					//vector direction = vector.Direction(m_Unit.GetPosition(), m_Position).Normalized();
					//m_Position = m_Unit.GetPosition() + (direction * maxNavMeshFindDistance);

					m_Time = -1;
				}
				if (!generatedPath)
				{
					m_AIWorld.FindPath(m_Unit.GetPosition(), m_Position, m_PathFilter, m_Path);
				}
*/
			}
		}
	}

	void SetPosition(vector pPosition)
	{
		m_Position = pPosition;
	}

	vector GetPosition()
	{
		return m_Position;
	}

	eAITargetOverriding GetOverride()
	{
		return m_Overriding;
	}

	void StopOverride()
	{
		//eAITrace trace(this, "StopOverride");
		
		m_Overriding = eAITargetOverriding.NONE;
	}

	void OverridePath()
	{
		//eAITrace trace(this, "OverridePath");

		m_Overriding = eAITargetOverriding.PATH;
		m_Path.Clear();
	}

	void OverridePath(array<vector> pPath)
	{
		//eAITrace trace(this, "OverridePath", pPath.ToString());

		m_Overriding = eAITargetOverriding.PATH;
		pPath.Copy(m_Path);
	}

	void OverridePosition(vector pPosition)
	{
		//eAITrace trace(this, "OverridePosition", pPosition.ToString());
		
		m_Overriding = eAITargetOverriding.POSITION;
		m_OverridePosition = pPosition;
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

		float epsilon = 0.5;
		for (int i = 0; i < m_Path.Count() - 1; i++)
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