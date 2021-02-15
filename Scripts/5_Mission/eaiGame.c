class eAIGame {
	
	vector debug_offset = "5 0 0"; // Offset from player to spawn a new AI entity at when debug called
	
    void eAIGame() {
        GetRPCManager().AddRPC("eAI", "TestRPCFunction", this, SingeplayerExecutionType.Client);
		GetRPCManager().AddRPC("eAI", "SpawnEntity", this, SingeplayerExecutionType.Client);
    }

    void TestRPCFunction(CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target) {
        Param1< string > data;
        if ( !ctx.Read( data ) ) return;
        
        if( type == CallType.Server )
        {
            Print( "eAI TestRPCFunction (HLynge) Server function called! " + data.param1);
        }
        else
        {
            Print( "eAI TestRPCFunction (HLynge) Client function called! " + data.param1);
        }
    }
	
	void SpawnEntity(CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target) {
		Param1< vector > data;
        if ( !ctx.Read( data ) ) return;
		if( type == CallType.Server )
        {
            Print( "eAI spawn entity RPC called.");
        }
		GetGame().CreatePlayer(null, "SurvivorF_Linda", data.param1, 0, "NONE");
	}

    void OnKeyPress( int key )
    {
        switch ( key )
        {
            case KeyCode.KC_K:
            {
                GetRPCManager().SendRPC("eAI", "TestRPCFunction", new Param1< string >( "Hello, World!" ) );
                break;
            }
			case KeyCode.KC_L:
			{
				GetRPCManager().SendRPC("eAI", "SpawnEntity", new Param1< vector >( GetGame().GetPlayer().GetPosition() + debug_offset ) );
				break;
			}
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