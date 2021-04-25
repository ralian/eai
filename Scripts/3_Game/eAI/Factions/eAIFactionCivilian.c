class eAIFactionCivilian : eAIFaction
{
	void eAIFactionCivilian()
	{
		m_Name = "Civilian";
	}

	override bool IsFriendly(eAIFaction other)
	{
		return true;
	}
};