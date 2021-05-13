class eAIRoadNodeJoinMap
{
	eAIRoadNode node;
	ref array<int> indices = new array<int>();

	void Add(int index)
	{
		indices.Insert(index);
	}
};