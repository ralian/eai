class eAITarget
{
	eAIGroup group;
	int found_at_time;
	int max_time;
	autoptr set<eAIBase> ai_list;
	eAITargetInformation info;

	void eAITarget(eAIGroup _group, int _found_at_time, int _max_time, eAITargetInformation _info)
	{
		group = _group;
		found_at_time = _found_at_time;
		if (_max_time != int.MIN)
			max_time = _max_time;
		else
			max_time = 10000;

		ai_list = new set<eAIBase>();
		info = _info;
	}

	bool AddAI(eAIBase ai)
	{
		if (ai_list.Find(ai) != -1)
			return false;

		ai_list.Insert(ai);
		return true;
	}

	bool RemoveAI(eAIBase ai)
	{
		int idx = ai_list.Find(ai);
		if (idx == -1)
			return false;

		ai_list.Remove(idx);

		return true;
	}

	int FindAI(eAIBase ai)
	{
		return ai_list.Find(ai);
	}

	int CountAI()
	{
		return ai_list.Count();
	}

	bool HasInfo()
	{
		return info != null;
	}

	EntityAI GetEntity()
	{
		return info.GetEntity();
	}

	vector GetPosition(eAIBase ai = null)
	{
		return info.GetPosition(ai);
	}

	vector GetAimOffset(eAIBase ai = null)
	{
		return info.GetAimOffset(ai);
	}

	float GetThreat(eAIBase ai = null)
	{
		return info.GetThreat(ai);
	}

	float ShouldRemove(eAIBase ai = null)
	{
		return info.ShouldRemove(ai);
	}

	float GetDistance(eAIBase ai)
	{
		return info.GetDistance(ai);
	}
};
