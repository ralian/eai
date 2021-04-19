modded class DayZPlayerImplement
{
    private autoptr eAITargetInformation m_TargetInformation;

    protected autoptr eAIGroup m_eAI_Group;

    void DayZPlayerImplement()
    {
        m_TargetInformation = new eAIEntityTargetInformation(this);
    }

    eAITargetInformation GetTargetInformation()
    {
        return m_TargetInformation;
    }

    bool IsAI()
    {
        return false;
    }

	void SetGroup(eAIGroup group)
    {
		m_eAI_Group = group;
	}

	eAIGroup GetGroup()
	{
		return m_eAI_Group;
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