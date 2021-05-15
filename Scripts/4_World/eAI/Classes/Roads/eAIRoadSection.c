class eAIRoadSection extends PathNode
{
	ref set<eAIRoadNode> m_Nodes = new set<eAIRoadNode>();

	eAIRoadNode m_Head;
	eAIRoadNode m_Tail;

	float m_Radius;

	void Init()
	{
		float minLen = 10000000.0;
		float maxLen = 0.0;

		m_Head = null;
		m_Tail = null;

		m_Position = "0 0 0";
		int count = m_Nodes.Count();
		for (int i = 0; i < count; i++)
		{
			m_Position = m_Position + (m_Nodes[i].m_Position * (1.0 / count));

			if (m_Nodes[i].Count() != 2)
			{
				if (m_Head) m_Tail = m_Nodes[i];
				else m_Head = m_Nodes[i];
			}

			float len = m_Nodes[i].m_Position.Length();

			if (len < minLen) minLen = len;
			if (len > maxLen) maxLen = len;
		}

		m_Radius = maxLen - minLen;
	}
};