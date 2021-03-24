class eAIGroup
{
	// Ordered array of group members. 0 is the leader.
	autoptr array<PlayerBase> m_Members = {};
	
	// The LeaderPosOld is used to calculate the heading of the formation
	// Recalculation of azumith is done only when m_DirRecalcDistSq is surpassed
	vector m_LeaderPosOld = "0 0 0";
	float m_DirRecalcDistSq = 25.0;
	vector m_LeaderDir = "0 0 0";
	vector m_LeaderDirPerp = "0 0 0";

	void SetLeader(PlayerBase leader)
	{
		if (m_Members[0])
			m_Members[0] = leader;
		else
			AddMember(leader);
	}

	PlayerBase GetLeader()
	{
		return m_Members[0];
	}
	
	int AddMember(PlayerBase member) {
		return m_Members.Insert(member);
	}
	
	void UpdateFormDir() {
		if (!m_Members[0])
			return;
		
		vector newPos = m_Members[0].GetPosition();

		if (vector.DistanceSq(newPos, m_LeaderPosOld) < m_DirRecalcDistSq)
			return;
		
		// Update the direction and perpend vectors
		m_LeaderDir = newPos - m_LeaderPosOld;
		m_LeaderDir.Normalize();
		m_LeaderDirPerp = m_LeaderDir.Perpend(); // This should come out normalized already.
		
		// Set the old pos for next time
		m_LeaderPosOld = newPos;
	}
	
	// Get the world location of a position within the formation.
	vector FormToWorld(vector formationOffset) {
		vector FormBasePos;
		
		if (m_Members[0])
			FormBasePos = m_Members[0].GetPosition();
		else
			FormBasePos = m_LeaderPosOld;
		
		return FormBasePos + (m_LeaderDir * formationOffset[2]) + (m_LeaderDirPerp * formationOffset[0]);
	}
	
	// Todo: build a class called FormationBase and do polymorphism for different types of forms
	// First number is horizontal offset, sec number is forwards in the formation
	// Could also do this algorithmically instead of case based but meh
	vector LocalFormPos(int member_no) {
		switch (member_no) {
			case 0: return "0 0 0";
			case 1: return "2 0 -2";
			case 2: return "-2 0 -2";
			case 3: return "4 0 -4";
			case 4: return "-4 0 -4";
			case 5: return "6 0 -6";
			case 6: return "-6 0 -6";
		}
		return "0 0 -6";
	}
	
	vector GetFormationMemberDest(PlayerBase member) {
		
		// Todo we will probably want to do this more slowly in a separate loop.
		UpdateFormDir();
		
		for (int i = 0; i < m_Members.Count(); i++)
			if (m_Members[i] == member)
				return FormToWorld(LocalFormPos(i));
		
		// Fallback, if not found in the group
		return "0 0 0";
	}
};