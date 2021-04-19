class eAIZombieTargetInformation extends eAITargetInformation
{
	private const float DISTANCE_COEF = 0.01;

    private ZombieBase m_Zombie;
	private DayZInfectedInputController m_DIIP;

    void eAIZombieTargetInformation(EntityAI target)
    {
        Class.CastTo(m_Zombie, target);

		m_DIIP = m_Zombie.GetInputController();
    }

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

		if (ai)
		{
			float distance = GetDistance(ai) * DISTANCE_COEF;
			if (distance > 1.0) levelFactor = levelFactor / distance;
		}

        return levelFactor;
    }
};