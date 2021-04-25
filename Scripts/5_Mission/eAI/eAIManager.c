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

class eAIManager extends eAIManagerImplement
{
	autoptr eAIAimingProfileManager m_AimingManager;
	
	const vector ZOMBIE_OFFSET = "20 0 0";
	
    void eAIManager()
	{
		m_AimingManager = new eAIAimingProfileManager();
		
		GetRPCManager().AddRPC("eAI", "SpawnAI", this, SingeplayerExecutionType.Server);
		GetRPCManager().AddRPC("eAI", "SpawnZombie", this, SingeplayerExecutionType.Server);
		GetRPCManager().AddRPC("eAI", "ClearAllAI", this, SingeplayerExecutionType.Server);

		GetRPCManager().AddRPC("eAI", "ReqFormationChange", this, SingeplayerExecutionType.Server);
		GetRPCManager().AddRPC("eAI", "ReqFormRejoin", this, SingeplayerExecutionType.Server);
		GetRPCManager().AddRPC("eAI", "ReqFormStop", this, SingeplayerExecutionType.Server);
    }
	
	override void OnUpdate(bool doSim, float timeslice)
	{
		super.OnUpdate(doSim, timeslice);
		
		//! only needs to be called on MP client
		if (GetGame().IsClient()) m_AimingManager.Update(timeslice);
	}
	
	//! @param owner Who is the manager of this AI
	//! @param formOffset Where should this AI follow relative to the formation?
	eAIBase SpawnAI_Helper(PlayerBase owner, string loadout = "SoldierLoadout.json")
	{		
		eAIBase ai;
		if (!Class.CastTo(ai, GetGame().CreateObject(GetRandomAI(), owner.GetPosition()))) return null;

		ai.SetGroup(eAIGroup.GetGroupByLeader(owner));

		HumanLoadout.Apply(ai, loadout);

		return ai;
	}
	
	eAIBase SpawnAI_Sentry(vector pos, string loadout = "SoldierLoadout.json")
	{
		eAIBase ai;
		if (!Class.CastTo(ai, GetGame().CreateObject(GetRandomAI(), pos))) return null;

		HumanLoadout.Apply(ai, loadout);

		return ai;
	}
	
	eAIBase SpawnAI_Patrol(vector pos, string loadout = "SoldierLoadout.json")
	{
		eAIBase ai;
		if (!Class.CastTo(ai, GetGame().CreateObject(GetRandomAI(), pos))) return null;

		HumanLoadout.Apply(ai, loadout);
				
		return ai;
	}
	
	// Server Side: This RPC spawns a helper AI next to the player, and tells them to join the player's formation.
	void SpawnAI(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		Param1<DayZPlayer> data;
        if (!ctx.Read(data)) return;
		
		if(type == CallType.Server )
		{
			if (!GetGame().IsMultiplayer()) data.param1 = GetGame().GetPlayer();
			
            Print("eAI: spawn entity RPC called.");
			SpawnAI_Helper(PlayerBase.Cast(data.param1));
		}
	}
	
	// Client Side: Link the given AI
	void HCLinkObject(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		Param1<PlayerBase> data;
        if ( !ctx.Read( data ) ) return;
		if(type == CallType.Server) {
            Print("HC: Linking object " + data.param1);
			eAIObjectManager.Register(data.param1);
        }
	}
	
	// Client Side: Link the given AI
	void HCUnlinkObject(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		Param1<PlayerBase> data;
        if ( !ctx.Read( data ) ) return;
		if(type == CallType.Server) {
            Print("HC: Unlinking object " + data.param1);
			eAIObjectManager.Unregister(data.param1);
        }
	}
	
	// Server Side: This RPC spawns a zombie. It's actually not the right way to do it. But it's only for testing.
	// BUG: this has sometimes crashed us before. Not sure why yet.
	void SpawnZombie(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		Param1<DayZPlayer> data;
        if (!ctx.Read(data)) return;
		
		if(type == CallType.Server) {
			if (!GetGame().IsMultiplayer()) Class.CastTo(data.param1, GetGame().GetPlayer());
			
            Print("eAI: SpawnZombie RPC called.");
			GetGame().CreateObject("ZmbF_JournalistNormal_Blue", data.param1.GetPosition() + ZOMBIE_OFFSET, false, true, true);
        }
	}
	
	// Server Side: Delete AI.
	void ClearAllAI(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		Param1<PlayerBase> data;
        if (!ctx.Read(data)) return;
		if (type == CallType.Server)
		{
			if (!GetGame().IsMultiplayer()) Class.CastTo(data.param1, GetGame().GetPlayer());
			
            Print("eAI: ClearAllAI called.");
			eAIGroup.DeleteAllAI();
		}
	}
	
	void ReqFormRejoin(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		Param1<DayZPlayer> data;
        if (!ctx.Read(data)) return;
		if(type == CallType.Server)
		{
			if (!GetGame().IsMultiplayer()) Class.CastTo(data.param1, GetGame().GetPlayer());
			
			Print("eAI: ReqFormRejoin called.");
			eAIGroup g = eAIGroup.GetGroupByLeader(DayZPlayerImplement.Cast(data.param1), false);
			g.SetFormationState(eAIGroupFormationState.IN);
		}
	}
	
	void ReqFormStop(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		Param1<DayZPlayer> data;
        if (!ctx.Read(data)) return;
		if(type == CallType.Server)
		{
			if (!GetGame().IsMultiplayer()) Class.CastTo(data.param1, GetGame().GetPlayer());
			
			Print("eAI: ReqFormStop called.");
			eAIGroup g = eAIGroup.GetGroupByLeader(DayZPlayerImplement.Cast(data.param1), false);
			g.SetFormationState(eAIGroupFormationState.NONE);
		}
	}
	
	void ReqFormationChange(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		Param2<DayZPlayer, int> data;
        if (!ctx.Read(data)) return;
		if(type == CallType.Server)
		{
			if (!GetGame().IsMultiplayer()) Class.CastTo(data.param1, GetGame().GetPlayer());
			
			Print("eAI: ReqFormationChange called.");
			eAIGroup g = eAIGroup.GetGroupByLeader(DayZPlayerImplement.Cast(data.param1), false);
			eAIFormation newForm;
			switch (data.param2)
			{
				case eAICommands.FOR_VEE:
					newForm = new eAIFormationVee(g);
					break;
				case eAICommands.FOR_FILE:
					newForm = new eAIFormationFile(g);
					break;
				case eAICommands.FOR_WALL:
					newForm = new eAIFormationWall(g);
					break;
				case eAICommands.FOR_COL:
					newForm = new eAIFormationColumn(g);
					break;
				// no default needed here
			}
			g.SetFormation(newForm);
		}
	}
};