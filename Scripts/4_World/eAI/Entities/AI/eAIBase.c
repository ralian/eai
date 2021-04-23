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

enum eAITargetOverriding
{
	NONE,
	POSITION,
	PATH
};

class eAIBase extends PlayerBase
{
	static autoptr array<eAIBase> m_eAI = new array<eAIBase>();

	private bool m_eAI_Is = false;

	private autoptr eAIHFSM m_FSM;
	private autoptr array<string> m_Transitions = {};

	// Targeting data 
	private autoptr array<eAITarget> m_eAI_Targets;
	
	// Aiming and aim arbitration
	bool m_AimArbitration = false;
	private Man m_CurrentArbiter = null;

    // Command handling
	private autoptr eAIAnimationST m_eAI_AnimationST;
	private eAICommandBase m_eAI_Command;

	private bool m_eAI_UnconsciousVehicle;

	private ref eAIAimingProfile m_AimingProfile;

	// Position for aiming/looking in the world
	private vector m_eAI_LookPosition_WorldSpace;
	private vector m_eAI_AimPosition_WorldSpace;

	// A direction vector (not YPR) in Model Space, not World Space
	private vector m_eAI_LookDirection_ModelSpace;
	private bool m_eAI_LookDirection_Recalculate;
	private vector m_eAI_AimDirection_ModelSpace;
	private bool m_eAI_AimDirection_Recalculate;
	
	private bool m_WeaponRaised;
	private bool m_WeaponRaisedPrev;
	private float m_WeaponRaisedTimer;

	private bool m_AimChangeState;
    
    // Path Finding
	private autoptr PGFilter m_PathFilter;
	private autoptr array<vector> m_Path = new array<vector>();
	private vector m_TargetPosition;
	private eAITargetOverriding m_eAI_TargetOverriding = eAITargetOverriding.NONE;

#ifndef SERVER
	private autoptr array<Shape> m_DebugShapes = new array<Shape>();
#endif

	void eAIBase()
	{
	}

	void ~eAIBase()
	{
		if (IsAI())
		{
			m_eAI_Group.RemoveMember(m_eAI_Group.GetIndex(this));

			m_eAI.RemoveItem(this);
		}
	}
	
	// Used for deciding the best aim arbiter for the AI.
	// TODO: particle system
	Man GetNearestPlayer() {
		autoptr array<Man> players = {};
		GetGame().GetPlayers(players);
		float min = 999999.0;
		float temp;
		Man closest = null;
		foreach (Man p : players) {
			temp = vector.DistanceSq(GetPosition(), p.GetPosition());
			if (temp < min) {
				min = temp;
				closest = p;
			}
		}
		return closest;
	}
	
	void StopAimArbitration()
	{
		GetAimingProfile().UpdateArbiter(null);
	}
	
	void UpdateAimArbitration()
	{
		if (eAIGlobal_HeadlessClient)
		{
			GetAimingProfile().UpdateArbiter(eAIGlobal_HeadlessClient);
			return;
		}
		
		GetAimingProfile().UpdateArbiter(GetNearestPlayer());
	}
	
	bool PlayerIsEnemy(EntityAI other) {
		PlayerBase player = PlayerBase.Cast(other);
		if (!player) return true;
		
		if (player.GetGroup() && GetGroup()) {
			if (player.GetGroup() == GetGroup())
				return false;
			if (player.GetGroup().GetFaction().isFriendly(GetGroup().GetFaction()))
				return false;
			
			// at this point we know both we and they have groups, and the groups aren't friendly towards each other
			return true;
		}
		return false;
	}
	
	// Update the aim during combat, return true if we are within parameters to fire.
	int m_MinTimeTillNextFire = 0; //! temp parameter, should be handled in fsm instead
	bool CanFire()
	{
		if (GetGame().GetTime() < m_MinTimeTillNextFire) return false;

		Weapon_Base weap = Weapon_Base.Cast(GetHumanInventory().GetEntityInHands());
		if (!weap) return false;
		
		if (GetDayZPlayerInventory().IsProcessing()) return false;
		if (!IsRaised()) return false;
		
		// This check is to see if a friendly happens to be in the line of fire
		/*
		vector hitPos;
		int contactComponent;
		EntityAI hitPlayer;
		if (weap.Hitscan(hitPlayer, hitPos, contactComponent) && !PlayerIsEnemy(hitPlayer)) return false;
		*/

		return true;
	}
	
	void FireWeapon()
	{
		Weapon_Base weap = Weapon_Base.Cast(GetHumanInventory().GetEntityInHands());
		if (weap)
		{
			m_MinTimeTillNextFire = GetGame().GetTime() + 500;
			m_MinTimeTillNextFire += Math.RandomInt(0, 300);

			GetWeaponManager().Fire(weap);
		}
	}
	
	override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
		
		if (!IsAI()) return;

        ZombieBase zmb;
        if (Class.CastTo(zmb, source))
		{
			if (!zmb.GetTargetInformation().IsTargetted(m_eAI_Group))
			{
				zmb.GetTargetInformation().AddAI(this);
			}
		}
	}
	
	override bool IsAI()
	{
		return m_eAI_Is;
	}

	ref eAIGroup SetAI(eAIGroup group = null)
	{
		m_eAI_Is = true;
        m_eAI_Group = group;

		m_eAI_Targets = new array<eAITarget>();

		m_AimingProfile = new eAIAimingProfile(this);
		
        if (m_eAI_Group)
		{
			m_eAI_Group.AddMember(this);
		}
		else
		{
			// We will only be using this case with AI which don't already have a group leader.
			m_eAI_Group = new eAIGroup();
			m_eAI_Group.SetLeader(this);
		}

		m_eAI.Insert(this);

		m_ActionManager = new eAIActionManager(this);
		m_WeaponManager = new eAIWeaponManager(this);
		
		m_eAI_AnimationST = new eAIAnimationST(this);

		m_PathFilter = new PGFilter();

		int inFlags = PGPolyFlags.WALK | PGPolyFlags.DOOR | PGPolyFlags.INSIDE | PGPolyFlags.JUMP_OVER;
		int exFlags = PGPolyFlags.DISABLED | PGPolyFlags.SWIM | PGPolyFlags.SWIM_SEA | PGPolyFlags.SPECIAL | PGPolyFlags.JUMP | PGPolyFlags.CLIMB | PGPolyFlags.CRAWL | PGPolyFlags.CROUCH;

		m_PathFilter.SetFlags( inFlags, exFlags, PGPolyFlags.NONE );
		m_PathFilter.SetCost( PGAreaType.WATER, 0.0 );

		eAIHFSMType type = eAIHFSM.LoadXML("eAI/scripts/FSM", "Master");
		if (type)
		{
			m_FSM = type.Spawn(this, null);
			m_FSM.StartDefault();
		}

		LookAtDirection("0 0 1");
		AimAtDirection("0 0 1");

		return m_eAI_Group;
	}

	eAIHFSM GetFSM()
	{
		return m_FSM;
	}

	void CreateAimingProfile()
	{
		if (GetGame().IsServer()) return;

		if (m_AimingProfile) return;

		m_AimingProfile = new eAIAimingProfile(this);
	}

	void DestroyAimingProfile()
	{
		if (GetGame().IsServer()) return;

		if (!m_AimingProfile) return;

		delete m_AimingProfile;
	}

	eAIAimingProfile GetAimingProfile()
	{
		return m_AimingProfile;
	}
	
	// Request that a transition with the given event name be forcibly undergone.
	// Does not preserve order in which transitions were forced.
	void RequestTransition(string s) {
		m_Transitions.Clear();
		m_Transitions.Insert(s);
	}
	
	// Warning: this should only be called when you don't want to clear the prior requests, which is usually only 
	// if there are two possible transitions that do the same effect.
	void RequestAdditionalTransition(string s) {
		m_Transitions.Insert(s);
	}
	
	bool GetRequestedTransition(string s) {
		int i = m_Transitions.Find(s);
		if (i > -1) {
			m_Transitions.Clear();
			return true;
		}
		return false;
	}

	array<eAITarget> GetTargets()
	{
		return m_eAI_Targets;
	}

	int TargetCount()
	{
		return m_eAI_Targets.Count();
	}

	eAITarget GetTarget(int index = 0)
	{
		return m_eAI_Targets[index];
	}

	void OnAddTarget(eAITarget target)
	{
		m_eAI_Targets.Insert(target);
	}

	void OnRemoveTarget(eAITarget target)
	{
		m_eAI_Targets.RemoveItem(target);
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
	
	bool IsViewOccluded(vector end) {
		vector start = GetPosition() + "0 1.5 0";
		vector hitPos, hitNormal;
		return PathBlocked(start, end, hitPos, hitNormal);
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

		float epsilon = -0.5;
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

	vector GetTargetPosition()
	{
		return m_TargetPosition;
	}

	float GetThreatToSelf()
	{
		if (m_eAI_Targets.Count() == 0) return 0.0;

		return m_eAI_Targets[0].GetThreat(this);
	}

	void UpdateTargets()
	{
		//TODO: use particle system instead

		array<CargoBase> proxyCargos = new array<CargoBase>();
		array<Object> newThreats = new array<Object>();
		GetGame().GetObjectsAtPosition(GetPosition(), 30.0, newThreats, proxyCargos);

		for (int i = 0; i < newThreats.Count(); i++)
		{
			PlayerBase playerThreat;
			if (Class.CastTo(playerThreat, newThreats[i]) && m_eAI_Group.IsMember(playerThreat)) continue;

			if (newThreats[i].IsInherited(ItemBase)) continue;
			if (newThreats[i].IsInherited(Building)) continue;
			if (newThreats[i].IsInherited(Transport)) continue;

			eAITargetInformation target = eAITargetInformation.GetTargetInformation(newThreats[i]);
			if (!target) continue;

			if (!target.IsActive()) continue;

			if (target.IsTargetted(m_eAI_Group)) continue;

			target.AddAI(this);
		}
	}

	void PriotizeTargets()
	{
		// sorting the targets so the highest the threat is indexed lowest

		for (int i = 0; i < m_eAI_Targets.Count() - 1; i++) 
		{
			int min_idx = i; 
			for (int j = i + 1; j < m_eAI_Targets.Count(); j++) 
			{
				if (m_eAI_Targets[j] && m_eAI_Targets[min_idx] && m_eAI_Targets[j].GetThreat(this) > m_eAI_Targets[min_idx].GetThreat(this)) 
				{
					min_idx = j;	
				}
			}

			m_eAI_Targets.SwapItems(min_idx, i);
		}

		for (i = m_eAI_Targets.Count() - 1; i >= 0; i--) 
		{
			if (m_eAI_Targets[i] == null) m_eAI_Targets.Remove(i);
			//Print("m_eAI_Targets[" + i + "] entity = " + m_eAI_Targets[i].GetEntity() + " threat = " + m_eAI_Targets[i].GetThreat(this));
		}
	}

	eAICommandMove GetCommand_MoveAI()
	{
		return eAICommandMove.Cast(GetCommand_Script());
	}

	eAICommandMove StartCommand_MoveAI()
	{
		eAICommandMove cmd = new eAICommandMove(this, m_eAI_AnimationST);
		StartCommand_Script(cmd);
		m_eAI_Command = cmd;
		return cmd;
	}

	eAICommandVehicle GetCommand_VehicleAI()
	{
		return eAICommandVehicle.Cast(GetCommand_Script());
	}

	eAICommandVehicle StartCommand_VehicleAI(Transport vehicle, int seatIdx, int seat_anim, bool fromUnconscious = false)
	{
		eAICommandVehicle cmd = new eAICommandVehicle(this, m_eAI_AnimationST, vehicle, seatIdx, seat_anim, fromUnconscious);
		StartCommand_Script(cmd);
		m_eAI_Command = cmd;
		return cmd;
	}

	void UseTargetting()
	{
		m_eAI_TargetOverriding = eAITargetOverriding.NONE;
	}

	void OverridePath()
	{
		m_eAI_TargetOverriding = eAITargetOverriding.PATH;
		m_Path.Clear();
	}

	void OverridePath(array<vector> pPath)
	{
		m_eAI_TargetOverriding = eAITargetOverriding.PATH;
		pPath.Copy(m_Path);
	}

	void OverridePosition(vector pPosition)
	{
		m_eAI_TargetOverriding = eAITargetOverriding.POSITION;
		m_TargetPosition = pPosition;
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
		
		if (!GetGame().IsServer()) return;
		int simulationPrecision = 0;

		UpdateTargets();
		PriotizeTargets();

		AIWorld world = GetGame().GetWorld().GetAIWorld();
		
		if (m_eAI_TargetOverriding != eAITargetOverriding.PATH)
		{
			m_Path.Clear();
			
			if (m_PathFilter)
			{
				if (m_eAI_TargetOverriding != eAITargetOverriding.POSITION && m_eAI_Targets.Count() > 0)
				{
					eAITarget target = m_eAI_Targets[0];
					if (target.HasInfo()) 
						m_TargetPosition = target.GetPosition(this);
				}

				vector modifiedTargetPosition = m_TargetPosition;
				if (vector.DistanceSq(GetPosition(), m_TargetPosition) > 5000.0)
				{
					modifiedTargetPosition = GetPosition() + (vector.Direction(GetPosition(), m_TargetPosition).Normalized() * 70.0);
				}
				
				world.FindPath(GetPosition(), modifiedTargetPosition, m_PathFilter, m_Path);
			}
		}
		
		//! handle death with high priority
		if (HandleDeath(pCurrentCommandID))
		{
			return;
		}

		vector transform[4];
		GetTransform(transform);

		if (m_eAI_Targets.Count() > 0) AimAtPosition(m_eAI_Targets[0].GetPosition(this) + "0 1.5 0");

		if (m_eAI_LookDirection_Recalculate) m_eAI_LookDirection_ModelSpace = vector.Direction(GetPosition() + "0 1.5 0", m_eAI_LookPosition_WorldSpace).Normalized().InvMultiply3(transform);
		if (m_eAI_AimDirection_Recalculate) m_eAI_AimDirection_ModelSpace = vector.Direction(GetPosition() + "0 1.5 0", m_eAI_AimPosition_WorldSpace).Normalized().InvMultiply3(transform);

		HumanInputController hic = GetInputController();
		EntityAI entityInHands = GetHumanInventory().GetEntityInHands();
		bool isWeapon = entityInHands && entityInHands.IsInherited(Weapon);
		
		// handle weapons
		if (hic)
		{
			if (isWeapon && (!hic.IsImmediateAction() || !m_ProcessFirearmMeleeHit || !m_ContinueFirearmMelee))
			{
				m_ProcessFirearmMeleeHit = false;
				bool exitIronSights = false;
				HandleWeapons(pDt, entityInHands, hic, exitIronSights);
			}	
		}

		if (m_WeaponManager) m_WeaponManager.Update(pDt);
		if (m_EmoteManager) m_EmoteManager.Update(pDt);
		if (m_FSM) m_FSM.Update(pDt, simulationPrecision);
		
		GetPlayerSoundManagerServer().Update();
		GetHumanInventory().Update(pDt);
		UpdateDelete();

		if (m_ActionManager)
		{
			m_ActionManager.Update(DayZPlayerConstants.COMMANDID_MOVE);

			if (pCurrentCommandID == DayZPlayerConstants.COMMANDID_UNCONSCIOUS)
			{
				OnUnconsciousUpdate(pDt, m_LastCommandBeforeUnconscious);
				if (!m_IsUnconscious) 
				{
					m_IsUnconscious = true;
					OnUnconsciousStart();
				}

				if (!m_ShouldBeUnconscious)
				{
					HumanCommandUnconscious	hcu = GetCommand_Unconscious();
					if (hcu) hcu.WakeUp();
				}
			}
			else
			{
				if (m_ShouldBeUnconscious)
				{
					m_LastCommandBeforeUnconscious = pCurrentCommandID;
					m_eAI_UnconsciousVehicle = false;

					eAICommandVehicle vehCmd = GetCommand_VehicleAI();
					if (vehCmd)
					{
						m_eAI_UnconsciousVehicle = true;

						// not going to bother supporting knocking players out at this current moment
						m_TransportCache = vehCmd.GetTransport();

						vehCmd.KeepInVehicleSpaceAfterLeave(true);
					}

					StartCommand_Unconscious(0);
				}

				if (m_IsUnconscious)
				{
					m_IsUnconscious = false;
					OnUnconsciousStop(pCurrentCommandID);
				}
			}
		}
		
		OnCommandHandlerTick(pDt, pCurrentCommandID);
		
		m_eAI_Command = eAICommandBase.Cast(GetCommand_Script());
				
		if (pCurrentCommandFinished)
		{
			if (pCurrentCommandID == DayZPlayerConstants.COMMANDID_UNCONSCIOUS)
			{
				if (m_eAI_UnconsciousVehicle && (m_TransportCache != null))
				{
					int crew_index = m_TransportCache.CrewMemberIndex(this);
					int seat = m_TransportCache.GetSeatAnimationType(crew_index);
					StartCommand_VehicleAI(m_TransportCache, crew_index, seat, true);
					m_TransportCache = null;
					return;
				}
			}

			if (PhysicsIsFalling(true))
			{
				StartCommand_Fall(0);

				return;
			}

			StartCommand_MoveAI();
		}

		// taken from vanilla DayZPlayerImplement
		if (pCurrentCommandID == DayZPlayerConstants.COMMANDID_FALL)
		{
			int landType = 0;
			HumanCommandFall fall = GetCommand_Fall();

			if (fall && fall.PhysicsLanded())
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
		
		if (HandleDamageHit(pCurrentCommandID))
		{
			return;
		}
		
		if (pCurrentCommandID == DayZPlayerConstants.COMMANDID_MOVE)
		{
			StartCommand_MoveAI();
			return;
		}
		
		if (pCurrentCommandID == DayZPlayerConstants.COMMANDID_SCRIPT && m_eAI_Command)
		{
			m_eAI_Command.SetLookDirection(m_eAI_LookDirection_ModelSpace);
			
			eAICommandMove hcm;
			if (Class.CastTo(hcm, m_eAI_Command))
			{
				hcm.SetRaised(m_WeaponRaised);

				hcm.SetAimPosition(m_WeaponRaised, m_eAI_AimPosition_WorldSpace);

				return;
			}
		}
	}
		
	// We should integrate this into ReloadWeapon
	void ReloadWeaponAI( EntityAI weapon, EntityAI magazine )
	{
		Print(this.ToString() + "(DayZPlayerInstanceType." + GetInstanceType().ToString() + ") is trying to reload " + magazine.ToString() + " into " + weapon.ToString());
		eAIActionManager mngr_ai;
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
	
	// Returns true only if there is a weapon in hands, and the weapon has no rounds ready, 
	// and the inventory does have magazines to reload to.
	bool ShouldReload(out EntityAI magazine, out EntityAI weapon) {
		Weapon weapon_in_hands;
		if (!Class.CastTo(weapon_in_hands, GetItemInHands())) return false;

		int mi = weapon_in_hands.GetCurrentMuzzle();
		Magazine mag = weapon_in_hands.GetMagazine(mi);
		if (mag && mag.GetAmmoCount() > 0) return false;

		magazine = GetMagazineToReload(weapon_in_hands);
		weapon = weapon_in_hands;
		if (!magazine || !weapon) return false;
		
		return true;
	}
	
	override void HandleWeapons(float pDt, Entity pInHands, HumanInputController pInputs, out bool pExitIronSights)
	{
		if (!IsAI())
		{
			super.HandleWeapons(pDt, pInHands, pInputs, pExitIronSights);
			return;
		}

		HumanCommandWeapons hcw = GetCommandModifier_Weapons();
    	GetDayZPlayerInventory().HandleWeaponEvents(pDt, pExitIronSights);
		
		Weapon_Base weapon;
		Class.CastTo(weapon, pInHands);
		
		float currentLR = hcw.GetBaseAimingAngleLR();
		float currentUD = hcw.GetBaseAimingAngleUD();

		float targetLR = 0.0;
		float targetUD = 0.0;
		
		float offsetLR = 0.0;
		float offsetUD = 0.0;

		if (m_WeaponRaised && m_WeaponRaised != m_WeaponRaisedPrev)
		{
			m_WeaponRaisedTimer = 0.0;
		}

		m_WeaponRaisedPrev = m_WeaponRaised;
		m_WeaponRaisedTimer += pDt;

		if (IsRaised())
		{
			#ifndef SERVER
			vector position;
			vector direction;
			
			GetAimingProfile().Get(position, direction);
			
			vector points[2];
			points[0] = position;
			points[1] = position + (direction * 500.0);
			m_DebugShapes.Insert(Shape.CreateLines(COLOR_BLUE, ShapeFlags.VISIBLE, points, 2));
			#endif

			targetLR = m_eAI_AimDirection_ModelSpace.VectorToAngles()[0];
			targetUD = m_eAI_AimDirection_ModelSpace.VectorToAngles()[1];
			
			float dist = vector.Distance(GetPosition() + "0 1.5 0", m_eAI_AimPosition_WorldSpace);
			if (dist < 1.0) dist = 1.0;
			
			offsetLR = -15.0 / dist;
			offsetUD = 0.0;
		}
		
		targetLR += offsetLR;
		targetUD += offsetUD;
		
		if (targetLR > 180.0) targetLR = targetLR - 360.0;
		if (targetUD > 180.0) targetUD = targetUD - 360.0;
		targetLR = Math.Clamp(targetLR, -90.0, 90.0);
		targetUD = Math.Clamp(targetUD, -90.0, 90.0);

		float deltaLR = (targetLR - currentLR) * pDt * 0.1;
		float deltaUD = (targetUD - currentUD) * pDt * 0.1;

		if (m_AimChangeState)
		{
			pInputs.OverrideAimChangeX(true, deltaUD);
			pInputs.OverrideAimChangeY(true, 0.0);
		}
		else
		{
			pInputs.OverrideAimChangeX(true, deltaLR);
			pInputs.OverrideAimChangeY(false, 0.0);
		}

		m_AimChangeState = !m_AimChangeState;

		return;
		
		CheckLiftWeapon();
		ProcessLiftWeapon();
		
		GetMovementState(m_MovementState);
		
		//Print("IsInIronsights " + IsInIronsights());
		//Print("IsInOptics " + IsInOptics());
		// hold breath
		if (pInputs.IsHoldBreath() && m_MovementState.IsRaised() && (IsInIronsights() || IsInOptics()))
		{
			m_IsTryingHoldBreath = true;
		}
		else
		{
			m_IsTryingHoldBreath = false;
		}
		
		ItemOptics optic = weapon.GetAttachedOptics();
	
		if (pInputs.IsFireModeChange())
		{
			GetWeaponManager().SetNextMuzzleMode();
		}
		if (pInputs.IsZeroingUp())
		{
			if (optic && (optic.IsInOptics() || optic.IsUsingWeaponIronsightsOverride()) )
				optic.StepZeroingUp();
			else
			{
				//weapon.StepZeroingUp(weapon.GetCurrentMuzzle());
				weapon.StepZeroingUpAllMuzzles();
			}
		}
		if (pInputs.IsZeroingDown())
		{
			if (optic && (optic.IsInOptics() || optic.IsUsingWeaponIronsightsOverride()) )
				optic.StepZeroingDown();
			else
			{
				//weapon.StepZeroingDown(weapon.GetCurrentMuzzle());
				weapon.StepZeroingDownAllMuzzles();
			}
		}
		
		if (!m_LiftWeapon_player && (m_CameraIronsight || !weapon.CanEnterIronsights() || m_CameraOptics/*m_ForceHandleOptics*/)) 	// HACK straight to optics, if ironsights not allowed
		{
			if (optic)
				HandleOptic(optic, false, pInputs, pExitIronSights);
		}
	
		if (!m_MovementState.IsRaised())
		{
			m_IsFireWeaponRaised = false;
			if (weapon && weapon.IsInOptics())
				weapon.ExitOptics();
			
			StopADSTimer();
	
			return; // if not raised => return
		}
		else
		{
			m_IsFireWeaponRaised = true;
			if ( !m_WeaponRaiseCompleted && (!m_ADSAutomationTimer || (m_ADSAutomationTimer && !m_ADSAutomationTimer.IsRunning())) )
			{
				if (GetInstanceType() == DayZPlayerInstanceType.INSTANCETYPE_SERVER || !GetGame().IsMultiplayer())
					RunADSTimer();
			}
			if (m_ProcessWeaponRaiseCompleted)
			{
				CompleteWeaponRaise();
				m_ProcessWeaponRaiseCompleted = false;
			}
		}
			
		//! fire
		//if (!m_LiftWeapon_player && weapon && !weapon.IsDamageDestroyed() && weapon.CanProcessWeaponEvents() )
		if (GetWeaponManager().CanFire(weapon))
		{
			bool autofire = weapon.GetCurrentModeAutoFire(weapon.GetCurrentMuzzle()) && weapon.IsChamberEjectable(weapon.GetCurrentMuzzle());
			int burst = weapon.GetCurrentModeBurstSize(weapon.GetCurrentMuzzle());
			if (!autofire)
			{
				if (pInputs.IsUseButtonDown())
				{
					GetWeaponManager().Fire(weapon);
				}
			}
			else
			{
				if (pInputs.IsUseButton())
				{
					GetWeaponManager().Fire(weapon);
				}
			}
		}		
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
	
	// @param true to put weapon up, false to lower
	void RaiseWeapon(bool up)
	{
		m_WeaponRaised = up;
	}
	
	override bool IsRaised()
	{
		return m_WeaponRaised && m_WeaponRaisedTimer > 0.3;
	}
	
	// @param LookWS a position in WorldSpace to look at
	void LookAtPosition(vector pPositionWS)
	{
		m_eAI_LookPosition_WorldSpace = pPositionWS;
		m_eAI_LookDirection_Recalculate = true;
	}
	
	// @param AimWS a position in WorldSpace to Aim at
	void AimAtPosition(vector pPositionWS)
	{
		m_eAI_AimPosition_WorldSpace = pPositionWS;
		m_eAI_AimDirection_Recalculate = true;
	}

	void LookAtDirection(vector pDirectionMS)
	{
		m_eAI_LookDirection_ModelSpace = pDirectionMS;
		m_eAI_LookDirection_Recalculate = false;
	}

	void AimAtDirection(vector pDirectionMS)
	{
		m_eAI_AimDirection_ModelSpace = pDirectionMS;
		m_eAI_AimDirection_Recalculate = false;
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

	void OnCommandVehicleAIStart()
	{
		m_AnimCommandStarting = HumanMoveCommandID.CommandVehicle;
		
		if ( GetInventory() )
			GetInventory().LockInventory(LOCK_FROM_SCRIPT);
		
		ItemBase itemInHand = GetItemInHands();
		EntityAI itemOnHead = FindAttachmentBySlotName("Headgear");

		if ( itemInHand && itemInHand.GetCompEM() )
			itemInHand.GetCompEM().SwitchOff();

		TryHideItemInHands(true);

		if ( itemOnHead && itemOnHead.GetCompEM() )
			itemOnHead.GetCompEM().SwitchOff();
		
		eAICommandVehicle hcv = GetCommand_VehicleAI();
		if ( hcv && hcv.GetVehicleSeat() == DayZPlayerConstants.VEHICLESEAT_DRIVER )
			OnVehicleSeatDriverEnter();
	}
	
	void OnCommandVehicleAIFinish()
	{
		if ( GetInventory() )
			GetInventory().UnlockInventory(LOCK_FROM_SCRIPT);
		
		TryHideItemInHands(false, true);
		
		if ( m_IsVehicleSeatDriver )
			OnVehicleSeatDriverLeft();
	}

	override void OnUnconsciousUpdate(float pDt, int last_command)
	{
		if (IsAI() && !GetGame().IsMultiplayer() && GetGame().IsServer())
		{
			m_UnconsciousTime += pDt;

			int shock_simplified = SimplifyShock();
			
			if( m_ShockSimplified != shock_simplified )
			{
				m_ShockSimplified = shock_simplified;
				SetSynchDirty();
			}
			
			//PrintString(last_command.ToString());
			//PrintString(DayZPlayerConstants.COMMANDID_SWIM.ToString());
			
			if( m_UnconsciousTime > PlayerConstants.UNCONSCIOUS_IN_WATER_TIME_LIMIT_TO_DEATH && last_command == DayZPlayerConstants.COMMANDID_SWIM )
			{
				SetHealth("","",-100);
			}

			return;
		}

		super.OnUnconsciousUpdate(pDt, last_command);
	}
};