class eAIGroup
{
	private static int m_IDCounter = 0;

	private autoptr array<eAITargetInformation> m_Targets;
	private int m_ID;

	// Ordered array of group members. 0 is the leader.
	autoptr array<PlayerBase> m_Members = {};
	
	// The LeaderPosOld is used to calculate the heading of the formation
	// Recalculation of azumith is done only when m_DirRecalcDistSq is surpassed
	vector m_LeaderPosOld = "0 0 0";
	float m_DirRecalcDistSq = 25.0;
	vector m_LeaderDir = "0 0 0";
	vector m_LeaderDirPerp = "0 0 0";

	void eAIGroup()
	{
		m_Targets = new array<eAITargetInformation>();

		m_IDCounter++;
		m_ID = m_IDCounter;
	}

	int GetID()
	{
		return m_ID;
	}

	void OnTargetAdded(eAITargetInformation target)
	{
		m_Targets.Insert(target);
	}

	void OnTargetRemoved(eAITargetInformation target)
	{
		m_Targets.RemoveItem(target);
	}

	void ProcessTargets()
	{
		for (int i = m_Targets.Count() - 1; i >= 0; i--) m_Targets[i].Process(m_ID);
	}

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
	
	bool RemoveMember(int i) {
		if (i < m_Members.Count()) {
			m_Members.RemoveOrdered(i);
			return true;
		}
		
		return false;
	}
	
	PlayerBase GetMember(int i) {
		return m_Members[i];
	}
	
	int Count() {
		return m_Members.Count();
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
	vector LocalFormPos(int member_no) {
		int offset = Math.Floor((member_no+1)/2);
		float scaled_offset = 2*offset;
		if (member_no % 2 == 0)
			return Vector(scaled_offset, 0, -scaled_offset); // Right side
		else
			return Vector(-scaled_offset, 0, -scaled_offset); // Left Side
	}
	
	// get the global formation position of the i'th formation member
	vector GlobalFormPos(int i) {
		return FormToWorld(LocalFormPos(i));
	}
	
	// Get where a specific member of the group should run to
	vector GetFormationMemberDest(PlayerBase member) {
		
		// Todo we will probably want to do this more slowly in a separate loop.
		UpdateFormDir();
		
		for (int i = 0; i < m_Members.Count(); i++)
			if (m_Members[i] == member)
				return GlobalFormPos(i);
		
		// Fallback, if not found in the group
		return "0 0 0";
	}
};