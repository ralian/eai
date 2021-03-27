class eAIFormation {
	// Return a unit's assigned position relative to the formation
	// First number is horizontal offset, sec number is forwards in the formation
	vector LocalFormPos(int member_no) {return "0 0 0";}
	
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