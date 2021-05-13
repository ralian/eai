class PathNode
{
	vector m_Position;
	bool m_Valid;

	ref array<ref PathNode> m_Neighbours = new array<ref PathNode>();

	void Add(PathNode node)
	{
		m_Neighbours.Insert(node);
	}
};