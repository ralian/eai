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

modded class PlayerBase
{
	//Note: all of eAIPlayerHandler should be moved into the entity class.
	private eAIPlayerHandler m_eAI_Handler;
	private ref eAIAnimationST m_eAI_AnimationST;
	private eAICommandBase m_eAI_Command;
	
	private bool m_eAI_Is = false;

	void PlayerBase()
	{
		//todo: register net sync bool for 'm_eAI_Is'
	}
	
	// IMPORTANT: Since this class is always constructed in engine, we need some other way to mark a unit as AI outside of the constructor.
	// As such, when spawning AI make sure you ALWAYS CALL MARKAI() ON THE UNIT AFTER TELLING THE ENGINE TO SPAWN THE UNIT.
	// This is used in many functions for workarounds for AI controlled players and it will break things if AI are not properly marked.
	bool IsAI()
	{
		return m_eAI_Is;
	}

	eAIPlayerHandler SetAI()
	{
		m_eAI_Is = true;
		m_ActionManager = new ActionManagerAI(this);
		
		Class.CastTo(m_eAI_Handler, GetDayZGame().eAIManagerGet().AddAI(this));
		m_eAI_AnimationST = new eAIAnimationST(this);

		return m_eAI_Handler;
	}

	eAICommandMove GetCommand_AIMove()
	{
		return eAICommandMove.Cast(GetCommand_Script());
	}

	override void CommandHandler(float pDt, int pCurrentCommandID, bool pCurrentCommandFinished) 
	{
		if (!IsAI())
		{
			super.CommandHandler(pDt, pCurrentCommandID, pCurrentCommandFinished);
			return;
		}

#ifndef SERVER
		m_eAI_Handler.OnDebug();
#endif

		if (!GetGame().IsServer())
			return;

		if (m_WeaponManager) m_WeaponManager.Update(pDt);
		if (m_EmoteManager) m_EmoteManager.Update(pDt);
		
		GetPlayerSoundManagerServer().Update();
		GetHumanInventory().Update(pDt);
		UpdateDelete();
		
		m_eAI_Handler.OnUpdate(pDt);
		
		m_eAI_Command = eAICommandBase.Cast(GetCommand_Script());

		if (pCurrentCommandFinished)
		{
			if (PhysicsIsFalling(true))
			{
				StartCommand_Fall(0);

				return;
			}

			StartCommand_Script(new eAICommandMove(this, m_eAI_Handler, m_eAI_AnimationST));
		}

		// taken from vanilla DayZPlayerImplement
		if (pCurrentCommandID == DayZPlayerConstants.COMMANDID_FALL)
		{
			int landType = 0;
			HumanCommandFall fall = GetCommand_Fall();

			if (fall.PhysicsLanded())
			{
				DayZPlayerType type = GetDayZPlayerType();
				NoiseParams npar;

				// land
				m_FallYDiff = m_FallYDiff - GetPosition()[1];
				//Print(m_FallYDiff);
				if (m_FallYDiff < 0.5)
				{
					landType = HumanCommandFall.LANDTYPE_NONE; 
					fall.Land(landType);
					npar = type.GetNoiseParamsLandLight();
					AddNoise(npar);
				}
				else if (m_FallYDiff < 1.0)
				{
					landType = HumanCommandFall.LANDTYPE_LIGHT;
					fall.Land(landType);
					npar = type.GetNoiseParamsLandLight();
					AddNoise(npar);
				}
				else if (m_FallYDiff < 2.0)
				{
					landType = HumanCommandFall.LANDTYPE_MEDIUM;
					fall.Land(landType);
					npar = type.GetNoiseParamsLandHeavy();
					AddNoise(npar);
				}
				else
				{
					landType = HumanCommandFall.LANDTYPE_HEAVY;
					fall.Land(landType);
					npar = type.GetNoiseParamsLandHeavy();
					AddNoise(npar);
				}
				
				if( m_FallYDiff >= DayZPlayerImplementFallDamage.FD_DMG_FROM_HEIGHT && GetInstanceType() == DayZPlayerInstanceType.INSTANCETYPE_CLIENT )
				{
					this.SpawnDamageDealtEffect();
				}
				
				m_FallDamage.HandleFallDamage(m_FallYDiff);
				m_JumpClimb.CheckAndFinishJump(landType);
			}

			return;
		}
		
		if (PhysicsIsFalling(false))
		{
			StartCommand_Fall(0);
			m_FallYDiff = GetPosition()[1];
			return;
		}

		if (m_ActionManager)
		{
			m_ActionManager.Update(DayZPlayerConstants.COMMANDID_MOVE);
		}
		
		if (pCurrentCommandID == DayZPlayerConstants.COMMANDID_MOVE)
		{
			StartCommand_Script(new eAICommandMove(this, m_eAI_Handler, m_eAI_AnimationST));
			return;
		}
		
		if (pCurrentCommandID == DayZPlayerConstants.COMMANDID_SCRIPT )
		{
		}
	}
		
	// We should integrate this into ReloadWeapon
	void ReloadWeaponAI( EntityAI weapon, EntityAI magazine )
	{
		// The only reason this is an override is because there is a client-only condition here that I have removed.
		// There is probably a better way to do this.
		Print(this.ToString() + "(DayZPlayerInstanceType." + GetInstanceType().ToString() + ") is trying to reload " + magazine.ToString() + " into " + weapon.ToString());
		ActionManagerAI mngr_ai;
		CastTo(mngr_ai, GetActionManager());
		
		if (mngr_ai && FirearmActionLoadMultiBulletRadial.Cast(mngr_ai.GetRunningAction()))
		{
			mngr_ai.Interrupt();
		}
		else if ( GetHumanInventory().GetEntityInHands()!= magazine )
		{
			Weapon_Base wpn;
			Magazine mag;
			Class.CastTo( wpn,  weapon );
			Class.CastTo( mag,  magazine );
			if ( GetWeaponManager().CanUnjam(wpn) )
			{
				GetWeaponManager().Unjam();
			}
			else if ( GetWeaponManager().CanAttachMagazine( wpn, mag ) )
			{
				GetWeaponManager().AttachMagazine(mag);//, new FirearmActionAttachMagazineQuick() );
			}
			else if ( GetWeaponManager().CanSwapMagazine( wpn, mag ) )
			{
				GetWeaponManager().SwapMagazine( mag );
			}
			else if ( GetWeaponManager().CanLoadBullet( wpn, mag ) )
			{
				GetWeaponManager().LoadMultiBullet( mag );

				ActionTarget atrg = new ActionTarget(mag, this, -1, vector.Zero, -1.0);
				if ( mngr_ai && !mngr_ai.GetRunningAction() && mngr_ai.GetAction(FirearmActionLoadMultiBulletRadial).Can(this, atrg, wpn) )
					mngr_ai.PerformActionStart(mngr_ai.GetAction(FirearmActionLoadMultiBulletRadial), atrg, wpn);
			}
		}
	}
	
	override void QuickReloadWeapon(EntityAI weapon)
	{
		if (!IsAI())
		{
			super.QuickReloadWeapon(weapon);
			return;
		}

		ReloadWeaponAI(weapon, GetMagazineToReload(weapon));
	}
	
	// As with many things we do, this is an almagomation of the client and server code
	override void CheckLiftWeapon()
	{
		if (IsAI())
		{
			if (!GetGame().IsServer()) return;

			Weapon_Base weap;
			if ( Weapon_Base.CastTo(weap, GetItemInHands()) )
			{
				m_LiftWeapon_player = weap.LiftWeaponCheck(this);
			}

			return;
		}

		super.CheckLiftWeapon();
	}
	
		
	override bool AimingModel(float pDt, SDayZPlayerAimingModel pModel)
	{
		if (IsAI())
		{
			return m_eAI_Handler.AimingModel(pDt, pModel);
		}
			
		return super.AimingModel(pDt, pModel);
	}
		
	override bool HeadingModel(float pDt, SDayZPlayerHeadingModel pModel)
	{
		if (IsAI())
		{
			return m_eAI_Handler.HeadingModel(pDt, pModel);
		}
		
		return super.HeadingModel(pDt, pModel);
	}
	
	override void OnCommandDeathStart()
	{
		if (IsAI() && GetGame().IsServer())
		{
			m_eAI_Handler.notifyDeath();
		}

		super.OnCommandDeathStart();
	}
	
	// Credit: Wardog
	// Thanks for the amazing help!
	EntityAI GetHands()
    {
        int slot_id = InventorySlots.GetSlotIdFromString("Gloves"); // 98% positive this is correct
        return GetInventory().FindPlaceholderForSlot(slot_id);
    }
	
	// Credit: Wardog
	// dump of all the hand selections
    array<string> GetHandsSelections(string lodName = "1")
    {
        array<string> selection_names = {};
        array<Selection> selections = {};

        LOD lod = GetHands().GetLODByName(lodName); // get LOD 1
        lod.GetSelections(selections);

        foreach (auto sel : selections)
            selection_names.Insert(sel.GetName());
		
		return selection_names;
    }

	// Credit: Wardog
    vector GetHandSelectionPosition(string lodName, string selectionName)
    {
        LOD lod = GetHands().GetLODByName(lodName);
        if (!lod)
        {
            Print("Could not find LOD by name: " + lodName);
            return "0 0 0";
        }
        
        Selection selection = lod.GetSelectionByName(selectionName);
        if (!selection)
        {
            Print("Could not find selection by name: " + selectionName);
            return "0 0 0";
        }

        vector total_vertices = "0 0 0";
        int count = selection.GetVertexCount();

        for (int i = 0; i < count; ++i)
            total_vertices += lod.GetVertexPosition(selection.GetLODVertexIndex(i));
		
		if (count < 1) {
			Print("PlayerBase::GetHandSelectionPosition() - No verts found lod:" + lod + " selectionName:" + selectionName);
			return "0 0 0";
		}

        return Vector(total_vertices[0] / count, total_vertices[1] / count, total_vertices[2] / count); // divide by vertice count to get center
    }
};