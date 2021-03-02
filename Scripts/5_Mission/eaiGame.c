class eAIGame {
	// List of all eAI entities
	autoptr array<PlayerBase> aiList = new array<PlayerBase>();
	
	vector debug_offset = "-25 0 0"; // Offset from player to spawn a new AI entity at when debug called
	
	float gametime = 0;
	
    void eAIGame() {
        GetRPCManager().AddRPC("eAI", "TestRPCFunction", this, SingeplayerExecutionType.Client);
		GetRPCManager().AddRPC("eAI", "SpawnEntity", this, SingeplayerExecutionType.Client);
		GetRPCManager().AddRPC("eAI", "ClearAllEntity", this, SingeplayerExecutionType.Client);
		GetRPCManager().AddRPC("eAI", "ProcessReload", this, SingeplayerExecutionType.Client);
    }

    void TestRPCFunction(CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target) {
        Param1< string > data;
        if ( !ctx.Read( data ) ) return;
        
        if( type == CallType.Server ) {
            Print("eAI TestRPCFunction (HLynge) Server function called! " + data.param1);
        } else {
            Print("eAI TestRPCFunction (HLynge) Client function called! " + data.param1);
        }
    }
	
	void SpawnEntity(CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target) {
		Param1<DayZPlayer> data;
        if (!ctx.Read(data)) return;
		if(type == CallType.Server ) {
            Print("eAI spawn entity RPC called.");
        //}
		//Human h = Human.Cast(GetGame().CreateObject("SurvivorF_Linda", data.param1));
			//SurvivorM_Cyril
		PlayerBase h = PlayerBase.Cast(GetGame().CreatePlayer(null, "SurvivorF_Linda", data.param1.GetPosition() + debug_offset, 0, "NONE"));
		h.markAI(); // Important: Mark unit as AI since we don't control the constructor.
		h.GetInventory().CreateInInventory("TTSKOPants");
		h.GetInventory().CreateInInventory("TTsKOJacket_Camo");
		h.GetInventory().CreateInInventory("CombatBoots_Black");
		h.GetInventory().CreateInInventory("ImprovisedBag");

		h.GetInventory().CreateInInventory("SodaCan_Pipsi");
		h.GetInventory().CreateInInventory("SpaghettiCan");
		h.GetInventory().CreateInInventory("HuntingKnife");
		ItemBase rags = ItemBase.Cast(h.GetInventory().CreateInInventory("Rag"));
		rags.SetQuantity(4);

		EntityAI primary;
		EntityAI axe = h.GetInventory().CreateInInventory("FirefighterAxe");

		EntityAI gun = h.GetHumanInventory().CreateInHands("M4A1");
		gun.GetInventory().CreateAttachment("M4_RISHndgrd_Black");
		gun.GetInventory().CreateAttachment("M4_MPBttstck_Black");
		gun.GetInventory().CreateAttachment("ACOGOptic");
		EntityAI mag = h.GetInventory().CreateInInventory("Mag_STANAG_30Rnd");
		h.GetInventory().CreateInInventory("Mag_STANAG_30Rnd");
		
		// Set the target entity we should follow to the player that spawned it, then do the first pathfinding update
		h.eAIFollow(data.param1, 2);
		h.eAIUpdateBrain();
		
		aiList.Insert(h);
			
		}
	}
	
	void ClearAllEntity(CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target) {
		Param1<vector> data; // here the parameter is unused, maybe we could use an enum instead
        if (!ctx.Read(data)) return;
		if(type == CallType.Server ) {
            Print("eAI clear all entity RPC called.");
			foreach (PlayerBase e : aiList) {
				GetGame().ObjectDelete(e); // This is almost certainly not the right way to do this.
				// Need to check for mem leaks
			}
			
			aiList.Clear();
		}
	}
	
	void ProcessReload(CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target) {
		Param1<PlayerBase> data; // here the parameter is unused, maybe we could use an enum instead
        if ( !ctx.Read( data ) ) return;
		if(type == CallType.Server) {
            Print("eAI UpdateMovement RPC called.");
			//foreach (PlayerBase p : aiList) {

			//	p.QuickReloadWeapon(p.GetHumanInventory().GetEntityInHands());
			//}
			
			foreach (PlayerBase i : aiList) {
				i.eAIUpdateBrain();
			}
        }
	}

    void OnKeyPress(int key) {
        switch (key) {
            case KeyCode.KC_K: {
                //GetRPCManager().SendRPC("eAI", "TestRPCFunction", new Param1< string >( "Hello, World!" ) );
				GetRPCManager().SendRPC("eAI", "SpawnEntity", new Param1<DayZPlayer>(GetGame().GetPlayer()));
                break;
            }
			case KeyCode.KC_L: {
				GetRPCManager().SendRPC("eAI", "ClearAllEntity", new Param1<vector>(GetGame().GetPlayer().GetPosition()));
				break;
			}
			case KeyCode.KC_M: {
				GetRPCManager().SendRPC("eAI", "ProcessReload", new Param1<vector>(GetGame().GetPlayer().GetPosition()));
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
		
		// AI pathing calculations
		foreach (PlayerBase h : aiList) {
			if (timeDiv == 0) {
				numOfDivsPassed++;
				h.eAIUpdateMovement();
			} // set a new movement target 4 times per scond
		}
		
		if (timeDiv == 0 && numOfDivsPassed % 120 == 0) { // Every 30 seconds
			// AI pathing calculations
			//foreach (PlayerBase i : aiList) {
				//i.eAIUpdateBrain();
			//}
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