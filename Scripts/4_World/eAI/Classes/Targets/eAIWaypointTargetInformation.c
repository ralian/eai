class eAIWaypointTargetInformation extends eAITargetInformation
{
    private autoptr array<vector> waypoints = {};
	private int waypoint = -1;
	
	private bool looping = true;
	
	private eAIGroup m_Target; // here the target is actually the group we are in
	
	private float m_RadiusSq = 25.0;


    void eAIWaypointTargetInformation(eAIGroup target)
    {
        m_Target = target;
    }

    override EntityAI GetEntity()
    {
        return m_Target.GetLeader();
    }

    override vector GetPosition(eAIBase ai = null)
    {
        if (waypoints.Count() > 0 && waypoint > -1)
			return waypoints[waypoint];

        return GetEntity().GetPosition();
    }
	
	int AddWaypoint(vector pos) {
		waypoints.Insert(pos);
		if (waypoint < 0) waypoint++;
		return waypoint;
	}
	
	void ClearWaypoints() {
		waypoints.Clear();
		waypoint = -1;
	}
	
	int SkipWaypoint() {
		if (waypoint > -1) {
			if (waypoints.Count() > (++waypoint))
				return waypoint;
			if (looping) waypoint = 0;
			else waypoint = -1;
			return waypoint;
		}
		return -1;
	}
	
	void SetLooping(bool loop) {
		looping = loop;
	}

    void Update(float pDt)
    {
		if (!GetEntity()) return;
		
		vector newPos = GetEntity().GetPosition();

		if (vector.DistanceSq(newPos, GetPosition()) < m_RadiusSq)
			Print(SkipWaypoint());
		
    }
};