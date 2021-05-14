class PathNode
{
	vector m_Position;
	bool m_Valid;

	ref set<ref PathNode> m_Neighbours = new set<ref PathNode>();

	void Add(PathNode node)
	{
		m_Neighbours.Insert(node);
	}
};