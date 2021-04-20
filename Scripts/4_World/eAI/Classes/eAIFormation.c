class eAIFormation
{
	// The LeaderPosOld is used to calculate the heading of the formation
	// Recalculation of azumith is done only when m_DirRecalcDistSq is surpassed
	private vector m_LeaderPosOld = "0 0 0";
	private float m_DirRecalcDistSq = 9.0; // every 3 meters
	private vector m_LeaderDir = "0 0 0";
	private vector m_LeaderDirPerp = "0 0 0";

	private eAIGroup m_Group;

	void eAIFormation(eAIGroup group)
	{
		m_Group = group;
	}

	// Return a unit's assigned position relative to the formation
	// First number is horizontal offset, sec number is forwards in the formation
	vector LocalFormPos(int member_no) {return "0 0 0";}

	vector GetPosition(int member_no)
	{
		return FormToWorld(LocalFormPos(member_no));
	}

    private vector FormToWorld(vector formationOffset)
    {
		vector formBasePos = m_LeaderPosOld;
		if (m_Group.Count() > 0) formBasePos = m_Group.GetLeader().GetPosition();
		
		return formBasePos + (m_LeaderDir * formationOffset[2]) + (m_LeaderDirPerp * formationOffset[0]);
	}

    void Update(float pDt)
    {
		if (!m_Group.GetLeader()) return;
		
		vector newPos = m_Group.GetLeader().GetPosition();

		if (vector.DistanceSq(newPos, m_LeaderPosOld) < m_DirRecalcDistSq) return;
		
		// Update the direction and perpend vectors
		m_LeaderDir = newPos - m_LeaderPosOld;
		m_LeaderDir.Normalize();
		m_LeaderDirPerp = m_LeaderDir.Perpend(); // This should come out normalized already.
		
		// Set the old pos for next time
		m_LeaderPosOld = newPos;
    }
	
	// Unused thus far
	void SetScale(float separation) {}
	void SetSize(int num_of_people) {}
};

class eAIFormationVee : eAIFormation {
	override vector LocalFormPos(int member_no) {
		int offset = Math.Floor((member_no + 1) / 2);
		float scaled_offset = 2 * offset;
		if (member_no % 2 == 0) return Vector(scaled_offset, 0, -scaled_offset); // Right side
		return Vector(-scaled_offset, 0, -scaled_offset); // Left Side
	}
};

class eAIFormationWall : eAIFormation {
	override vector LocalFormPos(int member_no) {
		int offset = Math.Floor((member_no + 1) / 2);
		float scaled_offset = 2 * offset;
		if (member_no % 2 == 0) return Vector(scaled_offset, 0, 0); // Right side
		return Vector(-scaled_offset, 0, 0); // Left Side
	}
};

class eAIFormationFile : eAIFormation {
	override vector LocalFormPos(int member_no) {
		return Vector(0, 0, -2 * member_no);
	}
};

class eAIFormationColumn : eAIFormation {
	override vector LocalFormPos(int member_no) {
		int offset = Math.Floor((member_no + 1) / 2);
		float scaled_offset = 2 * offset;
		if (member_no % 2 == 0) return Vector(2, 0, -scaled_offset); // Right side
		return Vector(-2, 0, -scaled_offset); // Left Side
	}
};