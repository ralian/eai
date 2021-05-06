class eAIActionWeaponControl : ActionBase
{
	void eAIActionWeaponControl()
	{
	}

	override typename GetInputType()
	{
		return DefaultActionInput;
	}

	override bool HasTarget()
	{
		return false;
	}

	override void CreateConditionComponents()
	{
		m_ConditionItem = new CCINonRuined();
		m_ConditionTarget = new CCTSelf;
	}

	override bool HasProgress()
	{
		return false;
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		return false;
	}
};