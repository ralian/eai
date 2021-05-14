class PathNode
{
	vector m_Position;
	bool m_Valid;

	ref array<ref PathNode> m_Neighbours = new array<ref PathNode>();

	void Add(PathNode node)
	{
		if (node == this) return;
		
		int idx = m_Neighbours.Find(node);
		if (idx == -1) m_Neighbours.Insert(node);
	}

	void Remove(PathNode node)
	{
		int idx = m_Neighbours.Find(node);
		while (idx != -1)
		{
			m_Neighbours.Remove(idx);
			idx = m_Neighbours.Find(node);
		}
	}
};