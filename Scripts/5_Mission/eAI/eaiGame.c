// Copyright 2021 William Bowers
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

class eAIGame {
	
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
			GetRPCManager().AddRPC("eAI", "eAIAimDetails", m_ServerAimMngr, SingeplayerExecutionType.Server);
		}
		
		GetRPCManager().AddRPC("eAI", "SpawnEntity", this, SingeplayerExecutionType.Server);
		GetRPCManager().AddRPC("eAI", "MoveAllToPos", this, SingeplayerExecutionType.Server);
		GetRPCManager().AddRPC("eAI", "DebugFire", this, SingeplayerExecutionType.Server);
		GetRPCManager().AddRPC("eAI", "DebugParticle", this, SingeplayerExecutionType.Server);
		GetRPCManager().AddRPC("eAI", "SpawnZombie", this, SingeplayerExecutionType.Server);
		//GetRPCManager().AddRPC("eAI", "DebugWeaponLocation", this, SingeplayerExecutionType.Server);
		//GetRPCManager().AddRPC("eAI", "SpawnBullet", this, SingeplayerExecutionType.Server);
		GetRPCManager().AddRPC("eAI", "DayZPlayerInventory_OnEventForRemoteWeaponAICallback", this, SingeplayerExecutionType.Server);
		//GetRPCManager().AddRPC("eAI", "ClientWeaponDataWithCallback", this, SingeplayerExecutionType.Server);
		//GetRPCManager().AddRPC("eAI", "ServerWeaponAimCheck", this, SingeplayerExecutionType.Server);
    }
	
	// TEMPORARY FIX THAT WILL ONLY WORK WITH ONE PERSON ONLINE
	autoptr eAIGroup m_group = new eAIGroup();
	
	//! @param owner Who is the manager of this AI
	//! @param formOffset Where should this AI follow relative to the formation?
	void SpawnAI_Helper(DayZPlayer owner) {
		PlayerBase pb_Human;
		if (!Class.CastTo(pb_Human, owner)) return;

		eAIBase pb_AI;
		if (!Class.CastTo(pb_AI, GetGame().CreatePlayer(null, "SurvivorF_Linda", pb_Human.GetPosition() + debug_offset, 0, "NONE"))) return;

		m_group.SetLeader(pb_Human);
		pb_AI.SetAI(m_group);
			
		SoldierLoadout.Apply(pb_AI);
	}
	
	// Server Side: This RPC spawns a helper AI next to the player, and tells them to join the player's formation.
	void SpawnEntity(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target) {
		Param1<DayZPlayer> data(null);
        if (!ctx.Read(data)) return;
		
		if (IsMissionOffline()) data.param1 = GetGame().GetPlayer();
		
		if(type == CallType.Server )
		{
            Print("eAI spawn entity RPC called.");
			SpawnAI_Helper(data.param1);
			
			//SpawnAI_Helper(data.param1, Vector(-3, 0, -3));
			//SpawnAI_Helper(data.param1, Vector(3, 0, -3));
			//SpawnAI_Helper(data.param1, Vector(-3, 0, 3));
			//SpawnAI_Helper(data.param1, Vector(3, 0, 3));
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
	
	void OnKeyPress(int key) {
        switch (key) {
            case KeyCode.KC_K: {
				GetRPCManager().SendRPC("eAI", "SpawnEntity", new Param1<DayZPlayer>(GetGame().GetPlayer()));
                break;
            }
			case KeyCode.KC_N: {
				GetRPCManager().SendRPC("eAI", "MoveAllToPos", new Param1<vector>(GetGame().GetPlayer().GetPosition()));
				break;
			}
			case KeyCode.KC_B: {
				GetRPCManager().SendRPC("eAI", "SpawnZombie", new Param1<vector>(GetGame().GetPlayer().GetPosition()));
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

		GetDayZGame().eAICreateManager();

        Print( "eAI - Loaded Server Mission" );
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