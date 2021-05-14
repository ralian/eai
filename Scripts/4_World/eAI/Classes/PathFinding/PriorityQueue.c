class PriorityQueue<Class T>
{
	private ref array<ref Param2<T, float>> elements = new array<ref Param2<T, float>>();

	int Count()
	{
		return elements.Count();
	}

	void Enqueue(T item, float priority)
	{
		elements.Insert(new Param2<T, float>(item, priority));
	}

	T Dequeue()
	{
		int bestIndex = 0;

		for (int i = 0; i < elements.Count(); i++)
		{
			if (elements[i].param2 < elements[bestIndex].param2)
			{
				bestIndex = i;
			}
		}

		T bestItem = elements[bestIndex].param1;
		elements.Remove(bestIndex);
		return bestItem;
	}
}