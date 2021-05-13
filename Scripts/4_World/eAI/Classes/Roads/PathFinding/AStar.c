class AStar
{
	static float Heuristic(vector a, vector b)
	{
		return Math.AbsFloat(a[0] - b[0]) + Math.AbsFloat(a[1] - b[1]) + Math.AbsFloat(a[2] - b[2]);
	}

	static void Search(PathNode start, vector goal, PathFilter filter, out array<vector> path)
	{
		PriorityQueue<PathNode> queue = new PriorityQueue<PathNode>();
		queue.Enqueue(start, 0);

		float distSq = filter.m_Distance * filter.m_Distance;

		map<PathNode, PathNode> mappedPath();
		map<PathNode, float> cost();

		PathNode current;

		mappedPath[start] = start;
		cost[start] = 0.0;

		while (queue.Count() > 0)
		{
			current = queue.Dequeue();

			if (vector.Distance(current.m_Position, goal) < distSq)
			{
				break;
			}

			float newCost = cost[current] + 1.0;
			foreach (PathNode next : current.m_Neighbours)
			{
				if (next.m_Valid && (!cost.Contains(next) || newCost < cost[next]))
				{
					cost[next] = newCost;
					float priority = newCost + Heuristic(next.m_Position, goal);
					queue.Enqueue(next, priority);
					mappedPath[next] = current;
				}
			}
		}

		path.Insert(current.m_Position);
		while (true)
		{
			path.Insert(current.m_Position);
			current = mappedPath[current];
		}
	}
}