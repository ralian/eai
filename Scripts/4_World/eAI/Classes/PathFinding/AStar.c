class AStar
{
	static float Heuristic(vector a, vector b)
	{
		return Math.AbsFloat(a[0] - b[0]) + Math.AbsFloat(a[1] - b[1]) + Math.AbsFloat(a[2] - b[2]);
	}

	static void Perform(PathNode start, PathNode goal, inout array<vector> path)
	{
		PriorityQueue<PathNode> queue = new PriorityQueue<PathNode>();
		queue.Enqueue(start, 0);

		map<ref PathNode, ref PathNode> mappedPath();
		map<ref PathNode, float> cost();

		PathNode current = null;

		mappedPath[start] = null;
		cost[start] = 0.0;

		while (queue.Count() > 0)
		{
			current = queue.Dequeue();

			if (current == goal) break;

			foreach (PathNode next : current.m_Neighbours)
			{
				float newCost = cost[current] + vector.DistanceSq(goal.m_Position, next.m_Position);
				if ((!cost.Contains(next) || newCost < cost[next]))
				{
					cost[next] = newCost;
					float priority = newCost + Heuristic(next.m_Position, goal.m_Position);
					queue.Enqueue(next, priority);
					mappedPath[next] = current;
				}
			}
		}

		//! By design, to ignore the goal when adding to the list
		while (mappedPath.Contains(current))
		{
			current = mappedPath[current];
			if (!current) return;
			
			path.Insert(current.m_Position);
		}
	}
}