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

	bool Generate(Object obj, int index, out array<ref Param3<eAIRoadNode, vector, bool>> connections)
	{
		int count = 0;

		for (int i = 0; i < 4; i++)
		{
			string lName = ROAD_MEMORY_POINT_PAIRS[i * 2];
			string rName = ROAD_MEMORY_POINT_PAIRS[i * 2 + 1];

			if (!obj.MemoryPointExists(lName) || !obj.MemoryPointExists(rName)) continue;

			vector lPos = obj.ModelToWorld(obj.GetMemoryPointPos(lName));
			vector rPos = obj.ModelToWorld(obj.GetMemoryPointPos(rName));
			vector pos = (lPos + rPos) * 0.5;

			m_Position = m_Position + pos;

			connections.Insert(new Param3<eAIRoadNode, vector, bool>(this, pos, false));
			count++;
		}

		//! The object doesn't have any valid memory points, it can't be a road
		if (count == 0) return false;

		//! The position is derived from the average of all the valid snapping memory points
		m_Position = m_Position * (1.0 / count);
		m_Index = index;

		return true;
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