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
		GetRPCManager().AddRPC("eAI", "DebugFire", this, SingeplayerExecutionType.Server);
		GetRPCManager().AddRPC("eAI", "DebugParticle", this, SingeplayerExecutionType.Server);
		GetRPCManager().AddRPC("eAI", "SpawnZombie", this, SingeplayerExecutionType.Server);
		GetRPCManager().AddRPC("eAI", "ClearAllAI", this, SingeplayerExecutionType.Server);
		GetRPCManager().AddRPC("eAI", "ProcessReload", this, SingeplayerExecutionType.Server);
		GetRPCManager().AddRPC("eAI", "ReqFormationChange", this, SingeplayerExecutionType.Server);
		GetRPCManager().AddRPC("eAI", "DayZPlayerInventory_OnEventForRemoteWeaponAICallback", this, SingeplayerExecutionType.Server);
    }
	
	// Todo we may want to make a "group manager" class
	autoptr array<autoptr eAIGroup> m_groups = {};
	
	// return the group owned by leader, otherwise create a new one.
	eAIGroup GetGroupByLeader(PlayerBase leader) {
		for (int i = 0; i < m_groups.Count(); i++)
			if (m_groups[i].GetLeader() == leader)
				return m_groups[i];
		
		eAIGroup newGroup = m_groups.Get(m_groups.Insert(new eAIGroup()));
		newGroup.SetLeader(leader);
		return newGroup;
	}
	
	//! @param owner Who is the manager of this AI
	//! @param formOffset Where should this AI follow relative to the formation?
	void SpawnAI_Helper(DayZPlayer owner) {
		PlayerBase pb_Human;
		if (!Class.CastTo(pb_Human, owner)) return;
		
		eAIGroup ownerGrp = GetGroupByLeader(pb_Human);

		eAIBase pb_AI;
		if (!Class.CastTo(pb_AI, GetGame().CreatePlayer(null, SurvivorRandom(), pb_Human.GetPosition(), 0, "NONE"))) return;
		
		pb_AI.SetAI(ownerGrp);
			
		SoldierLoadout.Apply(pb_AI);	//or PoliceLoadout.Apply(pb_AI);
	}
	
	// Server Side: This RPC spawns a helper AI next to the player, and tells them to join the player's formation.
	void SpawnEntity(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target) {
		Param1<DayZPlayer> data(null);
        if (!ctx.Read(data)) return;
		
		if (IsMissionOffline()) data.param1 = GetGame().GetPlayer();
		
		if(type == CallType.Server )
		{
            Print("eAI: spawn entity RPC called.");
			SpawnAI_Helper(data.param1);
		}
	}
	
	// Server Side: This RPC spawns a zombie. It's actually not the right way to do it. But it's only for testing.
	// BUG: this has sometimes crashed us before. Not sure why yet.
	void SpawnZombie(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target) {
		Param1<PlayerBase> data;
        if ( !ctx.Read( data ) ) return;
		if(type == CallType.Server) {
            Print("eAI: SpawnZombie RPC called.");
			GetGame().CreateObject("ZmbF_JournalistNormal_Blue", data.param1.GetPosition() + debug_offset_2, false, true, true);
        }
	}
	
	// Server Side: Delete AI.
	void ClearAllAI(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target) {
		Param1<PlayerBase> data;
        if (!ctx.Read(data)) return;
		if(type == CallType.Server ) {
            Print("eAI: ClearAllAI called.");
			foreach (eAIGroup g : m_groups) {
				for (int i = g.Count() - 1; i > -1; i--) {
					PlayerBase p = g.GetMember(i);
					if (p.IsAI()) {
						g.RemoveMember(i);
						GetGame().ObjectDelete(p);
					}
				}	
			}
		}
	}
	
	// Server Side: Delete AI.
	void ProcessReload(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target) {
		Param1<PlayerBase> data;
        if (!ctx.Read(data)) return;
		if(type == CallType.Server ) {
            Print("eAI: ProcessReload called.");
			
			eAIGroup g = GetGroupByLeader(data.param1);

			for (int i = g.Count() - 1; i > -1; i--) {
				PlayerBase p = g.GetMember(i);
				Weapon_Base w = Weapon_Base.Cast(p.GetHumanInventory().GetEntityInHands());
				if (p.IsAI() && w)
					p.QuickReloadWeapon(w);
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
	
	void ReqFormationChange(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target) {
		Param2<DayZPlayer, int> data;
        if (!ctx.Read(data)) return;
		if(type == CallType.Server ) {
			Print("eAI: ReqFormationChange called.");
			eAIGroup g = GetGroupByLeader(data.param1);
			eAIFormation newForm;
			switch (data.param2) {
				case eAICommands.FOR_VEE:
					newForm = new eAIFormationVee();
					break;
				case eAICommands.FOR_FILE:
					newForm = new eAIFormationFile();
					break;
				case eAICommands.FOR_WALL:
					newForm = new eAIFormationWall();
					break;
				case eAICommands.FOR_COL:
					newForm = new eAIFormationColumn();
					break;
				// no default needed here
			}
			g.SetFormation(newForm);
		}
	}
};

modded class MissionServer
{
    autoptr eAIGame m_eaiGame;

    void MissionServer()
    {
        m_eaiGame = new eAIGame();

		GetDayZGame().eAICreateManager();

        Print( "eAI - Loaded Server Mission" );
    }
};

modded class MissionGameplay
{
    autoptr eAIGame m_eaiGame;
	UAInput m_eAIRadialKey;

    void MissionGameplay()
    {
        m_eaiGame = new eAIGame();
		m_eAIRadialKey = GetUApi().GetInputByName("eAICommandMenu");

		GetDayZGame().eAICreateManager();

        Print( "eAI - Loaded Client Mission" );
    }
	
	override void OnUpdate(float timeslice) {
		super.OnUpdate(timeslice);

		// If we want to open the command menu, and nothing else is open
		if (m_eAIRadialKey.LocalPress() && !GetGame().GetUIManager().GetMenu()) {
			if (!eAICommandMenu.instance) new eAICommandMenu();
			GetUIManager().ShowScriptedMenu(eAICommandMenu.instance, null);
		}
		
		// If we want to close the command menu, and our menu is open
		if (m_eAIRadialKey.LocalRelease() && GetGame().GetUIManager().GetMenu() == eAICommandMenu.instance) {
			eAICommandMenu.instance.OnMenuRelease();
			GetUIManager().Back();
		}
	}
};