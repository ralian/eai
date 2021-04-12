class eAIFaction {
	protected string name;
	string getName() {return name;}
	bool isFriendly(eAIFaction other) {
		return true;
	}
};

class eAIFactionRaiders : eAIFaction { // this is the faction that seeks the Lost Ark
	void eAIFactionRaiders() {name = "Raiders";}
	override bool isFriendly(eAIFaction other) {
		return false;
	}
};

class eAIFactionWest : eAIFaction {
	void eAIFactionWest() {name = "West";}
	override bool isFriendly(eAIFaction other) {
		if (other.getName() == "West") return true;
		if (other.getName() == "Civilian") return true;
		return false;
	}
};

class eAIFactionEast : eAIFaction {
	void eAIFactionEast() {name = "Raiders";}
	override bool isFriendly(eAIFaction other) {
		if (other.getName() == "Raiders") return true;
		if (other.getName() == "Civilian") return true;
		return false;
	}
};

class eAIFactionCivilian : eAIFaction {
	void eAIFactionCivilian() {name = "Civilian";}
	override bool isFriendly(eAIFaction other) {
		return true;
	}
};