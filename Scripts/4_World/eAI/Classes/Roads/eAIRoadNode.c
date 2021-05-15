class eAIRoadNode extends PathNode
{
	static ref array<string> ROAD_MEMORY_POINT_PAIRS =
	{
		"LB", "PB",
		"LE", "PE",
		"LD", "LH",
		"PD", "PH",
	};

	int m_Index;
	
	ref set<eAIRoadSection> m_Sections = new set<eAIRoadSection>();

	void InsertSection(eAIRoadSection section)
	{
		if (section == null) return;

		for (int i = 0; i < m_Sections.Count(); i++)
		{
			section.Add(m_Sections[i]);
			m_Sections[i].Add(section);
		}

		m_Sections.Insert(section);
		section.m_Nodes.Insert(this);
	}

	void RemoveSection(eAIRoadSection section)
	{
		int idx = m_Sections.Find(section);
		if (idx != -1) m_Sections.Remove(idx);
	}

	int CountSection()
	{
		return m_Sections.Count();
	}

	eAIRoadSection GetSection(int index)
	{
		return m_Sections[index];
	}

	bool ContainsSection(eAIRoadSection section)
	{
		return m_Sections.Find(section) != -1;
	}

	void ClearSections()
	{
		m_Sections.Clear();
	}

	bool RoadAlreadyConnectedWithin(eAIRoadNode node, vector position, float radius, eAIRoadNode parent = null)
	{
		if (vector.Distance(Vector(m_Position[0], 0, m_Position[2]), position) > radius) return false;

		if (m_Neighbours.Find(node) != -1) return true;

		for (int i = 0; i < m_Neighbours.Count(); i++)
		{
			if (m_Neighbours[i] == parent) continue;

			eAIRoadNode neighbourNode;
			if (Class.CastTo(neighbourNode, m_Neighbours[i]) && neighbourNode.RoadAlreadyConnectedWithin(node, position, radius, this)) return true;
		}

		return false;
	}

	bool Generate(Object obj, inout array<ref eAIRoadConnection> connections)
	{
		int count = 0;
		eAIRoadConnection connection;
		
		int colours[] = {0x1F1F001F, 0x1FF1001F, 0x1F1F00F1, 0x1FF100F1};
		
		bool flipI = true;
		if (vector.Dot(obj.GetDirection(), "0 0 1") > 0) flipI = false;
		if (vector.Dot(obj.GetDirection(), "1 0 0") > 0) flipI = false;

		for (int i = 0; i < 4; i++)
		{
			string lName = ROAD_MEMORY_POINT_PAIRS[i * 2];
			string rName = ROAD_MEMORY_POINT_PAIRS[i * 2 + 1];

			if (!obj.MemoryPointExists(lName) || !obj.MemoryPointExists(rName)) continue;

			vector lPos = obj.ModelToWorld(obj.GetMemoryPointPos(lName));
			vector rPos = obj.ModelToWorld(obj.GetMemoryPointPos(rName));
			vector pos = (lPos + rPos) * 0.5;

			if (i < 2) m_Position = m_Position + pos;
			
			connection = new eAIRoadConnection();
			connection.m_Node = this;
			connection.m_Position = pos;
			connections.Insert(connection);
			
			int dbgI = i;
			if (flipI)
			switch (dbgI)
			{
				case 0:
					dbgI = 1;
					break;
				case 1:
					dbgI = 0;
					break;
				case 2:
					dbgI = 3;
					break;
				case 3:
					dbgI = 2;
					break;
			}
			
			eAIRoadNetwork.DS_Add(Shape.CreateSphere(colours[dbgI], ShapeFlags.VISIBLE | ShapeFlags.TRANSP | ShapeFlags.WIREFRAME | ShapeFlags.NOZBUFFER, connection.m_Position + Vector(0, dbgI, 0), 0.25));

			count++;
		}

		//! The object doesn't have any valid memory points, it can't be a road
		if (count == 0)
		{
			//return false;
			m_Position = obj.GetPosition();

			vector min_max[2];
			float radius = obj.ClippingInfo(min_max);

			connection = new eAIRoadConnection();
			connection.m_Node = this;
			connection.m_Position = obj.ModelToWorld(Vector(0, 0, min_max[0][2]));
			connections.Insert(connection);
			eAIRoadNetwork.DS_Add(Shape.CreateSphere(0x1FFF00FF, ShapeFlags.VISIBLE | ShapeFlags.TRANSP | ShapeFlags.WIREFRAME | ShapeFlags.NOZBUFFER, connection.m_Position, 0.25));

			connection = new eAIRoadConnection();
			connection.m_Node = this;
			connection.m_Position = obj.ModelToWorld(Vector(0, 0, min_max[1][2]));
			connections.Insert(connection);
			eAIRoadNetwork.DS_Add(Shape.CreateSphere(0x1FFF00FF, ShapeFlags.VISIBLE | ShapeFlags.TRANSP | ShapeFlags.WIREFRAME | ShapeFlags.NOZBUFFER, connection.m_Position, 0.25));

			
			connection = new eAIRoadConnection();
			connection.m_Node = this;
			connection.m_Position = obj.ModelToWorld(Vector(min_max[0][0], 0, 0));
			connections.Insert(connection);
			eAIRoadNetwork.DS_Add(Shape.CreateSphere(0x1FFF00FF, ShapeFlags.VISIBLE | ShapeFlags.TRANSP | ShapeFlags.WIREFRAME | ShapeFlags.NOZBUFFER, connection.m_Position, 0.25));

			connection = new eAIRoadConnection();
			connection.m_Node = this;
			connection.m_Position = obj.ModelToWorld(Vector(min_max[1][0], 0, 0));
			connections.Insert(connection);
			eAIRoadNetwork.DS_Add(Shape.CreateSphere(0x1FFF00FF, ShapeFlags.VISIBLE | ShapeFlags.TRANSP | ShapeFlags.WIREFRAME | ShapeFlags.NOZBUFFER, connection.m_Position, 0.25));
			
			
			return true;
		}

		//! The position is derived from the average of all the valid snapping memory points
		if (count >= 2) count = 2;
		m_Position = m_Position * (1.0 / count);

		return true;
	}

	bool Optimize()
	{
		//! If we don't have 2 neighbours, then this node may be important
		if (m_Neighbours.Count() != 2)
		{			
			return false;
		}

		PathNode a = m_Neighbours[0];
		PathNode b = m_Neighbours[1];

		if (a.m_Neighbours.Count() == 2 && b.m_Neighbours.Count() == 2)
		{
			vector dirA = vector.Direction(a.m_Position, m_Position);
			vector dirB = vector.Direction(m_Position, b.m_Position);
	
			dirA[1] = 0;
			dirB[1] = 0;
	
			dirA.Normalize();
			dirB.Normalize();
	
			float minDist = 50.0;
	
			float distSq = vector.DistanceSq(a.m_Position, b.m_Position);
	
			//! If the angle of change is too small to notice and the distance is close enough, remove this node
			if ((vector.Dot(dirA, dirB) > 0.998 && distSq < (minDist * minDist)))
			{
				a.Remove(this);
				b.Remove(this);
	
				a.Add(b);
	
				m_Neighbours.Clear();
				return true;
			}
		}
		
		return false;
		
		distSq = vector.DistanceSq(m_Position, a.m_Position);
		if (distSq < 10.0)
		{
			a.Remove(this);
			b.Remove(this);

			a.Add(b);

			m_Neighbours.Clear();
			return true;
		}
		
		distSq = vector.DistanceSq(m_Position, b.m_Position);
		if (distSq < 10.0)
		{
			a.Remove(this);
			b.Remove(this);

			a.Add(b);

			m_Neighbours.Clear();
			return true;
		}

		return false;
	}

	void Save(FileHandle file_handle, int version)
	{
		FPrintln(file_handle, m_Index);
		FPrintln(file_handle, m_Position);
		FPrintln(file_handle, (int)m_Valid);
		FPrintln(file_handle, m_Neighbours.Count());
		for (int i = 0; i < m_Neighbours.Count(); i++)
		{
			FPrintln(file_handle, eAIRoadNode.Cast(m_Neighbours[i]).m_Index);
		}
	}

	void Load(FileHandle file_handle, int version, out array<ref eAIRoadNodeJoinMap> connections)
	{
		string line_content;
		int count;

		FGets(file_handle, line_content);
		m_Index = line_content.ToInt();

		FGets(file_handle, line_content);
		m_Position = line_content.ToVector();

		FGets(file_handle, line_content);
		m_Valid = line_content.ToInt();

		FGets(file_handle, line_content);
		count = line_content.ToInt();

		eAIRoadNodeJoinMap loadCon = new eAIRoadNodeJoinMap();
		loadCon.node = this;
		for (int i = 0; i < count; i++)
		{
			FGets(file_handle, line_content);
			loadCon.Add(line_content.ToInt());
		}

		connections.Insert(loadCon);
	}

	static bool ObjectIsRoad(Object obj, LOD geometry)
	{
		for ( int i = 0; i < geometry.GetPropertyCount(); ++i )
		{
			string name = geometry.GetPropertyName(i);
			string value = geometry.GetPropertyValue(i);
			name.ToLower();
			value.ToLower();
			if (name == "class")
			{
				return value == "road";
			}
		}

		return false;
	}
};