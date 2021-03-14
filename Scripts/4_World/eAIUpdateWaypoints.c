//! This function serves as a nonmember wrapper for the BI AIWorld::FindPath function so it can be multithreaded.
//! It may need some custom pathing functionality as well.

int atomic_findPathRunning = 0;

void updateWaypoints(eAIPlayerHandler p, vector whereTo, bool addzerowaypoint = false, bool debug_waypoints = false) {
	
	bool success = false;
	
	vector pos = p.unit.GetPosition();
	
	while (!success) {
		if (++atomic_findPathRunning < 2) { // if we are the only one about to call FindPath
			p.clearWaypoints();
			if (addzerowaypoint)
				p.waypoints.Insert(pos); // Make WP 0 the player's position. This is only necessary for the corner smoothing logic to work on the first waypoint.
			GetGame().GetWorld().GetAIWorld().FindPath(pos, whereTo, p.pgFilter, p.waypoints);
			atomic_findPathRunning--;
			success = true;
		} else {							// else, decrement the counter then yield before trying again
			atomic_findPathRunning--;
			Idle();
		}
	}
		
	if (!p.nextWaypoint())
		Error("eAI controlled unit " + p.ToString() + " called FindPath(), but no waypoints were generated!");
	
	// Apply corner smoothing logic
	// Basically, each "middle" waypoint is pushed outwards 1 meter along the line bisecting the previous and next waypoints.
	// This is so that the unit does not walk straight into things like the corner of a fence, rather it tries to walk around.
	vector midpoint = vector.Zero;
	vector delta = vector.Zero;
	int finalWPNumber = p.waypoints.Count() - 1;
	for (int i = 1; i < finalWPNumber; i++) {
		int previousWP = i - 1;
		int nextWP = i + 1;
		for (int j = 0; j < 3; j++) { // iterator over indices
			midpoint[j] = p.waypoints[previousWP][j];
			midpoint[j] = p.waypoints[nextWP][j] + midpoint[j];
			midpoint[j] = 0.5 + midpoint[j];
		}
		delta = vector.Direction(midpoint, p.waypoints.Get(i)); // imma be real with u chief, I don't know why the Get operator works here but the array accessor does not.
															    // the doc says they should do the same thing... I think it's an enforce parsing bug
		
		// on second thought, this condition harms trying to get around fences in some cases
		//if (delta.LengthSq() > 1)
			delta.Normalize();
		
		for (int m = 0; m < 3; m++) // iterator over indices
			delta[m] = 2*delta[m];
		
		p.waypoints[i] = p.waypoints[i] - delta;
	}
	
	// Debug info
	if (debug_waypoints) {
		Print("Current Pos: " + pos);
		Print("Current Waypoint: " + p.cur_waypoint_no);
		Print("Waypoint List:");
		for (int k = 0; k < p.waypoints.Count(); k++) {
			Print(p.waypoints[k][0].ToString() + "," + p.waypoints[k][2].ToString()); // Make sure to do this in an excel friendly way
			GetRPCManager().SendRPC("eAI", "DebugParticle", new Param2<vector, vector>(p.waypoints[k], vector.Zero));
		}
	}
}