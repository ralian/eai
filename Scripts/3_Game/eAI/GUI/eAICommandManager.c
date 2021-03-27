class eAICommandManager {
	bool Send(eAICommands cmd) {}
}

// This class handles the inputs from the eAICommandMenu locally and shoots RPCs to the server.
class eAICommandManagerClient : eAICommandManager {
	override bool Send(eAICommands cmd) {
		switch (cmd) {
			case eAICommands.DEB_SPAWNALLY:
				GetRPCManager().SendRPC("eAI", "SpawnEntity", new Param1<DayZPlayer>(GetGame().GetPlayer()));
				return true;
			
		} return false;
	}
};