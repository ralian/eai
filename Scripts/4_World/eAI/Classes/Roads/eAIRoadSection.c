class eAIRoadSection extends PathNode
{
	ref set<eAIRoadNode> m_Nodes = new set<eAIRoadNode>();
	float m_Radius;

	void Init()
	{
		m_Position = "0 0 0";
		int count = m_Nodes.Count();
		for (int i = 0; i < count; i++)
		{
			m_Position = m_Position + (m_Nodes[i].m_Position * (1.0 / count));
		}
	}
};