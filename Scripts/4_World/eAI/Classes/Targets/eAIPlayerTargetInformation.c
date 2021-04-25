class eAIPlayerTargetInformation extends eAIEntityTargetInformation
{
	private const float DISTANCE_COEF = 0.001;

    private DayZPlayerImplement m_Player;

    void eAIPlayerTargetInformation(EntityAI target)
    {
        Class.CastTo(m_Player, target);
    }

    override float GetThreat(eAIBase ai = null)
    {
		float levelFactor = 0.5;

		if (ai)
		{
			// the further away the zombie, the less likely it will be a threat
			float distance = GetDistance(ai) * DISTANCE_COEF;
			if (distance > 1.0) levelFactor = levelFactor / distance;
		}

        return Math.Clamp(levelFactor, 0.0, 1.0);
    }
};