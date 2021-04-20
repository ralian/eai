class eAIDynamicPatrol {
	vector m_pos;
	autoptr array<vector> m_waypoints;
	float m_minR, m_maxR, m_despawnR;
	int m_count;
	string m_loadout;
	
	eAIGroup m_activeGroup = null;
	
	// @brief Create a dynamic patrol object which spawns a patrol under the right conditions.
	// @param pos the position that the trigger distance is calculated from
	// @param waypoints the waypoints the patrol follows. Only stores a soft pointer.
	// @param loadout the loadout each member is given
	// @param count the number of ai 
	// @param minR minimum radius a player can be away from the spawn point, if a player teleports or spawns right on top of it, they won't 
	// @param maxR distance at which an incoming player will spawn the patrol 
	// @param despawnR the distance away that a player must run to despawn the patrol
	void eAIDynamicPatrol(vector pos, array<vector> waypoints, string loadout = "SoldierLoadout.json", int count = 1, float minR = 300, float maxR = 800, float despawnR = 1000) {
		m_pos = pos;
		m_waypoints = waypoints;
		m_minR = minR;
		m_maxR = maxR;
		m_despawnR = despawnR;
		m_count = count;
		m_loadout = loadout;
	}
	
	eAIGroup SpawnPatrol() {
		Print("Spawning patrol at " + m_pos);
		eAIGame game = MissionServer.Cast(GetGame().GetMission()).GetEAIGame();
		eAIBase ai = game.SpawnAI_Patrol(m_pos, m_loadout);
		m_activeGroup = ai.GetGroup();
		foreach (vector v : m_waypoints) {m_activeGroup.AddWaypoint(v);}
		int count = m_count; // temporary so we don't decrease the actual amount
		while (count > 1) {
			(game.SpawnAI_Helper(ai, m_loadout)).RequestTransition("Rejoin");
			count--;
		}
		return m_activeGroup;
	}
	
	void UpdateTriggers() {
		//Print("Patrol at " + m_pos + " updating triggers");
		bool shouldSpawn = false, shouldDespawn = true;
		autoptr array<Man> players = {};
		GetGame().GetPlayers(players);
		vector patrolPos = m_pos;
		if (m_activeGroup && m_activeGroup.GetLeader())
			patrolPos = m_activeGroup.GetLeader().GetPosition();
		foreach (PlayerBase p : players) {
			float dist = vector.Distance(patrolPos, p.GetPosition());
			if (dist < m_despawnR) shouldDespawn = false;
			if (dist < m_maxR) shouldSpawn = true;
			if (dist < m_minR) shouldSpawn = false;
		}
		if (m_activeGroup) {
			bool groupIsObsolete = true;
			for (int i = 0; i < m_activeGroup.Count(); i++) {
				if (m_activeGroup.GetMember(i) && m_activeGroup.GetMember(i).IsAlive())
					groupIsObsolete = false;
			}
			
			if ((groupIsObsolete && shouldSpawn) || shouldDespawn) {
				m_activeGroup.DeleteMembers();
				delete m_activeGroup;
				m_activeGroup = null;
				Print("Deleted patrol at " + m_pos);
			}
		} else { // If the trigger is not active
			if (shouldSpawn) SpawnPatrol();
		}
		
		// Call this function 15 seconds in the future, plus a random interval to spread out computation of a bunch of patrols
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.UpdateTriggers, 15000 + Math.RandomInt(0, 1000), false);
	}
};