class eAIGroup
{
	PlayerBase m_Leader;

	void SetLeader(PlayerBase leader)
	{
		m_Leader = leader;
	}

	PlayerBase GetLeader()
	{
		return m_Leader;
	}
};