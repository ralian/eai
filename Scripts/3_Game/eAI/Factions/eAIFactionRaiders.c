class eAIFactionRaiders : eAIFaction
{
	void eAIFactionRaiders()
	{
		m_Name = "Raiders";
	}

	override bool IsFriendly(eAIFaction other)
	{
		return false;
	}
};