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
		Resize(m_CenterPoint[0] * 2, m_CenterPoint[1] * 2);

		int x, z, i, j;

		array<ref Param3<eAIRoadNode, vector, bool>> connections();
		for (x = 0; x < m_Width; x++)
		{
			for (z = 0; z < m_Height; z++)
			{
				array<Object> objects();
				array<CargoBase> proxyCargos();
				GetGame().GetObjectsAtPosition(Vector(x, 0, z), 1.0, objects, proxyCargos);

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

		for (i = 0; i < connections.Count(); i++)
		{
			if (connections[i].param3) continue;

			for (j = 0; j < connections.Count(); j++)
			{
				if (connections[j].param3) continue;
				if (connections[i].param1 == connections[j].param1) continue;

				if (vector.Distance(connections[i].param2, connections[j].param2) < 0.5)
				{
					connections[i].param3 = true;
					connections[j].param3 = true;

					connections[i].param1.Add(connections[j].param1);
					connections[j].param1.Add(connections[i].param1);
				}
			}
		}
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
			float dist = vector.Distance(m_Roads[i].m_Position, position) ;
			if (dist < minDistance)
			{
				closest = m_Roads[i];
				minDistance = dist;
			}
		}

		return closest;
	}

	void FindPath(vector start, vector end, out array<vector> path)
	{
		eAIRoadNode closest = GetClosestNode(start);
		PathFilter filter = new PathFilter();
		AStar.Search(closest, end, filter, path);
	}
};