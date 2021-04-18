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

/* TODO: class eAIBase extends PlayerBase // needs config entries */
typedef PlayerBase eAIBase;
modded class PlayerBase
{
	static autoptr array<eAIBase> m_eAI = new array<eAIBase>();

	private bool m_eAI_Is = false;

	private autoptr eAIHFSM m_FSM;
    private autoptr eAIGroup m_eAI_Group;
	private autoptr array<string> m_Transitions = {};

	// Targeting data 
	private autoptr array<eAITarget> m_eAI_Targets;
	autoptr array<Object> threats = {}; // temporary
	
	// Aiming and aim arbitration
	bool m_AimArbitration = false;
	private Man m_CurrentArbiter = null;

    // Command handling
	private autoptr eAIAnimationST m_eAI_AnimationST;
	private eAICommandBase m_eAI_Command;

	private bool m_eAI_UnconsciousVehicle;

	// Position for aiming/looking in the world
	private vector m_eAI_LookPosition_WorldSpace;
	private vector m_eAI_AimPosition_WorldSpace;

	// A direction vector (not YPR) in Model Space, not World Space
	private vector m_eAI_LookDirection_ModelSpace;
	private bool m_eAI_LookDirection_Recalculate;
	private vector m_eAI_AimDirection_ModelSpace;
	private bool m_eAI_AimDirection_Recalculate;
	
	private bool m_WeaponRaised;
	
	private float m_AimDeltaY, m_AimDeltaX; // in deg
	
	bool ReloadingInADS = false; // specifically true if we are currently reloading while in combat mode
    
    // Path Finding
	private autoptr PGFilter m_PathFilter;
	private autoptr array<vector> m_Path = new array<vector>();
	private vector m_TargetPosition;
	private eAITargetOverriding m_eAI_TargetOverriding = eAITargetOverriding.NONE;

#ifndef SERVER
	private autoptr array<Shape> m_DebugShapes = new array<Shape>();
#endif

	void PlayerBase() /*eAIBase*/
	{
	}

	void ~PlayerBase() /*~eAIBase*/
	{
		if (IsAI())
		{
			m_eAI_Group.RemoveMember(m_eAI_Group.GetIndex(this));

			m_eAI.RemoveItem(this);
		}
	}
	
	// Used for deciding the best aim arbiter for the AI.
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
	
	// returns true if able
	bool StartAimArbitration() {
		Weapon_Base weap = Weapon_Base.Cast(GetHumanInventory().GetEntityInHands());
		if (/*m_AimArbitration || */!weap || (!m_CurrentArbiter && !eAIGlobal_HeadlessClient)) {
			m_AimArbitration = false;
			return false;
		}
		m_AimArbitration = true;
		if (eAIGlobal_HeadlessClient) {
			Print("Starting aim arbitration for " + this + " with HC");
			GetRPCManager().SendRPC("eAI", "eAIAimArbiterStart", new Param1<Weapon_Base>(weap), false, eAIGlobal_HeadlessClient);
			return true;
		}
		Print("Starting aim arbitration for " + this + " with client " + m_CurrentArbiter.GetIdentity());
		GetRPCManager().SendRPC("eAI", "eAIAimArbiterStart", new Param2<Weapon_Base, int>(weap, 100), false, m_CurrentArbiter.GetIdentity());
		return true;
	}
	
	// returns true if able to stop cleanly
	bool StopAimArbitration() {
		Weapon_Base weap = Weapon_Base.Cast(GetHumanInventory().GetEntityInHands());
		if (/*m_AimArbitration || */!weap || (!m_CurrentArbiter && !eAIGlobal_HeadlessClient)) {
			m_AimArbitration = false;
			return false;
		}
		m_AimArbitration = false;
		if (eAIGlobal_HeadlessClient) {
			Print("Stopping aim arbitration for " + this + " with HC");
			GetRPCManager().SendRPC("eAI", "eAIAimArbiterStop", new Param1<Weapon_Base>(weap), false, eAIGlobal_HeadlessClient);
			return true;
		}
		Print("Stopping aim arbitration for " + this + " with client " + m_CurrentArbiter.GetIdentity());
		GetRPCManager().SendRPC("eAI", "eAIAimArbiterStop", new Param1<Weapon_Base>(weap), false, m_CurrentArbiter.GetIdentity());
		return true;
	}
	
	// returns true if we were able to get a new arbiter
	bool RefreshAimArbitration() {
		Weapon_Base weap = Weapon_Base.Cast(GetHumanInventory().GetEntityInHands());
		if (!weap) {
			// todo how to clean this up?
			//if (m_CurrentArbiter)
			//	GetRPCManager().SendRPC("eAI", "eAIAimArbiterStop", new Param1<Weapon_Base>(weap));
			m_AimArbitration = false;
			return false;
		}
		
		if (eAIGlobal_HeadlessClient) {
			GetRPCManager().SendRPC("eAI", "eAIAimArbiterSetup", new Param1<Weapon_Base>(weap), false, eAIGlobal_HeadlessClient);
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.StartAimArbitration, 50, false);
		}
		
		Man nearest = GetNearestPlayer();
		if (!nearest) return false;
		
		Print("Refreshing aim arbitration for " + this + " current: " + m_CurrentArbiter + " closest: " + nearest);
		if (!m_CurrentArbiter || !m_CurrentArbiter.GetIdentity() || !m_CurrentArbiter.IsAlive()) {
			m_CurrentArbiter = nearest;
			GetRPCManager().SendRPC("eAI", "eAIAimArbiterSetup", new Param1<Weapon_Base>(weap), false, m_CurrentArbiter.GetIdentity());
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.StartAimArbitration, 50, false);
			return true;
		}
		
		if (m_CurrentArbiter != nearest) {
			GetRPCManager().SendRPC("eAI", "eAIAimArbiterStop", new Param1<Weapon_Base>(weap), false, m_CurrentArbiter.GetIdentity());
			m_CurrentArbiter = nearest;
			GetRPCManager().SendRPC("eAI", "eAIAimArbiterSetup", new Param1<Weapon_Base>(weap), false, m_CurrentArbiter.GetIdentity());
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.StartAimArbitration, 50, false);
			return true;
		}
		
		// In this case, we already have an appropriate arbiter set up, just need to restart it
		if (m_CurrentArbiter == nearest && !m_AimArbitration) {
			StartAimArbitration();
			return true;
		}
		
		Print("Aim arbitration was refreshed, but is already running with the best client.");
		
		return false;
	}
	
	bool PlayerIsEnemy(PlayerBase other) {
		if (other.GetGroup() && GetGroup()) {
			if (other.GetGroup() == GetGroup())
				return false;
			if (other.GetGroup().GetFaction().isFriendly(GetGroup().GetFaction()))
				return false;
			
			// at this point we know both we and they have groups, and the groups aren't friendly towards each other
			return true;
		}
		return false;
	}
	
		// Update the aim during combat, return true if we are within parameters to fire.
	int m_AllowedFireTime = 0;
	bool ShouldFire() {
		Weapon_Base weap = Weapon_Base.Cast(GetHumanInventory().GetEntityInHands());
		
		if (!weap) return false;
		
		if (GetGame().GetTime() < m_AllowedFireTime) return false;
		
		// This check is to see if a friendly happens to be in the line of fire
		vector hitPos;
		PlayerBase hitPlayer = PlayerBase.Cast(weap.HitCast(hitPos));
		if (hitPlayer && !PlayerIsEnemy(hitPlayer)) {
			return false;
		}
		
		// for now we just check the raw aim errors
		if (m_AimDeltaX < 1.0 && m_AimDeltaY < 1.0) {
			DelayFiring(500, 300);
			return true;
		}
		
		return false;
	
	}
	
	// Keep the unit from firing until time_ms from now.
	void DelayFiring(int time_ms, int randomAdditionalTime) {
		m_AllowedFireTime = GetGame().GetTime() + time_ms;
		if (randomAdditionalTime > 0) m_AllowedFireTime += Math.RandomInt(0, randomAdditionalTime);
	}
	
	void FireHeldWeapon() {
		Weapon_Base weap = Weapon_Base.Cast(GetHumanInventory().GetEntityInHands());
		if (weap && weap.CanFire()) {
			GetWeaponManager().Fire(weap);
			GetInputController().OverrideAimChangeY(true, 0.0);
		}
	}
	
	override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
		
		Print("eAI: Damage registered from " + source + " type: " + damageType + " component: " + component + " datageResult: " + damageResult + " dmgZone: " + dmgZone + " modelPos: " + modelPos + " speedCoef: " + speedCoef);
		
		Print("eAI: Damage registered from " + source);
		Weapon_Base player_weapon = Weapon_Base.Cast(source);
		if (player_weapon) {
			array<Object> objects = new array<Object>();
			autoptr array<CargoBase> proxyCargos = new array<CargoBase>();
			Object closest = null;
			float dist = 1000000.0;
			float testDist;
		
			GetGame().GetObjectsAtPosition3D(player_weapon.GetPosition(), 1.5, objects, proxyCargos);
		
			for (int j = 0; j < objects.Count(); j++) {
				if (PlayerBase.Cast(objects[j])) {
					testDist = vector.DistanceSq(objects[j].GetPosition(), player_weapon.GetPosition());
					if (testDist < dist) {
						closest = objects[j];
						dist = testDist;
					}
				}
			}
			
			// In theory this line could be removed, and the CanFIre check altered, to turn against group members that shoot them
			if (closest && PlayerIsEnemy(PlayerBase.Cast(closest)))
				AddToThreatList(closest, true);
		}
	}
	
	// Adds a threat to the threat list, if it isn't already there.
	bool AddToThreatList(EntityAI threat, bool prioritize = false) {
		if (!threat || threats.Find(threat) > -1)
			return false;
		if (prioritize)
			threats.InsertAt(threat, 0);
		else threats.Insert(threat);
		return true;
	}
	
	void ScanForDistantPlayers(out array<Object> seenPlayers) {
		autoptr array<Man> players = {};
		GetGame().GetPlayers(players);
		vector LookDir = MiscGameplayFunctions.GetHeadingVector(this);
		foreach (Man p : players) {
			float distSq = vector.DistanceSq(GetPosition(), p.GetPosition());
			// This checks that the distance is less than 500, and that the AI is at least facing in the half plane of the right way.
			// If we wanted to, we could normalize the direction vector, then do something like vector.Dot(v1, v2) > 0.5
			if (distSq < (500*500) && vector.Dot(LookDir, p.GetPosition() - GetPosition()) > 0) {
				if (!IsViewOccluded(p.GetPosition() + "0 1.5 0"))
					seenPlayers.Insert(p);
			}
		}
	}
	
	// Cleans out any invalid or dead targets
	// Returns the number of threats in the array
	ref array<CargoBase> proxyCargos = {};
	int CleanThreats() {
		vector center = GetPosition();
		
		// Leave threats in that don't need cleaning
		for (int j = 0; j < threats.Count(); j++)
			if (!threats[j] || !threats[j].IsAlive())
				threats.Remove(j);
		
		autoptr array<Object> newThreats = new array<Object>();
		GetGame().GetObjectsAtPosition(center, 30.0, newThreats, proxyCargos);
		
		// Add more distant things in the dir we're looking
		ScanForDistantPlayers(newThreats);
		
		// Todo find a faster way to do this... like a linked list?
		int i = 0;
		float minDistance = 1000000.0, temp;
		while (i < newThreats.Count()) {
			DayZInfected infected = DayZInfected.Cast(newThreats[i]);
			PlayerBase player = PlayerBase.Cast(newThreats[i]);
			if (infected && infected.IsAlive() && !IsViewOccluded(infected.GetPosition() + "0 1.5 0")) {
				// It's an infected, add it to teh threates array
				temp = vector.Distance(newThreats[i].GetPosition(), GetPosition());
				if (temp < minDistance) {
					AddToThreatList(infected, true);
				} else AddToThreatList(infected);
				
			} else if (player && PlayerIsEnemy(player) && player.IsAlive() && !IsViewOccluded(player.GetPosition() + "0 1.5 0")) {
				// If it's an enemy player
				temp = vector.Distance(newThreats[i].GetPosition(), GetPosition());
				if (temp < minDistance) {
					AddToThreatList(player, true);
				} else AddToThreatList(player);
			}
			i++;
		}
		return threats.Count();
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

		m_AimingModel = new eAIImplementAiming(this);
		m_ActionManager = new eAIActionManager(this);
		m_WeaponManager = new eAIWeaponManager(this);
		
		m_eAI_AnimationST = new eAIAnimationST(this);

		m_PathFilter = new PGFilter();

		int inFlags = PGPolyFlags.WALK | PGPolyFlags.DOOR | PGPolyFlags.INSIDE | PGPolyFlags.JUMP_OVER;
		int exFlags = PGPolyFlags.DISABLED | PGPolyFlags.SWIM | PGPolyFlags.SWIM_SEA | PGPolyFlags.SPECIAL | PGPolyFlags.JUMP | PGPolyFlags.CLIMB | PGPolyFlags.CRAWL | PGPolyFlags.CROUCH;

		m_PathFilter.SetFlags( inFlags, exFlags, PGPolyFlags.NONE );
		m_PathFilter.SetCost( PGAreaType.JUMP, 0.0 );
		m_PathFilter.SetCost( PGAreaType.FENCE_WALL, 0.0 );
		m_PathFilter.SetCost( PGAreaType.WATER, 1.0 );

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
	
	// This can be used by humans too
	void SetGroup(eAIGroup g) {
		m_eAI_Group = g;
	}

	eAIGroup GetGroup()
	{
		return m_eAI_Group;
	}

	eAIHFSM GetFSM()
	{
		return m_FSM;
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

	vector GetTargetPosition()
	{
		return m_TargetPosition;
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


		AIWorld world = GetGame().GetWorld().GetAIWorld();
		
		m_eAI_TargetOverriding = eAITargetOverriding.NONE;

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
				
				world.FindPath(GetPosition(), m_TargetPosition, m_PathFilter, m_Path);
			}
		}
		

		
		//! handle death with high priority
		if (HandleDeath(pCurrentCommandID))
		{
			return;
		}

		vector transform[4];
		GetTransform(transform);
		
		if (threats.Count() > 0 && threats[0]);
			AimAtPosition(threats[0].GetPosition());

		if (m_eAI_LookDirection_Recalculate) m_eAI_LookDirection_ModelSpace = (m_eAI_LookPosition_WorldSpace - transform[3]).Normalized().Multiply3(transform);
		if (m_eAI_AimDirection_Recalculate) m_eAI_AimDirection_ModelSpace = (m_eAI_AimPosition_WorldSpace - transform[3]).Normalized().InvMultiply3(transform);

		HumanInputController hic = GetInputController();
		EntityAI entityInHands = GetHumanInventory().GetEntityInHands();
		bool isWeapon		= entityInHands	&& entityInHands.IsInherited(Weapon);
		
		// handle weapons
		if(hic)
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
		if (m_FSM) m_FSM.Update(pDt, 0);
		
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
				hcm.SetFighting(threats.Count() > 0 && threats[0]);

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

	int correctionFlag = 0;
	int correctionCounter = 0;
	float lastdX = 0;
	float lastdY = 0;
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
		
		//TODO, properly use m_eAI_LookDirection_ModelSpace

		// Start of ADS code
		if (m_WeaponRaised && threats.Count() > 0 && threats[0] && !ReloadingInADS) {
			AimAtPosition(threats[0].GetPosition());
			
			float targetAngle, targetHeight, gunHeight;
			if (threats.Count() > 0 && threats[0]) {
				gunHeight = 1.5 + GetPosition()[1]; 			// Todo get the actual world gun height.
				targetHeight = 1.0 + threats[0].GetPosition()[1]; 	// Todo get actual threat height, but this should shoot center of mass in most cases
				targetAngle = Math.Atan2(targetHeight - gunHeight, vector.Distance(GetPosition(), threats[0].GetPosition()))*Math.RAD2DEG; // needs to be in deg
			} else targetAngle = 0;
			float X, Y;
			if (weapon && weapon.aim && weapon.aim.GetAge() < 250) {
				if (!weapon.aim.InterpolationStarted) {
					// begin the interpolation
					// todo we could improve the amount of interpolation that is given on a packet reception
					X = weapon.aim.Azmuith;
					Y = weapon.aim.Inclination;
					weapon.aim.InterpolationAzmuith = X;
					weapon.aim.InterpolationInclination = Y;
					weapon.aim.InterpolationStarted = true;
				} else {
					weapon.aim.InterpolationAzmuith += lastdX * pDt * 5.0; // 5.0 is a fudge factor
					weapon.aim.InterpolationInclination += lastdY * pDt * 10.0;
					Math.NormalizeAngle(weapon.aim.InterpolationAzmuith);
					Math.NormalizeAngle(weapon.aim.InterpolationInclination);
					X = weapon.aim.InterpolationAzmuith;
					Y = weapon.aim.InterpolationInclination;
				}
				//Print("data: " + weapon.aim.Inclination.ToString() + " " + weapon.aim.InterpolationInclination.ToString());
			} else if (weapon && weapon.aim) {
				// This is a note for future me. This may cause a server side performance hit from the time
				// a new arbiter is picked to the time it is started and data is received, since this will keep getting called.
				// We'll worry about that later.
				Print("Weapon aim data has gone out of sync for " + this);
				RefreshAimArbitration();
			} else { 
				// Todo: this fails because we can't set the direction of the player in the command script.
				X = Math.NormalizeAngle( GetOrientation()[0] + 9.0 ); // 9.0 is a fudge factor
				Y = hcw.GetBaseAimingAngleUD();
			}

			m_AimDeltaX = Math.DiffAngle(m_eAI_LookDirection_ModelSpace[0], X);
			m_AimDeltaY = (targetAngle-Y);
			//Print("Aim Debugging - X: " + X + " deltaX: " + m_AimDeltaX + " Y: " + Y + " deltaY: " + m_AimDeltaY);
			if (correctionCounter < 1) {
				pInputs.OverrideAimChangeX(true, 0.001*m_AimDeltaY);
				pInputs.OverrideAimChangeY(true, 1.0);
				lastdY = m_AimDeltaY;
			} else { // Aim along the x axis normally
				// could save time on like 2/10 updates by moving calculations in here
				pInputs.OverrideAimChangeX(true, m_AimDeltaX * (1/500.0));
				pInputs.OverrideAimChangeY(false, 0);
				lastdX = m_AimDeltaX;
			}
			correctionCounter++;
			if (correctionCounter > 5) correctionCounter = 0;
		}

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
	
	bool IsWeaponRaised()
	{
		return m_WeaponRaised;
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