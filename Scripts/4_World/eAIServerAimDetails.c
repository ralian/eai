// Slave class with the latest data living on the server
// We are just treating this like a struct
class AimProfile {
	int lastUpdated = -1;
	vector out_front, out_back;
	void AimProfile(vector out_f, vector out_b) {
		out_front = out_f;
		out_back = out_b;
		lastUpdated = GetGame().GetTime();
	}
};

// Class managing the list of weapon world space data
class eAIServerAimProfileManager {
	//autoptr map<Weapon_Base, ref AimProfile> weapons = new map<Weapon_Base, ref AimProfile>();
	
	// todo map cleanup
	
	void eAIAimDetails(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target) {
		Param3<Weapon_Base, vector, vector> data;
		if (!ctx.Read(data)) return;
		/*AimProfile a = weapons.Get(data.param1);
		if (a) { // If we already have an eentry for this weapon
			a.lastUpdated = GetGame().GetTime();
			a.out_front = data.param2;
			a.out_back = data.param3;
		} else { // otherwise
			weapons.Insert(data.param1, new AimProfile(data.param2, data.param3));
		}*/
		
		// Add an additional temporary way to lookup the two positions
		data.param1.aim.out_front = data.param2;
		data.param1.aim.out_back = data.param3;
		data.param1.aim.lastUpdated = GetGame().GetTime();
	}
};