// Copyright 2021 William Bowers
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//	http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

class eAIBase extends PlayerBase
{
	private static autoptr array<eAIBase> m_AllAI = new array<eAIBase>();

	private autoptr eAIFSM m_FSM;

	// Targeting data 
	private autoptr array<eAITarget> m_eAI_Targets;
	
	// Aiming and aim arbitration
	bool m_AimArbitration = false;
	private Man m_CurrentArbiter = null;

	// Command handling
	private autoptr eAIAnimationST m_eAI_AnimationST;
	private eAICommandBase m_eAI_Command;

	private bool m_eAI_UnconsciousVehicle;

	private Transport m_eAI_Transport;
	private int m_eAI_Transport_SeatIndex;

	private bool m_eAI_Melee;
	private bool m_eAI_MeleeDidHit;

	private ref eAIAimingProfile m_AimingProfile;

	private ref eAIActionManager m_eActionManager;
	private ref eAIMeleeCombat m_eMeleeCombat;

	// Position for aiming/looking in the world
	private vector m_eAI_LookPosition_WorldSpace;
	private vector m_eAI_AimPosition_WorldSpace;

	// A direction vector (not YPR) in Model Space, not World Space
	private vector m_eAI_LookDirection_ModelSpace;
	private bool m_eAI_LookDirection_Recalculate;
	private vector m_eAI_AimDirection_ModelSpace;
	private bool m_eAI_AimDirection_Recalculate;

	private eAIAimingState m_AimingState;

	private bool m_MovementSpeedActive;
	private int m_MovementSpeed;
	private bool m_MovementDirectionActive;
	private float m_MovementDirection;
	
	private bool m_WeaponRaised;
	private bool m_WeaponRaisedPrev;
	private float m_WeaponRaisedTimer;

	private bool m_AimChangeState;

	ref array<ItemBase> m_Weapons = {};
	ref array<ItemBase> m_MeleeWeapons = {};
	
	// Path Finding
	private ref eAIPathFinding m_PathFinding;

	private Apple m_DebugTargetApple;
	private float m_DebugAngle;

#ifndef SERVER
	private autoptr array<Shape> m_DebugShapes = new array<Shape>();
#endif

	void eAIBase()
	{
		if (IsMissionHost())
		{
			if (eAIGlobal_HeadlessClient && eAIGlobal_HeadlessClient.GetIdentity())
			{
				GetRPCManager().SendRPC("eAI", "HCLinkObject", new Param1<eAIBase>(this), false, eAIGlobal_HeadlessClient.GetIdentity());
			}
		}

		m_AllAI.Insert(this);
	}

	static eAIBase Get(int index)
	{
		return m_AllAI[index];
	}

	override void Init()
	{
		super.Init();

		m_eAI_Targets = new array<eAITarget>();

		m_AimingProfile = new eAIAimingProfile(this);

		m_eMeleeCombat = new eAIMeleeCombat(this);;
		m_MeleeCombat = m_eMeleeCombat;
		m_MeleeFightLogic = new DayZPlayerMeleeFightLogic_LightHeavy(this);

		m_eActionManager = new eAIActionManager(this);;
		m_ActionManager = m_eActionManager;

		m_WeaponManager = new eAIWeaponManager(this);
		m_ShockHandler = new eAIShockHandler(this);
		
		m_eAI_AnimationST = new eAIAnimationST(this);

		m_PathFinding = new eAIPathFinding(this);

		if (IsMissionHost())
		{
			SetGroup(eAIGroup.CreateGroup());

			eAIFSMType type = eAIFSMType.LoadXML("eAI/scripts/FSM", "Master");
			if (type)
			{
				m_FSM = type.Spawn(this, null);
				m_FSM.StartDefault();
			}
		}

		LookAtDirection("0 0 1");
		AimAtDirection("0 0 1");
	}

	void ~eAIBase()
	{
		m_AllAI.RemoveItem(this);

		if (IsAI() && GetGroup())
		{
			GetGroup().RemoveMember(GetGroup().GetIndex(this));
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
		m_AimingState = eAIAimingState.INACTIVE;
	}
	
	void UpdateAimArbitration()
	{
		m_AimingState = eAIAimingState.ACTIVE;
	}
	
	bool PlayerIsEnemy(EntityAI other)
	{
		PlayerBase player = PlayerBase.Cast(other);
		if (!player) return true;
		
		if (player.GetGroup() && GetGroup())
		{
			if (player.GetGroup() == GetGroup())
				return false;

			if (player.GetGroup().GetFaction().IsFriendly(GetGroup().GetFaction()))
				return false;
			
			// at this point we know both we and they have groups, and the groups aren't friendly towards each other
			return true;
		}

		return true;
	}
	
	void TryFireWeapon()
	{
		//eAITrace trace(this, "TryFireWeapon");
		
		Weapon_Base weapon = Weapon_Base.Cast(GetHumanInventory().GetEntityInHands());
		if (!weapon) return;
		
		if (GetDayZPlayerInventory().IsProcessing()) return;
		if (!IsRaised()) return;
		if (!weapon.CanFire()) return;
		if (GetWeaponManager().IsRunning()) return;

		int muzzleIndex = weapon.GetCurrentMuzzle();
		if (!weapon.CanFire(muzzleIndex)) return;
		
		// This check is to see if a friendly happens to be in the line of fire
		vector hitPos;
		int contactComponent;
		EntityAI hitPlayer;
		if (weapon.Hitscan(hitPlayer, hitPos, contactComponent) && !PlayerIsEnemy(hitPlayer)) return;

		GetWeaponManager().Fire(weapon);
	}
	
	override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
		
		if (!IsAI() || !GetGroup()) return;

		ZombieBase zmb;
		if (Class.CastTo(zmb, source))
		{
			if (!zmb.GetTargetInformation().IsTargetted(GetGroup()))
			{
				zmb.GetTargetInformation().AddAI(this);
			}
		}
	}
	
	override bool IsAI()
	{
		return true;
	}

	eAIPathFinding GetPathFinding()
	{
		return m_PathFinding;
	}

	eAIFSM GetFSM()
	{
		return m_FSM;
	}

	ItemBase GetWeaponToUse(bool hasAmmo = false)
	{
		//eAITrace trace(this, "GetWeaponToUse", hasAmmo.ToString());

		// very messy :)
		for (int i = 0; i < m_Weapons.Count(); i++)
		{
			if (m_Weapons[i])
			{
				if (GetHealth() > 0)
				{
					if (!hasAmmo || GetMagazineToReload(m_Weapons[i]))
					{
						return m_Weapons[i];
					}
				}
			}
		}

		return null;
	}

	ItemBase GetMeleeWeaponToUse()
	{
		//eAITrace trace(this, "GetMeleeWeaponToUse");

		// very messy :)
		for (int i = 0; i < m_MeleeWeapons.Count(); i++)
		{
			if (m_MeleeWeapons[i])
			{
				if (GetHealth() > 0)
				{
					return m_MeleeWeapons[i];
				}
			}
		}

		return null;
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

	float GetThreatToSelf()
	{
		if (m_eAI_Targets.Count() == 0) return 0.0;

		return m_eAI_Targets[0].GetThreat(this);
	}

	void UpdateTargets()
	{
		//eAITrace trace(this, "UpdateTargets");
		
		//TODO: use particle system instead

		array<CargoBase> proxyCargos = new array<CargoBase>();
		array<Object> newThreats = new array<Object>();
		GetGame().GetObjectsAtPosition(GetPosition(), 30.0, newThreats, proxyCargos);

		float group_count = GetGroup().Count();

		for (int i = 0; i < newThreats.Count(); i++)
		{
			PlayerBase playerThreat;
			if (Class.CastTo(playerThreat, newThreats[i])) if (GetGroup() && GetGroup().IsMember(playerThreat)) continue;

			if (newThreats[i].IsInherited(ItemBase)) continue;
			if (newThreats[i].IsInherited(Building)) continue;
			if (newThreats[i].IsInherited(Transport)) continue;

			eAITargetInformation target = eAITargetInformation.GetTargetInformation(newThreats[i]);
			if (!target) continue;

			if (!target.IsActive()) continue;

			float threatLevel = target.GetThreat(this);
			if (threatLevel == 0.0) continue;

			int num_ai_in_group_targetting = 0;
			if (target.IsTargetted(GetGroup(), num_ai_in_group_targetting))
			{
				float frac = (group_count - num_ai_in_group_targetting) / group_count;
				if ((frac * threatLevel) < (1.0 / group_count)) continue;
			}

			target.AddAI(this);

			if (m_eAI_Targets.Count() * 2 > group_count) break;
		}
	}

	void PriotizeTargets()
	{
		//eAITrace trace(this, "PriotizeTargets");
		
		// sorting the targets so the highest the threat is indexed lowest

		for (int i = m_eAI_Targets.Count() - 1; i >= 0; i--) 
		{
			if (m_eAI_Targets[i] == null) m_eAI_Targets.Remove(i);
			//eAILogger.Debug("m_eAI_Targets[" + i + "] entity = " + m_eAI_Targets[i].GetEntity() + " threat = " + m_eAI_Targets[i].GetThreat(this));
		}
		
		for (i = 0; i < m_eAI_Targets.Count() - 1; i++) 
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

	void Notify_Transport(Transport vehicle, int seatIndex)
	{
		//eAITrace trace(this, "Notify_Transport", Object.GetDebugName(vehicle), seatIndex.ToString());
		
		m_eAI_Transport = vehicle;
		m_eAI_Transport_SeatIndex = seatIndex;
	}

	void Notify_Melee()
	{
		//eAITrace trace(this, "Notify_Melee");

		m_eAI_Melee = true;
	}

	void UseTargetting()
	{
		//eAITrace trace(this, "UseTargetting");
		
		m_PathFinding.StopOverride();
	}

	/**
	 * @brief Overrides the desired path with no input
	 */
	void OverridePath()
	{
		//eAITrace trace(this, "OverridePath");

		m_PathFinding.OverridePath();
	}

	/**
	 * @brief Overrides the desired path
	 * 
	 * @param pPath the path for path finding
	 */
	void OverridePath(array<vector> pPath)
	{
		//eAITrace trace(this, "OverridePath", pPath.ToString());

		m_PathFinding.OverridePath(pPath);
	}

	/**
	 * @brief Overrides the desired position to generate a new path
	 * 
	 * @param pPosition the target position for path finding
	 */
	void OverridePosition(vector pPosition)
	{
		//eAITrace trace(this, "OverridePosition", pPosition.ToString());
		
		m_PathFinding.OverridePosition(pPosition);
	}

	/**
	 * @brief Overrides speed target at which the ai wants to move at
	 * 
	 * @param pActive is the override active
	 * @param pPosition 0 = IDLE, 1 = WALK, 2 = RUN, 3 = SPRINT
	 */
	void OverrideMovementSpeed(bool pActive, int pSpeed)
	{
		//eAITrace trace(this, "OverrideMovementSpeed", pActive.ToString(), pSpeed.ToString());
		
		m_MovementSpeedActive = pActive;
		m_MovementSpeed = pSpeed;
	}

	/**
	 * @brief Overrides movement direction (forwards, backwards or strafing)
	 * 
	 * @param pActive is the override active
	 * @param pDirection Relative angle on current model orientation from [-180, 180] where 0 is forward.
	 */
	void OverrideMovementDirection(bool pActive, float pDirection)
	{
		//eAITrace trace(this, "OverrideMovementDirection", pActive.ToString(), pDirection.ToString());
		
		m_MovementDirectionActive = pActive;
		m_MovementDirection = pDirection;
	}

	void CreateDebugApple()
	{
		Class.CastTo(m_DebugTargetApple, GetGame().CreateObject("Apple", GetPosition(), true));
	}

	void DestroyDebugApple()
	{
		if (m_DebugTargetApple) m_DebugTargetApple.Delete();
	}

	override void CommandHandler(float pDt, int pCurrentCommandID, bool pCurrentCommandFinished) 
	{
		//eAITrace trace(this, "CommandHandler", pDt.ToString(), pCurrentCommandID.ToString(), pCurrentCommandFinished.ToString());
		
#ifndef SERVER
		for (int i = m_DebugShapes.Count() - 1; i >= 0; i--)
			m_DebugShapes[i].Destroy();
		m_DebugShapes.Clear();
#endif

		#ifdef CF_DebugUI
		CF_DebugUI_Block dbg;
		#endif
		
		//! handle death with high priority
		if (HandleDeath(pCurrentCommandID))
		{
			return;
		}
		
		if (!GetGame().IsServer()) return;
		
		int simulationPrecision = 0;

		//if (!m_DebugTargetApple)
		{
			#ifdef EAI_DEBUG_PATH
			Class.CastTo(dbg, CF.DebugUI.Get("PATH", this));
			dbg.Clear();
			#endif

			UpdateTargets();
			PriotizeTargets();

			if (m_PathFinding.GetOverride() && m_eAI_Targets.Count() > 0)
			{
				eAITarget target = m_eAI_Targets[0];
				if (target.HasInfo()) m_PathFinding.SetPosition(target.GetPosition(this));
			}
			
			m_PathFinding.OnUpdate(pDt, simulationPrecision);
		}
		/*
		else
		{
			m_Path.Clear();

			Input input = GetGame().GetInput();

			float angleChange = 0;
			if (input.LocalValue("eAITestIncrease", false)) angleChange = 360.0;
			if (input.LocalValue("eAITestDecrease", false)) angleChange = -360.0;

			m_DebugAngle += angleChange * pDt;

			vector debugApplePosition = Vector(m_DebugAngle, 0, 0).AnglesToVector() * 5.0;
			debugApplePosition = GetPosition() + debugApplePosition + "0 1 0";
			m_DebugTargetApple.SetPosition(debugApplePosition);
			
			AimAtPosition(debugApplePosition);
			LookAtPosition(debugApplePosition);
		}
		*/

		vector transform[4];
		GetTransform(transform);

		if (m_eAI_Targets.Count() > 0) AimAtPosition(m_eAI_Targets[0].GetPosition(this) + m_eAI_Targets[0].GetAimOffset(this));

		if (m_eAI_LookDirection_Recalculate) m_eAI_LookDirection_ModelSpace = vector.Direction(GetPosition() + "0 1.5 0", m_eAI_LookPosition_WorldSpace).Normalized().InvMultiply3(transform);
		if (m_eAI_AimDirection_Recalculate) m_eAI_AimDirection_ModelSpace = vector.Direction(GetPosition() + "0 1.5 0", m_eAI_AimPosition_WorldSpace).Normalized().InvMultiply3(transform);

		HumanInputController hic = GetInputController();
		EntityAI entityInHands = GetHumanInventory().GetEntityInHands();
		bool isWeapon = entityInHands && entityInHands.IsInherited(Weapon);
		
		if (isWeapon && hic)
		{
			bool exitIronSights = false;
			HandleWeapons(pDt, entityInHands, hic, exitIronSights);
		}	

		if (m_WeaponManager) m_WeaponManager.Update(pDt);
		if (m_EmoteManager) m_EmoteManager.Update(pDt);
		if (m_StaminaHandler) m_StaminaHandler.Update(pDt, pCurrentCommandID);
		if (m_InjuryHandler) m_InjuryHandler.Update(pDt);
		if (m_ShockHandler) m_ShockHandler.Update(pDt);
		if (m_HCAnimHandler) m_HCAnimHandler.Update(pDt, m_MovementState);
		
		GetPlayerSoundManagerServer().Update();
		GetHumanInventory().Update(pDt);
		UpdateDelete();

		#ifdef EAI_DEBUG_FSM
		Class.CastTo(dbg, CF.DebugUI.Get("FSM", this));
		dbg.Clear();
		if (m_FSM) m_FSM.Debug_Update(dbg, 0, pDt, simulationPrecision);
		#else
		if (m_FSM) m_FSM.Update(pDt, simulationPrecision);
		#endif


		switch (m_AimingState)
		{
			case eAIAimingState.INACTIVE:
				GetAimingProfile().UpdateArbiter(null);
				break;
			case eAIAimingState.ACTIVE:
				if (eAIGlobal_HeadlessClient)
				{
					GetAimingProfile().UpdateArbiter(eAIGlobal_HeadlessClient);
					break;
				}
				
				GetAimingProfile().UpdateArbiter(GetNearestPlayer());
				break;
		}

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
				//eAILogger.Debug(m_FallYDiff);
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

		if (m_eAI_Transport)
		{
			int seat_anim_type = m_eAI_Transport.GetSeatAnimationType(m_eAI_Transport_SeatIndex);
			auto vehCommand = StartCommand_VehicleAI(m_eAI_Transport, m_eAI_Transport_SeatIndex, seat_anim_type);
			vehCommand.SetVehicleType(m_eAI_Transport.GetAnimInstance());
			
			m_eAI_Transport = null;
		}
		
		if (pCurrentCommandID == DayZPlayerConstants.COMMANDID_MELEE2)
		{
			HumanCommandMelee2 hcm2 = GetCommand_Melee2();
			if (hcm2)
			{
				if (hcm2.WasHit())
				{
					m_eMeleeCombat.OnHit();

					m_eAI_MeleeDidHit = true;
				}
			
				if (m_eAI_Melee && m_eAI_MeleeDidHit)
				{
					m_eAI_Melee = false;

					m_eMeleeCombat.Combo(hcm2);
				}
			}
		}
		else if (m_eAI_Melee)
		{
			m_eAI_Melee = false;

			m_eMeleeCombat.Start();
		}

		m_eAI_MeleeDidHit = false;
		
		if (pCurrentCommandID == DayZPlayerConstants.COMMANDID_SCRIPT && m_eAI_Command)
		{
			m_eAI_Command.SetLookDirection(m_eAI_LookDirection_ModelSpace);
			
			eAICommandMove hcm;
			if (Class.CastTo(hcm, m_eAI_Command))
			{
				float speedLimit = 3;

				hcm.SetRaised(m_WeaponRaised);
				if (m_WeaponRaised) speedLimit = Math.Min(speedLimit, 1);
				
				float turnTarget = GetOrientation()[0];
				
				if (m_eAI_Targets.Count() > 0) turnTarget = vector.Direction(GetPosition(), m_eAI_AimPosition_WorldSpace).Normalized().VectorToAngles()[0];

				hcm.SetTurnTarget(turnTarget);

				if (m_StaminaHandler && !CanConsumeStamina(EStaminaConsumers.SPRINT) || !CanSprint())
				{
					speedLimit = Math.Min(speedLimit, 2);
				}
				
				if (!m_MovementDirectionActive) m_MovementDirection = 0;

				hcm.SetSpeedOverrider(m_MovementSpeedActive);
				hcm.SetTargetSpeed(m_MovementSpeed);
				hcm.SetSpeedLimit(speedLimit);
				hcm.SetTargetDirection(m_MovementDirection);

				m_SprintFull = false;
				if (hcm.GetCurrentMovementSpeed() > 2.99)
				{
					m_SprintedTime += pDt;
				}
				else
				{
					m_SprintedTime = 0.0;
				}

				if (m_SprintedTime > 0.5)
					m_SprintFull = true;

				return;
			}
		}
	}
		
	// We should integrate this into ReloadWeapon
	void ReloadWeaponAI( EntityAI weapon, EntityAI magazine )
	{
		//eAILogger.Debug(this.ToString() + "(DayZPlayerInstanceType." + GetInstanceType().ToString() + ") is trying to reload " + magazine.ToString() + " into " + weapon.ToString());
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
		//eAITrace trace(this, "HandleWeapons", pDt.ToString(), Object.GetDebugName(pInHands));
		
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

		if (IsRaised() || m_DebugTargetApple)
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

			vector aimOrientation = m_eAI_AimDirection_ModelSpace.VectorToAngles();

			targetLR = aimOrientation[0];// + (pInputs.GetHeadingAngle() * Math.RAD2DEG);
			targetUD = aimOrientation[1];
			
			float dist = vector.Distance(GetPosition() + "0 1.5 0", m_eAI_AimPosition_WorldSpace);
			dist = Math.Clamp(dist, 1.0, 360.0);
			
			offsetLR = -45.0 / dist;
			offsetUD = 0.0;
		}
		
		if (targetLR > 180.0) targetLR = targetLR - 360.0;
		if (targetUD > 180.0) targetUD = targetUD - 360.0;
		
		targetLR += offsetLR;
		targetUD += offsetUD;

		targetLR = Math.Clamp(targetLR, -90.0, 90.0);
		targetUD = Math.Clamp(targetUD, -90.0, 90.0);

		float deltaLR = (targetLR - currentLR) * pDt * 0.1;
		float deltaUD = (targetUD - currentUD) * pDt * 0.1;

		if (IsRaised())
		{
			if (m_AimChangeState)
			{
				pInputs.OverrideAimChangeX(true, deltaUD);
				pInputs.OverrideAimChangeY(true, 0.01);
			}
			else
			{
				pInputs.OverrideAimChangeX(true, deltaLR);
				pInputs.OverrideAimChangeY(false, 0.0);
			}

			m_AimChangeState = !m_AimChangeState;
		}

		if (m_DebugTargetApple)
		{
			vector debugApplePosition = ModelToWorld(Vector(targetLR, targetUD, 0).AnglesToVector() * 1.0);
			m_DebugTargetApple.SetPosition(debugApplePosition + "0 1 0");
		}
	}
	
	// As with many things we do, this is an almagomation of the client and server code
	override void CheckLiftWeapon()
	{		
		if (!GetGame().IsServer()) return;

		Weapon_Base weap;
		if (Weapon_Base.CastTo(weap, GetItemInHands()))
		{
			m_LiftWeapon_player = weap.LiftWeaponCheck(this);
		}

		return;
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

	ActionBase StartAction(typename actionType, ActionTarget target)
	{
		ActionBase action = m_eActionManager.GetAction(actionType);

		m_eActionManager.PerformActionStart(action, target, GetItemInHands());

		return action;
	}

	ActionBase StartActionObject(typename actionType, Object target)
	{
		return StartAction(actionType, new ActionTarget(target, null, -1, vector.Zero, -1.0));
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
		return false;
	}
		
	override bool HeadingModel(float pDt, SDayZPlayerHeadingModel pModel)
	{
		return false;
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
		if (!GetGame().IsMultiplayer() && GetGame().IsServer())
		{
			m_UnconsciousTime += pDt;

			int shock_simplified = SimplifyShock();
			
			if( m_ShockSimplified != shock_simplified )
			{
				m_ShockSimplified = shock_simplified;
				SetSynchDirty();
			}
			
			//eAILogger.Debug(last_command.ToString());
			//eAILogger.Debug(DayZPlayerConstants.COMMANDID_SWIM.ToString());
			
			if( m_UnconsciousTime > PlayerConstants.UNCONSCIOUS_IN_WATER_TIME_LIMIT_TO_DEATH && last_command == DayZPlayerConstants.COMMANDID_SWIM )
			{
				SetHealth("","",-100);
			}
		}
	}
};