//! This function serves as a nonmember wrapper for the BI AIWorld::FindPath function so it can be multithreaded.
//! It may need some custom pathing functionality as well.

int atomic_findPathRunning = 0;

void updateWaypoints(PlayerBase p) {
	
	bool success = false;
	
	while (!success) {
		if (++atomic_findPathRunning < 2) { // if we are the only one about to call FindPath
			p.clearWaypoints();
			GetGame().GetWorld().GetAIWorld().FindPath(p.GetPosition(), p.m_FollowOrders.GetPosition(), p.pgFilter, p.waypoints);
			atomic_findPathRunning--;
			success = true;
		} else {							// else, decrement the counter then yield before trying again
			atomic_findPathRunning--;
			Idle();
		}
	}
		
	if (!p.nextWaypoint())
		Error("eAI controlled unit " + p.ToString() + " called FindPath(), but no waypoints were generated!");
	
	// Debug info
	Print("Current Pos: " + p.GetPosition());
	Print("Current Waypoint: " + p.cur_waypoint_no);
	for (int i = 0; i < p.waypoints.Count(); i++)
		Print(p.waypoints[i][0].ToString() + "," + p.waypoints[i][2].ToString()); // Make sure to do this in an excel friendly way
}