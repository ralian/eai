class eAIZombieTargetInformation extends eAIEntityTargetInformation
{
	private const float DISTANCE_COEF = 0.02;

    private ZombieBase m_Zombie;
	private DayZInfectedInputController m_DIIP;

    void eAIZombieTargetInformation(EntityAI target)
    {
        Class.CastTo(m_Zombie, target);

		m_DIIP = m_Zombie.GetInputController();
    }

	// https://www.desmos.com/calculator/r4mqu91qff
    override float GetThreat(eAIBase ai = null)
    {
		float levelFactor = 0;

		//TODO: check to see if ::GetMindState() returns int of 0->4
		int level = m_DIIP.GetMindState();
		switch (level)
		{
			case DayZInfectedConstants.MINDSTATE_CALM:
				levelFactor = 0.00;
				break;
			case DayZInfectedConstants.MINDSTATE_DISTURBED:
				levelFactor = 0.25;
				break;
			case DayZInfectedConstants.MINDSTATE_ALERTED:
				levelFactor = 0.50;
				break;
			case DayZInfectedConstants.MINDSTATE_CHASE:
				levelFactor = 0.75;
				break;
			case DayZInfectedConstants.MINDSTATE_FIGHT:
				levelFactor = 1.00;
				break;
		}

		levelFactor *= 0.75;
		if (m_DIIP.GetTargetEntity() == ai) levelFactor *= 2.0;

		if (ai)
		{
			// the further away the zombie, the less likely it will be a threat
			float distance = GetDistance(ai) * DISTANCE_COEF;
			if (distance > 1.0) levelFactor = levelFactor / distance;
		}

        return Math.Clamp(levelFactor, 0.0, 1.0);
    }
};