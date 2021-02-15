class eAIGame {
	// List of all eAI entities
	autoptr array<DayZPlayerImplement> aiList = new array<DayZPlayerImplement>();
	
	vector debug_offset = "5 0 0"; // Offset from player to spawn a new AI entity at when debug called

	
    void eAIGame() {
        GetRPCManager().AddRPC("eAI", "TestRPCFunction", this, SingeplayerExecutionType.Client);
		GetRPCManager().AddRPC("eAI", "SpawnEntity", this, SingeplayerExecutionType.Client);
		GetRPCManager().AddRPC("eAI", "ClearAllEntity", this, SingeplayerExecutionType.Client);
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
		Param1< vector > data;
        if ( !ctx.Read( data ) ) return;
		if( type == CallType.Server ) {
            Print( "eAI spawn entity RPC called.");
        //}
		//Human h = Human.Cast(GetGame().CreateObject("SurvivorF_Linda", data.param1));
		DayZPlayerImplement h = DayZPlayerImplement.Cast(GetGame().CreatePlayer(null, "SurvivorF_Linda", data.param1, 0, "NONE"));
		h.GetInventory().CreateInInventory("TTSKOPants");
		h.GetInventory().CreateInInventory("TTsKOJacket_Camo");
		h.GetInventory().CreateInInventory("CombatBoots_Black");
		h.GetInventory().CreateInInventory("ImprovisedBag");

		h.GetInventory().CreateInInventory("SodaCan_Pipsi");
		h.GetInventory().CreateInInventory("SpaghettiCan");
		h.GetInventory().CreateInInventory("HuntingKnife");
		ItemBase rags = h.GetInventory().CreateInInventory("Rag");
		rags.SetQuantity(4);

		EntityAI primary;
		EntityAI axe = h.GetInventory().CreateInInventory("FirefighterAxe");

		EntityAI gun = h.GetHumanInventory().CreateInHands("M4A1");
		gun.GetInventory().CreateAttachment("M4_RISHndgrd_Black");
		gun.GetInventory().CreateAttachment("M4_MPBttstck_Black");
		gun.GetInventory().CreateAttachment("ACOGOptic");
		h.GetInventory().CreateInInventory("Mag_STANAG_30Rnd");
		
		//h.StartCommand_Action(DayZPlayerConstants.CMD_ACTIONFB_FILLMAG);

		//HumanCommandMove movement = h.StartCommand_Move();
		//movement.ForceStance(DayZPlayerConstants.STANCEIDX_CROUCH);
		//movement
		
		aiList.Insert(h);
			
		}
	}
	
	void ClearAllEntity(CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target) {
		Param1< vector > data; // here the parameter is unused, maybe we could use an enum instead
        if ( !ctx.Read( data ) ) return;
		if( type == CallType.Server )
        {
            Print( "eAI clear all entity RPC called.");
        }
		
		foreach (DayZPlayerImplement e : aiList) {
			GetGame().ObjectDelete(e); // This is almost certainly not the right way to do this.
			// Need to check for mem leaks
		}
		
		aiList.Clear();
	}

    void OnKeyPress(int key) {
        switch (key) {
            case KeyCode.KC_K: {
                //GetRPCManager().SendRPC("eAI", "TestRPCFunction", new Param1< string >( "Hello, World!" ) );
				GetRPCManager().SendRPC("eAI", "SpawnEntity", new Param1< vector >( GetGame().GetPlayer().GetPosition() + debug_offset ) );
                break;
            }
			case KeyCode.KC_L: {
				GetRPCManager().SendRPC("eAI", "ClearAllEntity", new Param1< vector >( GetGame().GetPlayer().GetPosition()) );
				break;
			}
        }
    }
	
	void OnUpdate(bool doSim, float timeslice) {
		foreach (DayZPlayerImplement h : aiList) {
			h.eAIUpdateMovement();
			//Print("OnUpdate debug triggered!");
			// overriding this doesn't work?
			//h.GetInputController().OverrideAimChangeX(true, 0.5);
			h.GetInputController().OverrideMovementSpeed(true, 2.0);
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