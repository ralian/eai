class eAIEntityTargetInformation extends eAITargetInformation
{
	private EntityAI m_Target;

	void eAIEntityTargetInformation(EntityAI target)
	{
		m_Target = target;
	}

	override string DebugString()
	{
		return Object.GetDebugName(m_Target);
	}

	override EntityAI GetEntity()
	{
		return m_Target;
	}

	override bool IsActive()
	{
		if (!m_Target)
			return false;

		return m_Target.GetHealth() > 0.0;
	}

	override vector GetPosition(eAIBase ai = null)
	{
		return m_Target.GetPosition();
	}

	override float GetDistance(eAIBase ai)
	{
		return vector.Distance(ai.GetPosition(), m_Target.GetPosition());
	}
};

class eAIEntity<Class T>
{
	static eAITargetInformation GetTargetInformation(T entity)
	{
		return entity.GetTargetInformation();
	}
};
