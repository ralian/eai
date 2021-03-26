class eAIGroupTargetInformation extends eAITargetInformation
{
    private eAIGroup m_Target;

	// The LeaderPosOld is used to calculate the heading of the formation
	// Recalculation of azumith is done only when m_DirRecalcDistSq is surpassed
	private vector m_LeaderPosOld = "0 0 0";
	private float m_DirRecalcDistSq = 25.0;
	private vector m_LeaderDir = "0 0 0";
	private vector m_LeaderDirPerp = "0 0 0";

    void eAIGroupTargetInformation(eAIGroup target)
    {
        m_Target = target;
    }

    override EntityAI GetEntity()
    {
        return m_Target.GetLeader();
    }

	vector GetLeaderPosition()
	{
		if (GetEntity()) return GetEntity().GetPosition();

		return m_LeaderPosOld;
	}

    override vector GetPosition(eAIBase ai = null)
    {
        if (ai.GetGroup() == m_Target)
        {
            int index = m_Target.GetIndex(ai);
			vector localPos = LocalFormPos(index);
			vector worldPos = FormToWorld(localPos);
            return worldPos;
        }

        return GetLeaderPosition();
    }

	// Todo: build a class called FormationBase and do polymorphism for different types of forms
	// First number is horizontal offset, sec number is forwards in the formation
	vector LocalFormPos(int member_no)
    {
		int offset = Math.Floor((member_no + 1) / 2);
		float scaled_offset = 2 * offset;
		if (member_no % 2 == 0) return Vector(scaled_offset, 0, -scaled_offset); // Right side
		
		return Vector(-scaled_offset, 0, -scaled_offset); // Left Side
	}

    private vector FormToWorld(vector formationOffset)
    {
		vector formBasePos = m_LeaderPosOld;
		if (m_Target.Count() > 0) formBasePos = m_Target.GetLeader().GetPosition();
		
		return formBasePos + (m_LeaderDir * formationOffset[2]) + (m_LeaderDirPerp * formationOffset[0]);
	}

    void Update(float pDt)
    {
		for (int i = 0; i < m_Target.Count(); i++)
		{
			//Print("" + i + "  " + m_Target.GetMember(i));
		}
		
		vector newPos = GetLeaderPosition();
		if (vector.DistanceSq(newPos, m_LeaderPosOld) < m_DirRecalcDistSq)
			return;
		
		// Update the direction and perpend vectors
		m_LeaderDir = newPos - m_LeaderPosOld;
		m_LeaderDir.Normalize();
		m_LeaderDirPerp = m_LeaderDir.Perpend(); // This should come out normalized already.
		
		// Set the old pos for next time
		m_LeaderPosOld = newPos;
    }
};