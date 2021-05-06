modded class ZombieBase
{
    private autoptr eAIZombieTargetInformation m_TargetInformation;

    void ZombieBase()
    {
		Class.CastTo(m_TargetInformation, CreateTargetInformation());
    }

    protected eAITargetInformation CreateTargetInformation()
    {
		//eAITrace trace(this, "CreateTargetInformation");
        return new eAIZombieTargetInformation(this);
    }

    eAIZombieTargetInformation GetTargetInformation()
    {
		//eAITrace trace(this, "GetTargetInformation");
        return m_TargetInformation;
    }

	override void EEKilled(Object killer)
	{
        m_TargetInformation.OnDeath();

        super.EEKilled(killer);
    }

	override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
        m_TargetInformation.OnHit();

		super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
    }

	override bool ModCommandHandlerBefore(float pDt, int pCurrentCommandID, bool pCurrentCommandFinished)
	{
        m_TargetInformation.m_Crawling = pCurrentCommandID == DayZInfectedConstants.COMMANDID_CRAWL;
        
		return super.ModCommandHandlerBefore(pDt, pCurrentCommandID, pCurrentCommandFinished);
	}
};