class eAIFactionEast : eAIFaction
{
	void eAIFactionEast()
	{
		m_Name = "Raiders";
	}

	override bool IsFriendly(eAIFaction other)
	{
		if (other.IsInherited(eAIFactionRaiders)) return true;
		if (other.IsInherited(eAIFactionCivilian)) return true;
		return false;
	}
};