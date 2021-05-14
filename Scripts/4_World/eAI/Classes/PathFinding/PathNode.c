class PathNode
{
	vector m_Position;
	bool m_Valid;

	ref set<PathNode> m_Neighbours = new set<PathNode>();

	void Add(PathNode node)
	{
		m_Neighbours.Insert(node);
	}

	void Remove(PathNode node)
	{
		int idx = m_Neighbours.Find(node);
		if (idx != -1) m_Neighbours.Remove(idx);
	}
};