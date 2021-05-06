modded class AnimalBase
{
    private autoptr eAITargetInformation m_TargetInformation;

    void AnimalBase()
    {
        m_TargetInformation = CreateTargetInformation();
    }

    protected eAITargetInformation CreateTargetInformation()
    {
		//eAITrace trace(this, "CreateTargetInformation");
        return new eAIEntityTargetInformation(this);
    }

    eAITargetInformation GetTargetInformation()
    {
		//eAITrace trace(this, "UpdateTargets");
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
};