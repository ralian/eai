class eAIGame {
	// List of all eAI entities
	autoptr array<ref eAIPlayerHandler> aiList = {};
	
	// On client, list of weapons we are asked to provide a feed of
	autoptr eAIClientAimArbiterManager m_ClientAimMngr;
	
	// Server side list of weapon data 
	autoptr eAIServerAimProfileManager m_ServerAimMngr;
	
	vector debug_offset = "8 0 0"; // Offset from player to spawn a new AI entity at when debug called
	vector debug_offset_2 = "20 0 0"; // Electric bugaloo
	
	float gametime = 0;
	
    void eAIGame() {
		if (GetGame().IsClient()) {
			m_ClientAimMngr = new eAIClientAimArbiterManager();
			GetRPCManager().AddRPC("eAI", "eAIAimArbiterSetup", m_ClientAimMngr, SingeplayerExecutionType.Client);
			GetRPCManager().AddRPC("eAI", "eAIAimArbiterStart", m_ClientAimMngr, SingeplayerExecutionType.Client);
			GetRPCManager().AddRPC("eAI", "eAIAimArbiterStop", m_ClientAimMngr, SingeplayerExecutionType.Client);
		}
		
		if (GetGame().IsServer()) {
			m_ServerAimMngr = new eAIServerAimProfileManager();
			GetRPCManager().AddRPC("eAI", "eAIAimDetails", m_ServerAimMngr, SingeplayerExecutionType.Client);
		}
		
		GetRPCManager().AddRPC("eAI", "SpawnEntity", this, SingeplayerExecutionType.Client);
		GetRPCManager().AddRPC("eAI", "ClearAllEntity", this, SingeplayerExecutionType.Client);
		GetRPCManager().AddRPC("eAI", "ClearMyEntity", this, SingeplayerExecutionType.Client);
		GetRPCManager().AddRPC("eAI", "TargetPos", this, SingeplayerExecutionType.Client);
		GetRPCManager().AddRPC("eAI", "ProcessReload", this, SingeplayerExecutionType.Client);
		GetRPCManager().AddRPC("eAI", "MoveAllToPos", this, SingeplayerExecutionType.Client);
		GetRPCManager().AddRPC("eAI", "UpdateMovement", this, SingeplayerExecutionType.Client);
		GetRPCManager().AddRPC("eAI", "DebugFire", this, SingeplayerExecutionType.Client);
		GetRPCManager().AddRPC("eAI", "DebugParticle", this, SingeplayerExecutionType.Client);
		GetRPCManager().AddRPC("eAI", "ToggleWeaponRaise", this, SingeplayerExecutionType.Client);
		GetRPCManager().AddRPC("eAI", "SpawnZombie", this, SingeplayerExecutionType.Client);
		//GetRPCManager().AddRPC("eAI", "DebugWeaponLocation", this, SingeplayerExecutionType.Client);
		//GetRPCManager().AddRPC("eAI", "SpawnBullet", this, SingeplayerExecutionType.Client);
		GetRPCManager().AddRPC("eAI", "DayZPlayerInventory_OnEventForRemoteWeaponAICallback", this, SingeplayerExecutionType.Client);
		//GetRPCManager().AddRPC("eAI", "ClientWeaponDataWithCallback", this, SingeplayerExecutionType.Client);
		//GetRPCManager().AddRPC("eAI", "ServerWeaponAimCheck", this, SingeplayerExecutionType.Client);
    }
	
	//! @param owner Who is the manager of this AI
	//! @param formOffset Where should this AI follow relative to the formation?
	void SpawnAI_Helper(PlayerBase owner, vector formOffset) {
		//Human h = Human.Cast(GetGame().CreateObject("SurvivorF_Linda", data.param1));

		PlayerBase h = PlayerBase.Cast(GetGame().CreatePlayer(null, "SurvivorF_Linda", owner.GetPosition() + debug_offset, 0, "NONE"));
			
		h.markAIServer( ); // Important: Mark unit as AI since we don't control the constructor.
		 // Do the same in the clients
			
		SoldierLoadout.Apply(h);

		eAIPlayerHandler handler = new eAIPlayerHandler(h);
		h.markOwner(handler);
		
		aiList.Insert(handler);
		
		// Set the target entity we should follow to the player that spawned it, then do the first pathfinding update
		handler.Follow(owner, formOffset, 2);
		handler.UpdatePathing();
	}
	
	// Server Side: This RPC spawns a helper AI next to the player, and tells them to join the player's formation.
	void SpawnEntity(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target) {
		Param1<DayZPlayer> data;
        if (!ctx.Read(data)) return;
		if(type == CallType.Server ) {
            Print("eAI spawn entity RPC called.");
			SpawnAI_Helper(data.param1, Vector(0, 0, 0));
			//SpawnAI_Helper(data.param1, Vector(-3, 0, -3)); // First number is horizontal offset, sec number is forwards in the formation
			//SpawnAI_Helper(data.param1, Vector(3, 0, -3));
		}
	}
	
	// Server Side: This RPC deletes all the active AI.
	void ClearAllEntity(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target) {
		Param1<vector> data;
        if (!ctx.Read(data)) return;
		if(type == CallType.Server ) {
            Print("eAI clear all entity RPC called.");
			foreach (eAIPlayerHandler e : aiList) {
				GetGame().ObjectDelete(e.unit); // This is almost certainly not the right way to do this.
				// Need to check for mem leaks
			}
			
			aiList.Clear();
		}
	}
	
	// Server Side: This RPC clears all entities belonging to a particular player. It is garbage code and needs rewritten.
	void ClearMyEntity(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target) {
		Param1<DayZPlayer> data;
        if (!ctx.Read(data)) return;
		if(type == CallType.Server ) {
            Print("eAI clear my entity RPC called.");
			for (int i = 0; i < aiList.Count(); i++) {
				if (aiList[i].m_FollowOrders == data.param1) {
					GetGame().ObjectDelete(aiList[i].unit);
					aiList.RemoveOrdered(i);
				}
			}
			
			// hacky workaround for now
			for (int j = 0; j < aiList.Count(); j++) {
				if (aiList[j].m_FollowOrders == data.param1) {
					GetGame().ObjectDelete(aiList[j].unit);
					aiList.RemoveOrdered(j);
				}
			}
			
		}
	}
	
	// Server Side: This RPC tells AI that belong to a player to target creatures that are pointed at by the player.
	void TargetPos(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target) {
		Param2<DayZPlayer, vector> data;
        if (!ctx.Read(data)) return;
		if(type == CallType.Server ) {
			//Print("eAI TargetPOS " + data.param1 + data.param2);
			for (int i = 0; i < aiList.Count(); i++) {
				if (aiList[i].m_FollowOrders == data.param1) {
					aiList[i].RecalcThreatList(data.param2);
				}
			}
			
		}
	}
	
	// Server Side: This RPC forces all units to reload if they have a weapon, for debugging.
	void ProcessReload(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target) {
		Param1<PlayerBase> data;
        if ( !ctx.Read( data ) ) return;
		if(type == CallType.Server || true) {
            Print("eAI ProcessReload RPC called.");
			foreach (eAIPlayerHandler p : aiList) {
				//GetRPCManager().SendRPC("eAI", data.param1, new Param1<PlayerBase>(p));
				//GetRPCManager().SendRPC("eAI", "ProcessReload", new Param1<PlayerBase>(p));
				p.unit.QuickReloadWeapon(p.unit.GetHumanInventory().GetEntityInHands());
			}
        }
	}
	
	// Server Side: This RPC forces all AI to recalc their pathing.
	void UpdateMovement(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target) {
		Param1<PlayerBase> data;
        if ( !ctx.Read( data ) ) return;
		if(type == CallType.Server) {
            Print("eAI UpdateMovement RPC called.");
			foreach (eAIPlayerHandler i : aiList) {
				i.UpdatePathing();
			}
        }
	}
	
	// Server Side: This RPC forces all AI to fire their held weapon for debugging.
	void DebugFire(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target) {
		Param1<PlayerBase> data;
        if ( !ctx.Read( data ) ) return;
		if(type == CallType.Server) {
            Print("eAI DebugFire RPC called.");
			foreach (eAIPlayerHandler i : aiList) {
				i.FireHeldWeapon();
			}
        }
	}
	
	// Server Side: This RPC forces all AI to toggle ADS, for debugging.
	void ToggleWeaponRaise(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target) {
		Param1<PlayerBase> data;
        if ( !ctx.Read( data ) ) return;
		if(type == CallType.Server) {
            Print("eAI ToggleWeaponRaise RPC called.");
			foreach (eAIPlayerHandler i : aiList) {
				i.ToggleWeaponRaise();
			}
        }
	}
	
	// Server Side: This RPC spawns a zombie. It's actually not the right way to do it. But it's only for testing.
	// BUG: this has sometimes crashed us before. Not sure why yet.
	void SpawnZombie(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target) {
		Param1<PlayerBase> data;
        if ( !ctx.Read( data ) ) return;
		if(type == CallType.Server) {
            Print("eAI SpawnZombie RPC called.");
			GetGame().CreateObject("ZmbF_JournalistNormal_Blue", data.param1.GetPosition() + debug_offset_2, false, true, true);
        }
	}
	
	// Server Side: TP all players to the caller's position. For testing only.
	void MoveAllToPos(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target) {
		Param1<vector> data;
        if (!ctx.Read(data)) return;
		if(type == CallType.Server ) {
            Print("Moving all units to position...");
			array<Man> players = new array<Man>();
			GetGame().GetPlayers(players);
			foreach (Man p : players) {
				p.SetPosition(data.param1);
			}
		}
	}
	
	// Client Side: This RPC spawns a debug particle at a location requested by the server.
	void DebugParticle(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target) {
		Param2<vector, vector> data;
        if (!ctx.Read(data)) return;
		if(type == CallType.Client ) {
			Particle p = Particle.PlayInWorld(ParticleList.DEBUG_DOT, data.param1);
			p.SetOrientation(data.param2);			
		}
	}
	
	// Client Side: This RPC replaces a member function of DayZPlayerInventory that handles remote weapon events. I cannot override the functionality that 
	// class, but this workaround seems to do a pretty good job.
	void DayZPlayerInventory_OnEventForRemoteWeaponAICallback(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target) {
		Param3<int, DayZPlayer, Magazine> data;
        if (!ctx.Read(data)) return;
		//if(type == CallType.Client ) {
			Print("Received weapon event for " + data.param1.ToString() + " player:" + data.param2.ToString() + " mag:" + data.param3.ToString());
            DayZPlayerInventory_OnEventForRemoteWeaponAI(data.param1, data.param2, data.param3);
		//}
	}
	
	// Client Side: This RPC gets the client side transformation of a Weapon_Base, then sends some data back to server
	void DebugWeaponLocation(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target) {
		Param2<Weapon_Base, string> data;
        if (!ctx.Read(data)) return;
		
		// Set up the model space transformation locally on the client
		vector usti_hlavne_position = data.param1.GetSelectionPositionLS("usti hlavne"); // front?
		vector konec_hlavne_position = data.param1.GetSelectionPositionLS("konec hlavne"); // back?
		vector out_front, out_back;
		out_front = data.param1.ModelToWorld(usti_hlavne_position);
		out_back = data.param1.ModelToWorld(konec_hlavne_position);
		
		// Now, sync data to server.
		GetRPCManager().SendRPC("eAI", "SpawnBullet", new Param3<Weapon_Base, vector, vector>(data.param1, out_front, out_back));
	}
	
	// Client Side: This RPC takes the weapon data like in the previous RPC, then forwards it to the ServerWeaponAimCheck RPC
	void ClientWeaponDataWithCallback(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target) {
		Param2<Weapon_Base, string> data;
        if (!ctx.Read(data)) return;

		if(type == CallType.Client ) {
			vector usti_hlavne_position = data.param1.GetSelectionPositionLS("usti hlavne"); // front?
			vector konec_hlavne_position = data.param1.GetSelectionPositionLS("konec hlavne"); // back?
			vector out_front, out_back;
			out_front = data.param1.ModelToWorld(usti_hlavne_position);
			out_back = data.param1.ModelToWorld(konec_hlavne_position);
		
			// Now, sync data to server.
			GetRPCManager().SendRPC("eAI", data.param2, new Param3<Weapon_Base, vector, vector>(data.param1, out_front, out_back));
		}
		else {Error("ClientWeaponDataWithCallback called wrongfully");}
	}
	
	// from weapon_base, was originally protected
	PhxInteractionLayers hit_mask = PhxInteractionLayers.CHARACTER | PhxInteractionLayers.BUILDING | PhxInteractionLayers.DOOR | PhxInteractionLayers.VEHICLE | PhxInteractionLayers.ROADWAY | PhxInteractionLayers.TERRAIN | PhxInteractionLayers.ITEM_SMALL | PhxInteractionLayers.ITEM_LARGE | PhxInteractionLayers.FENCE | PhxInteractionLayers.AI;
	
	// Server Side: This RPC takes the client location data, and performs an aim check on the weapon's last known aimpoint.
	void ServerWeaponAimCheck(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target) {
		
		Param3<Weapon_Base, vector, vector> data;
        if (!ctx.Read(data)) return;
		
		if(type == CallType.Server ) {
	
			vector begin_point = data.param2;
			vector back = data.param3;
			
			vector aim_point = begin_point - back;
	
			vector end_point = (500*aim_point) + begin_point;
			
			// Prep Raycast
			Object hitObject;
			vector hitPosition, hitNormal;
			float hitFraction;
			int contact_component = 0;
			DayZPhysics.RayCastBullet(begin_point, end_point, hit_mask, null, hitObject, hitPosition, hitNormal, hitFraction);
			
			// This makes no guarantees that any objects were even hit
			data.param1.whereIAmAimedAt = hitPosition;
		}
		else {Error("ServerWeaponAimCheck called wrongfully");}

	}
	
	// Server Side: Kick off the ballistics code with the latest data from client
	/*void SpawnBullet(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target) {
		Param3<Weapon_Base, vector, vector> data;
        if (!ctx.Read(data)) return;
		if(type == CallType.Server ) {
			data.param1.BallisticsPostFrame(data.param2, data.param3);		
		}
	}*/
	
	void OnKeyPress(int key) {
        switch (key) {
            case KeyCode.KC_K: {
				//GetRPCManager().SendRPC("eAI", "ClearMyEntity", new Param1<DayZPlayer>(GetGame().GetPlayer()));
				GetRPCManager().SendRPC("eAI", "SpawnEntity", new Param1<DayZPlayer>(GetGame().GetPlayer()));
                break;
            }
			case KeyCode.KC_T: {
				vector end = GetGame().GetCurrentCameraPosition() + (GetGame().GetCurrentCameraDirection().AnglesToVector() * 200);
				vector hitPos, contactDir;
				int contactComp;
				DayZPhysics.RaycastRV(GetGame().GetCurrentCameraPosition(), end, hitPos, contactDir, contactComp);
				GetRPCManager().SendRPC("eAI", "TargetPos", new Param2<DayZPlayer,vector>(GetGame().GetPlayer(), hitPos));
                break;
            }
			case KeyCode.KC_L: {
				GetRPCManager().SendRPC("eAI", "ClearAllEntity", new Param1<vector>(GetGame().GetPlayer().GetPosition()));
				break;
			}
			case KeyCode.KC_N: {
				GetRPCManager().SendRPC("eAI", "MoveAllToPos", new Param1<vector>(GetGame().GetPlayer().GetPosition()));
				break;
			}
			case KeyCode.KC_I: {
				GetRPCManager().SendRPC("eAI", "UpdateMovement", new Param1<vector>(GetGame().GetPlayer().GetPosition()));
				break;
			}
			case KeyCode.KC_P: {
				GetRPCManager().SendRPC("eAI", "DebugFire", new Param1<vector>(GetGame().GetPlayer().GetPosition()));
				break;
			}
			case KeyCode.KC_O: {
				GetRPCManager().SendRPC("eAI", "ToggleWeaponRaise", new Param1<vector>(GetGame().GetPlayer().GetPosition()));
				break;
			}
			case KeyCode.KC_M: {
				GetRPCManager().SendRPC("eAI", "ProcessReload", new Param1<vector>(GetGame().GetPlayer().GetPosition()));
				break;
			}
			case KeyCode.KC_B: {
				GetRPCManager().SendRPC("eAI", "SpawnZombie", new Param1<vector>(GetGame().GetPlayer().GetPosition()));
				break;
			}
        }
    }
	
	int numOfDivsPassed = 0; // Okay, so this is a terrible way to do this. But AI will recalculate once each time "Div," a div (currently) being 250ms. This int is the floor of # of divs that have passed.
	int timeDiv = 0; // This is the number of OnUpdates that have triggered since the last Div.
	
	int current_ai = 0;
	
	void OnUpdate(bool doSim, float timeslice) {
		gametime += (8*timeslice); // timeslice*x where x is the number of slices per second
		timeDiv++;
		if (Math.Floor(gametime - (4*timeslice)) != Math.Floor(gametime)) {timeDiv = 0;}

		// If at least one unit was killed, scan for dead units and clean up.
		if (eAIGlobal_UnitKilled) {
			for (int i = 0; i < aiList.Count(); i++)
				if (aiList[i] == null || aiList[i].isDead())
					aiList.Remove(i);
			eAIGlobal_UnitKilled = false;
		}
		
		// AI pathing calculations
		
		if (timeDiv == 0) {
			current_ai = 0;
			numOfDivsPassed++;
		}
		
		if (current_ai < aiList.Count()) {
			aiList[current_ai].UpdatePathing();
			aiList[current_ai].UpdateState();
			while (aiList[current_ai].UpdateMovement()) {} // update the movement as many times as needed (usually once, sometimes twice)
			current_ai++;
		}
		
		if (timeDiv == 0 && numOfDivsPassed % 12 == 0) { // Every 3 seconds, update all pathing
			//foreach (eAIPlayerHandler h : aiList)
				//h.UpdatePathing();
				
		}
	}
};

modded class MissionServer
{
    ref eAIGame m_eaiGame;

    void MissionServer()
    {
        m_eaiGame = new ref eAIGame();

        Print( "eAI - Loaded Server Mission" );
    }
	
	override void OnUpdate(float timeslice) {
		m_eaiGame.OnUpdate(true, timeslice);
		super.OnUpdate(timeslice);
	}
};

modded class MissionGameplay
{
    ref eAIGame m_eaiGame;

    void MissionGameplay()
    {
        m_eaiGame = new eAIGame();

        Print( "eAI - Loaded Client Mission" );
    }

    override void OnKeyPress( int key )
    {
        super.OnKeyPress( key );

        m_eaiGame.OnKeyPress( key );
    }
};