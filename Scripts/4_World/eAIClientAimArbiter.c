enum eAIBoneTrust {
	TRUST_CLIENT,		// Client decides raycast
	TRUST_SERVER,		// Server decides raycast
	REQUIRE_AGREEMENT
};

// Control class that lives on the client
class eAIClientAimArbiter {
	int delay_ms = -1;
	bool flag_for_deletion = false;
	
	// Activate the Aim callback to the server, then call continuously until deactivated
	// @return returns true if the RPC was sent
	bool Activate(Weapon_Base weap, int new_delay_ms = -1) {
		if (!weap)
			return Deactivate();
		vector usti_hlavne_position = weap.GetSelectionPositionLS("usti hlavne"); // front?
		vector konec_hlavne_position = weap.GetSelectionPositionLS("konec hlavne"); // back?
		vector out_front = weap.ModelToWorld(usti_hlavne_position);
		vector out_back = weap.ModelToWorld(konec_hlavne_position);
		GetRPCManager().SendRPC("eAI", "eAIAimDetails", new Param3<Weapon_Base, vector, vector>(weap, out_front, out_back));
		if (new_delay_ms > 0)
			delay_ms = new_delay_ms;
		if (delay_ms > 0) {
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.Activate, delay_ms, false, weap);
			return true;
		}
		return false;
	}
	
	// Stops Activate from continuously being called. The RPC will be sent 1 more time if weap is non-null.
	bool Deactivate() {
		delay_ms = -1;
		flag_for_deletion = true;
		return false;
	}
};

class eAIClientAimArbiterManager {

	autoptr map<Weapon_Base, ref eAIClientAimArbiter> AimArbiterMap = new map<Weapon_Base, ref eAIClientAimArbiter>();
	
	// Todo - use MapIterator
	/*void cleanMap() {
		for (int i = 0; i < AimArbiterMap.Count(); i++)
	}*/
	
	// Insert a new entry into the map
	// @param param1 The weapon we care about
	void eAIAimArbiterSetup(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target) {
		Param1<Weapon_Base> data;
		if (!ctx.Read(data)) return;
		if (!AimArbiterMap.Get(data.param1))
			AimArbiterMap.Insert(data.param1, new eAIClientAimArbiter());
	}
	
	// Start transmission for a weapon in the map
	// @param param1 The weapon in the map
	// @param param2 The time delay between updates
	void eAIAimArbiterStart(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target) {
		Param2<Weapon_Base, int> data;
		if (!ctx.Read(data)) return;
		Print("Client: Starting Arbiter for weapon: " + data.param1.ToString());
		eAIClientAimArbiter e = AimArbiterMap.Get(data.param1);
		if (e) e.Activate(data.param1, data.param2);
	}
	
	// Stop transmission
	// @param param1 The weapon we don't care about anymore
	void eAIAimArbiterStop(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target) {
		Param1<Weapon_Base> data;
		if (!ctx.Read(data)) return;
		Print("Client: Starting Arbiter for weapon: " + data.param1.ToString());
		eAIClientAimArbiter e = AimArbiterMap.Get(data.param1);
		if (e) e.Deactivate();
	}
	
};
