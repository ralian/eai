class eAIRoadNetwork
{
	const static int LATEST_VERSION = 4;

	private int m_Width;
	private int m_Height;
	private ref array<ref eAIRoadNode> m_Roads;

	private ref array<string> m_Directories;
	private string m_FilePath;

	private string m_WorldName;
	private vector m_CenterPoint;

	#ifndef SERVER
	private ref array<Shape> m_DebugShapes = new array<Shape>();
	#endif

	void eAIRoadNetwork()
	{
		m_Roads = new array<ref eAIRoadNode>();
		m_Directories = new array<string>();

		m_WorldName = GetGame().GetWorldName();
		m_CenterPoint = GetGame().ConfigGetVector("CfgWorlds " + m_WorldName + " centerPosition");

		m_Directories.Insert("$profile:eAI");
		m_Directories.Insert("eai/Scripts/Data/Roads");

		int mod_count = GetGame().ConfigGetChildrenCount("CfgMods");
		for (int i = 0; i < mod_count; i++)
		{
			string mod_name;
			GetGame().ConfigGetChildName("CfgMods", i, mod_name);
			
			if (GetGame().ConfigIsExisting("CfgMods " + mod_name + " roadNetworkDirectory"))
			{
				string directory;
				GetGame().ConfigGetText("CfgMods " + mod_name + " roadNetworkDirectory", directory);
				m_Directories.Insert(directory);
			}
		}

		Resize(0, 0);
	}

	void ~eAIRoadNetwork()
	{
		DS_Destroy();
	}

	void DS_Destroy()
	{
		#ifndef SERVER
		for (int i = 0; i < m_DebugShapes.Count(); i++)
		{
			m_DebugShapes[i].Destroy();
		}
		m_DebugShapes.Clear();
		#endif
	}

	void DS_Create(vector position, float radius)
	{
		DS_Destroy();

		#ifndef SERVER
		array<PathNode> visited();
		for (int i = 0; i < m_Roads.Count(); i++)
		{
			if (visited.Find(m_Roads[i]) != -1) continue;
			vector p1 = m_Roads[i].m_Position;
			p1[1] = 0;
			position[1] = 0;
			if (vector.Distance(p1, position) > radius) continue;
			visited.Insert(m_Roads[i]);

			m_DebugShapes.Insert(Shape.CreateSphere(0xFF0000FF, ShapeFlags.VISIBLE | ShapeFlags.WIREFRAME, m_Roads[i].m_Position, 0.5));
			
			for (int j = 0; j < m_Roads[i].m_Neighbours.Count(); j++)
			{
				if (visited.Find(m_Roads[i].m_Neighbours[j]) != -1) continue;

				vector points[2];
				points[0] = m_Roads[i].m_Position;
				points[1] = m_Roads[i].m_Neighbours[j].m_Position;
				m_DebugShapes.Insert(Shape.CreateLines(0xFFFF0000, ShapeFlags.VISIBLE | ShapeFlags.NOZBUFFER, points, 2));
			}
		}
		#endif
	}

	void Init()
	{
		foreach (string directory : m_Directories)
		{
			m_FilePath = directory + "/" + m_WorldName + ".roads";
			if (FileExist(m_FilePath))
			{
				if (Load(m_Directories[0] == directory))
				{
					return;
				}
			}
		}

		m_FilePath = m_Directories[0] + "/" + m_WorldName + ".roads";
		Generate();
		Save();
	}

	private void Resize(int width, int height)
	{
		m_Roads.Clear();

		m_Width = width;
		m_Height = height;
	}

	private void Generate()
	{
		Print("Generating");

		Resize(m_CenterPoint[0] * 2, m_CenterPoint[1] * 2);

		int x, z, i, j;

		Print("Finding Objects");
		array<ref Param3<eAIRoadNode, vector, bool>> connections();
		//for (x = 0; x < m_Width; x++)
		{
			//for (z = 0; z < m_Height; z++)
			{
				array<Object> objects();
				array<CargoBase> proxyCargos();

				float radius = m_Width;
				if (radius < m_Height) radius = m_Height;

				GetGame().GetObjectsAtPosition(m_CenterPoint, radius * 2.5, objects, proxyCargos);

				for (i = 0; i < objects.Count(); i++)
				{
					Object obj = objects[i];

					LOD geometry = obj.GetLODByName("geometry");
					if (!geometry || !eAIRoadNode.ObjectIsRoad(obj, geometry)) continue;

					eAIRoadNode road = new eAIRoadNode();
					if (road.Generate(obj, m_Roads.Count(), connections))
					{
						m_Roads.Insert(road);
					}
				}
			}
		}

		Print("Connecting Roads (Memory Points)");

		Param3<eAIRoadNode, vector, bool> a;
		Param3<eAIRoadNode, vector, bool> b;

		//! Connect roads that were placed properly
		for (i = 0; i < connections.Count(); i++)
		{
			a = connections[i];
			if (a.param3) continue;

			for (j = 0; j < connections.Count(); j++)
			{
				b = connections[j];

				if (b.param3) continue;
				if (a.param1 == b.param1) continue;

				vector aPos = a.param2;
				vector bPos = b.param2;

				aPos[1] = 0.0;
				bPos[1] = 0.0;

				if (vector.DistanceSq(aPos, bPos) < (2.0))
				{
					a.param3 = true;
					b.param3 = true;

					a.param1.Add(b.param1);
					b.param1.Add(a.param1);
				}
			}
		}

		Print("Connecting Roads (Nearby)");

		float nearByDist = 10.0;
		float nearByDistSq = nearByDist * nearByDist;

		//! Connect roads that weren't placed properly (ADAM!!!!!!!!!!!!)
		for (i = 0; i < m_Roads.Count(); i++)
		{
			for (j = 0; j < m_Roads.Count(); j++)
			{
				if (m_Roads[i] == m_Roads[j]) continue;

				if (vector.DistanceSq(m_Roads[i].m_Position, m_Roads[j].m_Position) < nearByDistSq)
				{
					m_Roads[i].Add(m_Roads[j]);
					m_Roads[j].Add(m_Roads[i]);
				}
			}
		}

		Print("Optimizing");

		for (i = m_Roads.Count() - 1; i >= 0; i--)
		{
			if (!m_Roads[i] || (m_Roads[i] && m_Roads[i].Optimize()))
			{
				m_Roads.RemoveOrdered(i);
			}
		}

		Print("Fixing indices");

		for (i = 0; i < m_Roads.Count(); i++)
		{
			m_Roads[i].m_Index = i;
		}

		Print("Finished Generating");
	}

	private void Save()
	{
		MakeDirectory(m_Directories[0]);

		FileHandle file_handle = OpenFile(m_FilePath, FileMode.WRITE);

		int version = LATEST_VERSION;

		FPrintln(file_handle, LATEST_VERSION);
		FPrintln(file_handle, m_Width);
		FPrintln(file_handle, m_Height);
		FPrintln(file_handle, m_Roads.Count());
		
		for (int i = 0; i < m_Roads.Count(); i++)
		{
			m_Roads[i].Save(file_handle, LATEST_VERSION);
		}

		CloseFile(file_handle);
	}

	private bool OnLoad(FileHandle file_handle, out int version)
	{
		m_Roads.Clear();

		string line_content;
		int x;
		int z;
		int i;
		int count;

		FGets(file_handle, line_content);
		version = line_content.ToInt();

		if (version < 4) return false;

		FGets(file_handle, line_content);
		m_Width = line_content.ToInt();

		FGets(file_handle, line_content);
		m_Height = line_content.ToInt();

		if (m_Width != m_CenterPoint[0] * 2) return false;
		if (m_Height != m_CenterPoint[1] * 2) return false;

		Resize(m_Width, m_Height);

		FGets(file_handle, line_content);
		count = line_content.ToInt();
		
		array<ref eAIRoadNodeJoinMap> connections();

		for (i = 0; i < count; i++)
		{
			eAIRoadNode road = new eAIRoadNode();
			road.Load(file_handle, version, connections);
			m_Roads.Insert(road);
		}

		for (i = 0; i < connections.Count(); i++)
		{
			foreach (int index : connections[i].indices)
			{
				connections[i].node.Add(m_Roads[index]);
			}
		}

		return true;
	}

	private bool Load(bool saveOnFail = true)
	{
		FileHandle file_handle = OpenFile(m_FilePath, FileMode.READ);
		int version;

		if (!OnLoad(file_handle, version))
		{
			CloseFile(file_handle);
			return false;
		}

		CloseFile(file_handle);

		if (version != LATEST_VERSION && saveOnFail)
		{
			Save();
		}

		return true;
	}

	eAIRoadNode GetClosestNode(vector position)
	{
		float minDistance = m_CenterPoint.Length() * 4.0;
		eAIRoadNode closest = null;
		for (int i = 0; i < m_Roads.Count(); i++)
		{
			float dist = vector.Distance(m_Roads[i].m_Position, position);
			if (dist < minDistance)
			{
				closest = m_Roads[i];
				minDistance = dist;
			}
		}

		return closest;
	}

	void FindPath(vector start, vector end, inout array<vector> path)
	{
		eAIRoadNode start_node = GetClosestNode(start);
		eAIRoadNode end_node = GetClosestNode(end);
		
		if (!start_node) return;
		if (!end_node) return;
				
		AStar.Perform(start_node, end_node, path);
	}
};