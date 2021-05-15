class eAIRoadSection extends PathNode
{
	ref set<eAIRoadNode> m_Nodes = new set<eAIRoadNode>();

	ref array<vector> m_Points = new array<vector>();
	ref array<int> m_End = new array<int>();

	eAIRoadNode m_Head;
	eAIRoadNode m_Tail;

	float m_Radius;

	void Init()
	{
		m_Radius = 0;

		m_Head = null;
		m_Tail = null;

		m_Position = "0 0 0";
		int count = m_Nodes.Count();
		for (int i = 0; i < count; i++)
		{
			//m_Position = m_Position + (m_Nodes[i].m_Position * (1.0 / count));

			if (m_Nodes[i].Count() != 2)
			{
				if (m_Head) m_Tail = m_Nodes[i];
				else m_Head = m_Nodes[i];
			}
			
			for (int j = 0; j < count; j++)
			{
				float dist = vector.Distance(m_Nodes[i].m_Position, m_Nodes[j].m_Position) * 0.5;
				if (dist > m_Radius)
				{
					m_Position = (m_Nodes[i].m_Position + m_Nodes[j].m_Position) * 0.5;
					m_Radius = dist;
				}
			}
		}
		
		m_Radius *= 1.1;

		if (!m_Tail) m_Tail = m_Head;

		m_Nodes.Clear();

		eAIRoadNode current = m_Head;
		while (current != null)
		{
			m_Nodes.Insert(current);
			m_Points.Insert(current.m_Position);

			if (current.Count() != 2) break;

			int idx = m_Nodes.Find(eAIRoadNode.Cast(current[0]));
			if (idx == -1) Class.CastTo(current, current[0]);
			else Class.CastTo(current, current[1]);
			break;
		}

		for (i = 0; i < m_Neighbours.Count(); i++)
		{
			eAIRoadSection section;
			Class.CastTo(section, m_Neighbours[i]);

			int type = 0;
			if (section.ContainsPath(m_Tail)) type = 1;
			m_End.Insert(type);
		}
	}

	bool ConnectedViaHead(int idx)
	{
		return m_End[idx] == 0;
	}

	bool ConnectedViaHead(PathNode section)
	{
		return m_End[m_Neighbours.Find(section)] == 0;
	}

	bool ContainsPath(eAIRoadNode node)
	{
		return m_Nodes.Find(node) != -1;
	}
};