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

/* TODO: class eAIBase extends PlayerBase // needs config entries */
typedef PlayerBase eAIBase;
modded class PlayerBase
{
	private bool m_eAI_Is = false;

	private autoptr array<ref eAIGoal> m_Goals = new array<ref eAIGoal>;

	private ref eAIHFSM m_FSM;

    // Command handling
	private ref eAIAnimationST m_eAI_AnimationST;
	private eAICommandBase m_eAI_Command;

    // Look/Aiming
	private vector m_TargetLocation;

	private vector m_eAI_LookDirection_WorldSpace;
	private vector m_eAI_AimDirection_WorldSpace;

	private vector m_eAI_LookDirection_ModelSpace;
	private vector m_eAI_AimDirection_ModelSpace;

    private ref eAIGroup m_eAI_Group;
    
    // Path Finding
	private ref PGFilter m_PathFilter;
	private autoptr array<vector> m_Path = new array<vector>();

#ifndef SERVER
	private autoptr array<Shape> m_DebugShapes = new array<Shape>();
#endif

	void PlayerBase()
	{
	}
	
	bool IsAI()
	{
		return m_eAI_Is;
	}

	ref eAIGroup SetAI(eAIGroup group = null)
	{
		m_eAI_Is = true;
        m_eAI_Group = group;
		
        if (m_eAI_Group) {
			m_eAI_Group.AddMember(this);
		} else {
			// We will only be using this case with AI which don't already have a group leader.
			m_eAI_Group = new eAIGroup();
			m_eAI_Group.SetLeader(this);
		}

		m_ActionManager = new ActionManagerAI(this);
		
		m_eAI_AnimationST = new eAIAnimationST(this);

		m_PathFilter = new PGFilter();

		int inFlags = PGPolyFlags.WALK | PGPolyFlags.DOOR | PGPolyFlags.INSIDE | PGPolyFlags.JUMP_OVER;
		int exFlags = PGPolyFlags.DISABLED | PGPolyFlags.SWIM | PGPolyFlags.SWIM_SEA | PGPolyFlags.JUMP | PGPolyFlags.CLIMB | PGPolyFlags.CRAWL | PGPolyFlags.CROUCH;

		m_PathFilter.SetFlags( inFlags, exFlags, PGPolyFlags.NONE );
		m_PathFilter.SetCost( PGAreaType.JUMP, 0.0 );
		m_PathFilter.SetCost( PGAreaType.FENCE_WALL, 0.0 );
		m_PathFilter.SetCost( PGAreaType.WATER, 1.0 );
		
		string stateMachine = "eAI/scripts/Targetting_StateMachine.xml";

		Print(stateMachine);
		eAIHFSMType type = eAIHFSM.LoadXML(stateMachine);
		Print(type);
		if (type)
		{
			m_FSM = type.Spawn(this, null);
			Print(m_FSM);
			m_FSM.StartDefault();
		}

		return m_eAI_Group;
	}

	eAIGroup GetGroup()
	{
		return m_eAI_Group;
	}

	void SetTargetLocation(vector location)
	{
		m_TargetLocation = location;
	}
	
#ifndef SERVER	
	void AddShape(Shape shape)
	{
		m_DebugShapes.Insert(shape);
	}
#endif	

	float Distance(int index, vector position)
	{
		vector begin = m_Path[index];
		vector end = m_Path[index + 1] - begin;
		vector relative = position - begin;
		float eSize2 = end.LengthSq();
		if (eSize2 > 0)
		{
			float time = (end * relative) / eSize2;
			vector nearest = begin + Math.Clamp(time, 0, 1) * end;
			return vector.DistanceSq(nearest, position);
		}

		return vector.DistanceSq(begin, position);
	}

	void PathClear()
	{
		m_Path.Clear();
	}
	
	int PathCount()
	{
		return m_Path.Count();
	}
	
	vector PathGet(int index)
	{
		if (index < 0 || index >= m_Path.Count()) return "0 0 0";
		
		return m_Path[index];
	}

	int FindNext(vector position)
	{
		float dist;
		return FindNext(position, dist);
	}
	
	// Checks to see if the path between the start and end is blocked
	bool PathBlocked(vector start, vector end)
	{
		vector hitPos;
		vector hitNormal;
		
		AIWorld world = GetGame().GetWorld().GetAIWorld();
		
		bool blocked = world.RaycastNavMesh(start, end, m_PathFilter, hitPos, hitNormal);

#ifndef SERVER
		int debugColour = 0xFF00FF00;
		if (blocked) debugColour = 0xFFFF0000;
		vector points[2];
		points[0] = start;
		points[1] = end;
		if (blocked) points[1] = hitPos;
		m_DebugShapes.Insert(Shape.CreateLines(debugColour, ShapeFlags.NOZBUFFER, points, 2));
#endif

		return blocked;
	}
	
	// Checks to see if the path between the start and end is blocked
	bool PathBlocked(vector start, vector end, out vector hitPos, out vector hitNormal)
	{
		AIWorld world = GetGame().GetWorld().GetAIWorld();
		
		bool blocked = world.RaycastNavMesh(start, end, m_PathFilter, hitPos, hitNormal);

#ifndef SERVER
		int debugColour = 0xFF00FF00;
		if (blocked) debugColour = 0xFFFF0000;
		vector points[2];
		points[0] = start;
		points[1] = end;
		if (blocked) points[1] = hitPos;
		m_DebugShapes.Insert(Shape.CreateLines(debugColour, ShapeFlags.NOZBUFFER, points, 2));
#endif

		return blocked;
	}

	int FindNext(vector position, out float minDist)
	{
		int index = 0;

		minDist = 1000000000.0;

		float epsilon = 0.1;
		for (int i = 0; i < m_Path.Count() - 1; ++i)
		{
			float dist = Distance(i, position);
			
			if (minDist >= dist - epsilon)
			{
				if (!PathBlocked(position, m_Path[i + 1]))
				{
					minDist = dist;
					index = i;
				}
			}
		}

		return index + 1;
	}

	float AngleBetweenPoints(vector pos1, vector pos2)
	{
		return vector.Direction(pos1, pos2).Normalized().VectorToAngles()[0];
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
		for (int i = m_DebugShapes.Count() - 1; i >= 0; i--)
			m_DebugShapes[i].Destroy();
		m_DebugShapes.Clear();
#endif

		m_Path.Clear();
		if (m_PathFilter)
		{
			AIWorld world = GetGame().GetWorld().GetAIWorld();
			world.FindPath(GetPosition(), m_TargetLocation, m_PathFilter, m_Path);
		}
	

		if (!GetGame().IsServer()) return;

		if (m_WeaponManager) m_WeaponManager.Update(pDt);
		if (m_EmoteManager) m_EmoteManager.Update(pDt);
		if (m_FSM) m_FSM.Update(pDt);
		
		GetPlayerSoundManagerServer().Update();
		GetHumanInventory().Update(pDt);
		UpdateDelete();
		
		m_eAI_Command = eAICommandBase.Cast(GetCommand_Script());

		if (pCurrentCommandFinished)
		{
			if (PhysicsIsFalling(true))
			{
				StartCommand_Fall(0);

				return;
			}

			StartCommand_Script(new eAICommandMove(this, m_eAI_AnimationST));
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
			StartCommand_Script(new eAICommandMove(this, m_eAI_AnimationST));
			return;
		}
		
		if (pCurrentCommandID == DayZPlayerConstants.COMMANDID_SCRIPT)
		{
		}
	}
		
	// We should integrate this into ReloadWeapon
	void ReloadWeaponAI( EntityAI weapon, EntityAI magazine )
	{
		Print(this.ToString() + "(DayZPlayerInstanceType." + GetInstanceType().ToString() + ") is trying to reload " + magazine.ToString() + " into " + weapon.ToString());
		ActionManagerAI mngr_ai;
		CastTo(mngr_ai, GetActionManager());
		
		if (mngr_ai && FirearmActionLoadMultiBulletRadial.Cast(mngr_ai.GetRunningAction()))
		{
			mngr_ai.Interrupt();
		}
		else if (GetHumanInventory().GetEntityInHands() != magazine)
		{
			Weapon_Base wpn;
			Magazine mag;
			Class.CastTo( wpn,  weapon );
			Class.CastTo( mag,  magazine );
			if (GetWeaponManager().CanUnjam(wpn))
			{
				GetWeaponManager().Unjam();
			}
			else if (GetWeaponManager().CanAttachMagazine(wpn, mag))
			{
				GetWeaponManager().AttachMagazine(mag);
			}
			else if (GetWeaponManager().CanSwapMagazine( wpn, mag))
			{
				GetWeaponManager().SwapMagazine( mag);
			}
			else if (GetWeaponManager().CanLoadBullet( wpn, mag))
			{
				GetWeaponManager().LoadMultiBullet(mag);

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
			if (Weapon_Base.CastTo(weap, GetItemInHands()))
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
			return false;
		}
			
		return super.AimingModel(pDt, pModel);
	}
		
	override bool HeadingModel(float pDt, SDayZPlayerHeadingModel pModel)
	{
		if (IsAI())
		{
			return false;
		}
		
		return super.HeadingModel(pDt, pModel);
	}
	
	override void OnCommandDeathStart()
	{
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