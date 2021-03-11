class eAIGame {
	// List of all eAI entities
	ref array<autoptr eAIPlayerHandler> aiList = new array<autoptr eAIPlayerHandler>();
	
	vector debug_offset = "-25 0 0"; // Offset from player to spawn a new AI entity at when debug called
	
	float gametime = 0;
	
    void eAIGame() {
		GetRPCManager().AddRPC("eAI", "SpawnEntity", this, SingeplayerExecutionType.Client);
		GetRPCManager().AddRPC("eAI", "ClearAllEntity", this, SingeplayerExecutionType.Client);
		GetRPCManager().AddRPC("eAI", "ProcessReload", this, SingeplayerExecutionType.Client);
		GetRPCManager().AddRPC("eAI", "MoveAllToPos", this, SingeplayerExecutionType.Client);
		GetRPCManager().AddRPC("eAI", "UpdateMovement", this, SingeplayerExecutionType.Client);
		GetRPCManager().AddRPC("eAI", "DebugFire", this, SingeplayerExecutionType.Client);
		GetRPCManager().AddRPC("eAI", "DebugParticle", this, SingeplayerExecutionType.Client);
		GetRPCManager().AddRPC("eAI", "ToggleWeaponRaise", this, SingeplayerExecutionType.Client);
		GetRPCManager().AddRPC("eAI", "SpawnZombie", this, SingeplayerExecutionType.Client);
		GetRPCManager().AddRPC("eAI", "DebugWeaponLocation", this, SingeplayerExecutionType.Client);
		GetRPCManager().AddRPC("eAI", "SpawnBullet", this, SingeplayerExecutionType.Client);
		GetRPCManager().AddRPC("eAI", "DayZPlayerInventory_OnEventForRemoteWeaponAICallback", this, SingeplayerExecutionType.Client);
    }
	
	void SpawnEntity(CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target) {
		Param1<DayZPlayer> data;
        if (!ctx.Read(data)) return;
		if(type == CallType.Server ) {
            Print("eAI spawn entity RPC called.");
			//Human h = Human.Cast(GetGame().CreateObject("SurvivorF_Linda", data.param1));
				//SurvivorM_Cyril
			PlayerBase h = PlayerBase.Cast(GetGame().CreatePlayer(null, "SurvivorF_Linda", data.param1.GetPosition() + debug_offset, 0, "NONE"));
				
			h.markAIServer( ); // Important: Mark unit as AI since we don't control the constructor.
			 // Do the same in the clients
				
			//h.OnCameraChanged(new eAIDayZPlayerCamera(h, h.GetInputController()));
				
			h.GetHumanInventory().CreateInInventory("TTSKOPants");
			h.GetHumanInventory().CreateInInventory("TTsKOJacket_Camo");
			h.GetHumanInventory().CreateInInventory("CombatBoots_Black");
			h.GetHumanInventory().CreateInInventory("ImprovisedBag");
	
			h.GetHumanInventory().CreateInInventory("SodaCan_Pipsi");
			h.GetHumanInventory().CreateInInventory("SpaghettiCan");
			h.GetHumanInventory().CreateInInventory("HuntingKnife");
			ItemBase rags = ItemBase.Cast(h.GetHumanInventory().CreateInInventory("Rag"));
			rags.SetQuantity(4);
	
			EntityAI primary;
			EntityAI axe = h.GetInventory().CreateInInventory("FirefighterAxe");
	
			EntityAI gun = h.GetHumanInventory().CreateInHands("M4A1");
			gun.GetInventory().CreateAttachment("M4_RISHndgrd_Black");
			gun.GetInventory().CreateAttachment("M4_MPBttstck_Black");
			gun.GetInventory().CreateAttachment("ACOGOptic");
			//gun.GetInventory().CreateAttachment("Mag_STANAG_30Rnd");
			EntityAI mag = h.GetHumanInventory().CreateInInventory("Mag_STANAG_30Rnd");
			h.GetHumanInventory().CreateInInventory("Mag_STANAG_30Rnd");

			eAIPlayerHandler handler = new eAIPlayerHandler(h);
			h.markOwner(handler);
			
			aiList.Insert(handler);
			
			// Set the target entity we should follow to the player that spawned it, then do the first pathfinding update
			handler.Follow(data.param1, 2);
			handler.UpdatePathing();
			
			
		}
	}
	
	void ClearAllEntity(CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target) {
		Param1<vector> data; // here the parameter is unused, maybe we could use an enum instead
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
	
	// This RPC is to be sent from a client to the server, so that the server can send the RPC (stored as data.param1 string) to all clients
	/*void ServerSendGlobalRPC(CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target) {
		Param1<string> data;
        if ( !ctx.Read( data ) ) return;
		if(type == CallType.Server) {
            Print("ServerSendGlobalRPC: Synching event " + data.param1);
			foreach (eAIPlayerHandler p : aiList) {
				//GetRPCManager().SendRPC("eAI", data.param1, new Param1<PlayerBase>(p));
				//GetRPCManager().SendRPC("eAI", "ProcessReload", new Param1<PlayerBase>(p));
				p.unit.QuickReloadWeapon(p.unit.GetHumanInventory().GetEntityInHands());
			}
        }
	}*/
	
	void ProcessReload(CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target) {
		Param1<PlayerBase> data; // here the parameter is unused, maybe we could use an enum instead
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
	
	void UpdateMovement(CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target) {
		Param1<PlayerBase> data; // here the parameter is unused, maybe we could use an enum instead
        if ( !ctx.Read( data ) ) return;
		if(type == CallType.Server) {
            Print("eAI UpdateMovement RPC called.");
			foreach (eAIPlayerHandler i : aiList) {
				i.UpdatePathing();
			}
        }
	}
	
	void DebugFire(CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target) {
		Param1<PlayerBase> data; // here the parameter is unused, maybe we could use an enum instead
        if ( !ctx.Read( data ) ) return;
		if(type == CallType.Server) {
            Print("eAI DebugFire RPC called.");
			foreach (eAIPlayerHandler i : aiList) {
				i.FireHeldWeapon();
			}
        }
	}
	
	void ToggleWeaponRaise(CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target) {
		Param1<PlayerBase> data; // here the parameter is unused, maybe we could use an enum instead
        if ( !ctx.Read( data ) ) return;
		if(type == CallType.Server) {
            Print("eAI ToggleWeaponRaise RPC called.");
			foreach (eAIPlayerHandler i : aiList) {
				//i.WeaponRaise(true);
			}
        }
	}
	
	// BUG: this has sometimes crashed us before. Not sure why yet.
	void SpawnZombie(CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target) {
		Param1<PlayerBase> data; // here the parameter is unused, maybe we could use an enum instead
        if ( !ctx.Read( data ) ) return;
		if(type == CallType.Server) {
            Print("eAI SpawnZombie RPC called.");
			GetGame().CreateObject("ZmbF_JournalistNormal_Blue", data.param1.GetPosition() + debug_offset + debug_offset, false, true, true);
        }
	}
	
	void MoveAllToPos(CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target) {
		Param1<vector> data; // here the parameter is unused, maybe we could use an enum instead
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
	
	void DebugParticle(CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target) {
		Param2<vector, vector> data;
        if (!ctx.Read(data)) return;
		if(type == CallType.Client ) {
			Particle p = Particle.PlayInWorld(ParticleList.DEBUG_DOT, data.param1);
			p.SetOrientation(data.param2);			
		}
	}
	
	void DayZPlayerInventory_OnEventForRemoteWeaponAICallback(CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target) {
		Param3<int, DayZPlayer, Magazine> data;
        if (!ctx.Read(data)) return;
		//if(type == CallType.Client ) {
			Print("Received weapon event for " + data.param1.ToString() + " player:" + data.param2.ToString() + " mag:" + data.param3.ToString());
            DayZPlayerInventory_OnEventForRemoteWeaponAI(data.param1, data.param2, data.param3);
		//}
	}
	
	void DebugWeaponLocation(CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target) {
		Param1<Weapon_Base> data;
        if (!ctx.Read(data)) return;
		
		Particle p = Particle.PlayInWorld(ParticleList.DEBUG_DOT, "0 0 0");
		///p.SetOrientation(vector.Zero);
		data.param1.AddChild(p, -1);
		//data.param1.Update();
		p.Update();
		Print(p.GetPosition());
		
		vector usti_hlavne_position = data.param1.GetSelectionPositionLS("usti hlavne"); // This is therefore the begin_point
		Particle p1 = Particle.PlayInWorld(ParticleList.DEBUG_DOT, usti_hlavne_position);
		///p1.SetOrientation(vector.Zero);
		data.param1.AddChild(p1, -1);
		//data.param1.Update();
		p1.Update();
		Print(p1.GetPosition());
		
		vector konec_hlavne_position = data.param1.GetSelectionPositionLS("konec hlavne");
		Particle p2 = Particle.PlayInWorld(ParticleList.DEBUG_DOT, konec_hlavne_position);
		p2.SetOrientation(vector.Zero);
		data.param1.AddChild(p2, -1);
		//data.param1.Update();
		p2.Update();
		Print(p2.GetPosition());
		
		// Now, sync data to server.
		GetRPCManager().SendRPC("eAI", "SpawnBullet", new Param3<Weapon_Base, vector, vector>(data.param1, p1.GetPosition(), p2.GetPosition()));
	}
	
	void SpawnBullet(CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target) {
		Param3<Weapon_Base, vector, vector> data;
        if (!ctx.Read(data)) return;
		if(type == CallType.Server ) {
			data.param1.BallisticsPostFrame(data.param2, data.param3);		
		}
	}

    void OnKeyPress(int key) {
        switch (key) {
            case KeyCode.KC_K: {
				GetRPCManager().SendRPC("eAI", "SpawnEntity", new Param1<DayZPlayer>(GetGame().GetPlayer()));
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
	void OnUpdate(bool doSim, float timeslice) {
		gametime += (4*timeslice); // timeslice*x where x is the number of slices per second
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
		foreach (eAIPlayerHandler h : aiList) {
			if (timeDiv == 0) {
				numOfDivsPassed++;
				h.UpdateState();
				while (h.UpdateMovement()) {} // update the movement as many times as needed
			} // set a new movement target 4 times per scond
		}
		
		if (timeDiv == 0 && numOfDivsPassed % 120 == 0) { // Every 30 seconds

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
        m_eaiGame = new ref eAIGame();

        Print( "eAI - Loaded Client Mission" );
    }

    override void OnKeyPress( int key )
    {
        super.OnKeyPress( key );

        m_eaiGame.OnKeyPress( key );
    }
};