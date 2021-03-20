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

enum eAIBehaviorGlobal { // Global states that dictate each m_Unit's actions
	//BRAINDEAD = -1, 	// Do absolutely nothing. For optimization of units outside a certain radius (or unconcious, etc.)
	RELAXED = 0, 	// Idle and watch surroundings loosely. Will leisurely walk to waypoint.
	ALERT, 		// Scan surroundings carefully. Will run to waypoint.
	//PANIC, 		// Unused, temp m_State if shot or injured from unknown direction
	COMBAT 		// Will engage the threats in the threat list in order. Will ignore waypoints to seek out threats.
};

// Combat behavior flags... mostly unused at the moment
enum eAICombatPriority {
	ELIMINATE_TARGET,	// Kill the first target in the target array.
	FIND_COVER,			// Find cover before engaging
	COVERING_FIRE,		// Used to protect squadmates while they move
	IGNORE,				// Ignore the combat and move to waypoint
	RELOADING,
	OPPORTUNITY_SHOT	// The enemy does not see us (such as an infected) and we should only shoot if we have to.
};

enum eAICombatFlags
{
	NONE = 0,
	FIRING = 1,
	RELOADING = 2
};

enum eAIActionInterlock { // To be implemented later, so that only one anim runs at a time
	NONE,
	WEAPON_UPDOWN,
	RELOADING,
	FIRING
};

/*
class eAIGoalList
{
	eAIGoal m_Head;
	eAIGoal m_Tail;

	// Get the next goal
	eAIGoal Get()
	{
		return m_Head;
	}

	// Clears the list
	void Clear()
	{
		while (m_Head != nullptr) delete Pop();
	}

	// Push to the end
	void Push(eAIGoal goal)
	{
		goal.m_List = this;

		if (m_Head == null)
		{
			m_Head = goal;
			m_Tail = goal;
			goal.m_Previous = null;
			goal.m_Next = null;
			return;
		}

		m_Tail.m_Next = goal;
		goal.m_Previous = m_Tail;
		goal.m_Next = null;
		goal = m_Tail;
	}

	// Remove the front
	eAIGoal Pop()
	{
		if (m_Head == null)
		{
			return null;
		}
		
		eAIGoal popped = m_Head;
		m_Head.m_Next.m_Previous = null;
		m_Head = m_Head.m_Next;

		return popped;
	}
};
*/

class eAIGoal
{
	//eAIGoalList m_List;

	ref eAIGoal m_Previous;
	ref eAIGoal m_Next;

	Object TargetObject;
	vector Position_WorldSpace;
	float ThreatCost;

	void ~eAIGoal()
	{
#ifndef SERVER
		DestroyDebug();
#endif
	}

#ifndef SERVER
	Widget LayoutRoot;
	TextWidget TextA;
	TextWidget TextB;
	TextWidget TextC;

	void DestroyDebug()
	{
		if (LayoutRoot) LayoutRoot.Unlink();
	}

	void Debug(eAIPlayerHandler handler, int idx)
	{
		if (!LayoutRoot)
		{
			LayoutRoot = GetGame().GetWorkspace().CreateWidgets("eai/GUI/eai_goal_debug.layout");
			if (!LayoutRoot) return;

			Class.CastTo(TextA, LayoutRoot.FindAnyWidget("TextWidget0"));
			Class.CastTo(TextB, LayoutRoot.FindAnyWidget("TextWidget1"));
			Class.CastTo(TextC, LayoutRoot.FindAnyWidget("TextWidget2"));
		}

		vector screenPos = GetGame().GetScreenPos(Position_WorldSpace);
		LayoutRoot.SetPos(screenPos[0], screenPos[1]);
		LayoutRoot.Show(screenPos[2] > 0); 

		TextA.SetText("Type: " + TargetObject.GetType());
		TextB.SetText("Cost: " + ThreatCost);
		TextC.SetText("Index: " + idx);

		handler.AddShape(Shape.CreateSphere(0xFFFFFFFF, ShapeFlags.NOZBUFFER | ShapeFlags.WIREFRAME, Position_WorldSpace, 0.01));
	}
#endif
};

// This class is assigned to a single PlayerBase to manage its behavior.
class eAIPlayerHandler
{
	PlayerBase m_Unit;
	//private autoptr eAIGoalList m_Goals();
	private autoptr array<ref eAIGoal> m_Goals = new array<ref eAIGoal>;

	private bool m_IsDead;
	private float m_LastUpdateTime;

	private vector m_LookDirection_WorldSpace;
	private vector m_AimDirection_WorldSpace;

	private vector m_LookDirection_ModelSpace;
	private vector m_AimDirection_ModelSpace;

	private DayZPlayer m_Leader = null;
	private vector m_FormationOffset = "0 0 0";
	
	// Brain data
	private int m_State; // (eAIBehaviorGlobal)
	private float m_StateTime;

	private int m_CombatFlags;
	private bool m_UpdateCombat;

	private eAICombatPriority m_CombatState;
	
	// This is the bitmask used for pathfinding (i.e. whether we want a path that goes over a fence
	ref PGFilter m_PathFilter;
	autoptr array<Shape> m_DebugShapeArray = new array<Shape>();
	
	// Formation following logic
	
	// Data for the heading computation
	float targetAngle, heading, delta;
	
	// Logic for waypoints
	autoptr TVectorArray m_Path = new TVectorArray(); // This is the list of waypoints the AI will follow. It is set by AIWorld.
	
	int cur_waypoint_no = -1; // Next point to move to in the waypoints array. -1 for "none"
	float arrival_radius = 2.0; // This is the distance that we consider a waypoint to be 'reached' at.
	float lastDistToWP = -1; // If it is set to -1, the AI will not try to sprint on the first movement update.
	
	void eAIPlayerHandler(notnull PlayerBase p) {
		if (!GetGame().IsServer())
			Error("AI may only be managed server side!");
				
		if (p.IsAI())
			m_Unit = p;
		else
			Error("Attempting to call eAIPlayerHandler() on " + p.ToString() + ", which is not marked as AI.");
		
		m_State = eAIBehaviorGlobal.RELAXED;
		m_CombatState = eAICombatPriority.ELIMINATE_TARGET;
				
		m_PathFilter = new PGFilter();

		int inFlags = PGPolyFlags.WALK | PGPolyFlags.DOOR | PGPolyFlags.INSIDE | PGPolyFlags.JUMP_OVER;
		int exFlags = PGPolyFlags.DISABLED | PGPolyFlags.SWIM | PGPolyFlags.SWIM_SEA | PGPolyFlags.JUMP | PGPolyFlags.CLIMB | PGPolyFlags.CRAWL | PGPolyFlags.CROUCH;

		m_PathFilter.SetFlags( inFlags, exFlags, PGPolyFlags.NONE );
		m_PathFilter.SetCost( PGAreaType.JUMP, 0.0 );
		m_PathFilter.SetCost( PGAreaType.FENCE_WALL, 0.0 );
		m_PathFilter.SetCost( PGAreaType.WATER, 1.0 );
		
		// This is a separate PGFilter for raycasting what we can "See"
		// Todo move this to a global filter?
		int allowFlags2 = 0;
		allowFlags2 |= PGPolyFlags.ALL;
		allowFlags2 |= PGPolyFlags.WALK;
		occlusionPGFilter.SetFlags(allowFlags2, 0, 0);
	}
	
	void ~eAIPlayerHandler()
	{
#ifndef SERVER
		DestroyDebug();
#endif
	}
	
	// Set a player to follow.
	void SetLeader(DayZPlayer p, vector formationPos) {
		m_FormationOffset = formationPos;
		m_Leader = p;
	}
	
	void notifyDeath()
	{
		m_IsDead = true;
	}

	bool isDead()
	{
		return m_IsDead;
	}
	
	//--------------------------------------------------------------------------------------------------------------------------
	// CODE AND HELPER FUNCS FOR PATHING
	//--------------------------------------------------------------------------------------------------------------------------
	
#ifndef SERVER	
	void AddShape(Shape shape)
	{
		m_DebugShapeArray.Insert(shape);
	}
#endif	

	float Distance( int index, vector position )
	{
		vector b = m_Path[index];
		vector e = m_Path[index + 1] - b;
		vector p = position - b;
		float eSize2 = e.LengthSq();
		if ( eSize2 > 0 )
		{
			float t = ( e * p ) / eSize2;
			vector nearest = b + Math.Clamp( t, 0, 1 ) * e;
			return vector.DistanceSq( nearest, position );
		} else
		{
			return vector.DistanceSq( b, position );
		}
	}

	void PathClear()
	{
		m_Path.Clear();
	}
	
	int PathCount()
	{
		return m_Path.Count();
	}
	
	vector PathGet( int index )
	{
		if ( index < 0 || index >= m_Path.Count() )
			return "0 0 0";
		
		return m_Path[ index ];
	}

	int FindNext( vector position )
	{
		float dist;
		return FindNext( position, dist );
	}
	
	// Checks to see if the path between the start and end is blocked
	bool PathBlocked( vector start, vector end )
	{
		vector hitPos;
		vector hitNormal;
		
		AIWorld world = GetGame().GetWorld().GetAIWorld();
		
		bool blocked = world.RaycastNavMesh( start, end, m_PathFilter, hitPos, hitNormal );

#ifndef SERVER
		int debugColour = 0xFF00FF00;
		if (blocked) debugColour = 0xFFFF0000;
		vector points[2];
		points[0] = start;
		points[1] = end;
		if (blocked) points[1] = hitPos;
		m_DebugShapeArray.Insert(Shape.CreateLines(debugColour, ShapeFlags.NOZBUFFER, points, 2));
#endif

		return blocked;
	}
	
	// Checks to see if the path between the start and end is blocked
	bool PathBlocked( vector start, vector end, out vector hitPos, out vector hitNormal )
	{
		AIWorld world = GetGame().GetWorld().GetAIWorld();
		
		bool blocked = world.RaycastNavMesh( start, end, m_PathFilter, hitPos, hitNormal );

#ifndef SERVER
		int debugColour = 0xFF00FF00;
		if (blocked) debugColour = 0xFFFF0000;
		vector points[2];
		points[0] = start;
		points[1] = end;
		if (blocked) points[1] = hitPos;
		m_DebugShapeArray.Insert(Shape.CreateLines(debugColour, ShapeFlags.NOZBUFFER, points, 2));
#endif

		return blocked;
	}

	int FindNext( vector position, out float minDist )
	{
		int index = 0;

		minDist = 1000000000.0;

		float epsilon = 0.1;
		for ( int i = 0; i < m_Path.Count() - 1; ++i )
		{
			float dist = Distance( i, position );
			
			if ( minDist >= dist - epsilon )
			{
				if ( !PathBlocked( position, m_Path[i + 1] ) )
				{
					minDist = dist;
					index = i;
				}
			}
		}

		return index + 1;
	}

	float AngleBetweenPoints( vector pos1, vector pos2 )
	{
		return vector.Direction( pos1, pos2 ).Normalized().VectorToAngles()[0];
	}

	void OnUpdate(float pDt)
	{
		if (m_State != eAIBehaviorGlobal.COMBAT) return;
		
		Command_UpdateCombat(pDt);

		vector transform[4];
		m_Unit.GetTransform(transform);

		m_AimDirection_ModelSpace = m_AimDirection_WorldSpace.InvMultiply3(transform);

		vector aimAngles = m_AimDirection_ModelSpace.VectorToAngles();
		if (aimAngles[0] > 180.0) aimAngles[0] = aimAngles[0] - 360.0;
		if (aimAngles[1] > 180.0) aimAngles[1] = aimAngles[1] - 360.0;

		HumanCommandWeapons hcw = m_Unit.GetCommandModifier_Weapons();

		float dtLR = (aimAngles[0] - hcw.GetBaseAimingAngleLR()) * pDt * 0.1;
		float dtUD = (aimAngles[1] - hcw.GetBaseAimingAngleUD()) * pDt * 0.1;
		
		m_Unit.GetInputController().OverrideAimChangeX(true, dtLR);
		m_Unit.GetInputController().OverrideAimChangeY(true, dtUD);
	}

	void OnTick()
	{
		float tickTime = GetGame().GetTickTime();
		float deltaTime = m_LastUpdateTime - tickTime;
		m_LastUpdateTime = tickTime;
		
		//TODO: check if we have an outside given goal:
		// e.g. such as a player telling an AI to go to position - if so,
		// only check to see if there are nearby zombies that might need
		// dealing with or if picking up a weapon is imperiative

		int i, j;

		m_Goals.Clear();
		
		//TODO: use particle system
		array<Object> threats();
		array<CargoBase> proxyCargos();
		GetGame().GetObjectsAtPosition(m_Unit.GetPosition(), 30.0, threats, proxyCargos);
		for (i = 0; i < threats.Count(); i++)
		{
			float cost = ComputeThreatCost(threats[i]);
			if (cost > 0)
			{
				if (IsViewOccluded(threats[i].GetPosition())) continue;

				eAIGoal goal = new eAIGoal();
				goal.TargetObject = threats[i];
				goal.Position_WorldSpace = threats[i].GetPosition();
				goal.ThreatCost = cost;

				m_Goals.Insert(goal);
			}
		}
		
		goal = new eAIGoal();
		goal.TargetObject = m_Leader;
		goal.Position_WorldSpace = m_Leader.GetPosition();
		goal.ThreatCost = 0;
		m_Goals.Insert(goal);

		for (i = 1; i < m_Goals.Count(); i++)
		{
			goal = m_Goals[i];
			for (j = i - 1; j >= 0; j--)
			{
				if (goal.ThreatCost < m_Goals[j].ThreatCost) break;
				m_Goals[j + 1] = m_Goals[j];
			}
			m_Goals[j + 1] = goal;
		}

		goal = m_Goals[0];

		if (goal.ThreatCost > 50.0) SetState(eAIBehaviorGlobal.COMBAT);

		if (m_StateTime > 5.0) SetState(m_State - 1);
		
		switch (m_State)
		{
			case eAIBehaviorGlobal.RELAXED:
				UpdateRelaxed(deltaTime);
				break;
			case eAIBehaviorGlobal.ALERT:
				UpdateAlert(deltaTime);
				break;
			case eAIBehaviorGlobal.COMBAT:
				UpdateCombat(deltaTime);
				break;
		}
	}

	void UpdateRelaxed(float pDt)
	{
		m_Unit.GetCommand_AIMove().SetSpeedLimit(3);
		m_Unit.GetCommand_AIMove().SetRaised(false);

		m_Path.Clear();

		AIWorld world = GetGame().GetWorld().GetAIWorld();
		
		vector outPos = m_Leader.GetPosition() + m_FormationOffset;
		vector outNormal;
		PathBlocked(m_Leader.GetPosition(), outPos, outPos, outNormal);

		world.FindPath(m_Unit.GetPosition(), outPos, m_PathFilter, m_Path);
	}

	void UpdateAlert(float pDt)
	{
		UpdateRelaxed(pDt);
	}

	void UpdateCombat(float pDt)
	{
		eAIGoal goal = m_Goals[0];

		m_Unit.GetCommand_AIMove().SetSpeedLimit(1);
		m_Unit.GetCommand_AIMove().SetRaised(true);

		if (vector.DistanceSq(goal.Position_WorldSpace, m_Unit.GetPosition()) > 100.0)
		{
			m_CombatFlags = eAICombatFlags.NONE;

			m_Path.Clear();

			AIWorld world = GetGame().GetWorld().GetAIWorld();

			vector outPos = goal.Position_WorldSpace;
			vector outNormal;
			world.SampleNavmeshPosition(outPos, 10.0, m_PathFilter, outPos);

			world.FindPath(m_Unit.GetPosition(), outPos, m_PathFilter, m_Path);

			return;
		}

		m_AimDirection_WorldSpace = vector.Direction(m_Unit.ModelToWorld("0 1 0"), goal.TargetObject.ModelToWorld("0 1 0")).Normalized();
		
		m_UpdateCombat = true;
	}
	
	bool m_Reloading = false;
	bool m_Firing = false;
	
	void Command_UpdateCombat(float pDt)
	{
		if (!m_UpdateCombat) return;
		m_UpdateCombat = false;
		
		EntityAI hands = m_Unit.GetHumanInventory().GetEntityInHands();

		Weapon_Base weapon_base;
		if (Class.CastTo(weapon_base, hands))
		{
			if (m_Unit.GetWeaponManager() && m_Unit.GetWeaponManager().IsRunning()) return;

			if (weapon_base.IsChamberEmpty(weapon_base.GetCurrentMuzzle()))
			{
				m_Unit.QuickReloadWeapon(weapon_base);
				m_Reloading = true;
				return;
			}

			if (weapon_base.CanFire())
			{
				m_Unit.GetWeaponManager().Fire(weapon_base);
				m_Firing = true;
				return;
			}

			return;
		}
	}

	void SetState(int state)
	{
		m_State = state;
		if (m_State < 0) m_State = 0;
		m_StateTime = 0;
	}

#ifndef SERVER
	Widget LayoutRoot;
	TextWidget TextA;
	TextWidget TextB;
	TextWidget TextC;

	void DestroyDebug()
	{
		if (LayoutRoot) LayoutRoot.Unlink();
	}

	void DebugUI()
	{
		if (!LayoutRoot)
		{
			LayoutRoot = GetGame().GetWorkspace().CreateWidgets("eai/GUI/eai_goal_debug.layout");
			if (!LayoutRoot) return;

			Class.CastTo(TextA, LayoutRoot.FindAnyWidget("TextWidget0"));
			Class.CastTo(TextB, LayoutRoot.FindAnyWidget("TextWidget1"));
			Class.CastTo(TextC, LayoutRoot.FindAnyWidget("TextWidget2"));
		}
		
		vector aimAngles = m_AimDirection_ModelSpace.VectorToAngles();
		if (aimAngles[0] > 180.0) aimAngles[0] = aimAngles[0] - 360.0;
		if (aimAngles[1] > 180.0) aimAngles[1] = aimAngles[1] - 360.0;
		
		HumanCommandWeapons hcw = m_Unit.GetCommandModifier_Weapons();

		vector currAim = Vector(hcw.GetBaseAimingAngleLR(), hcw.GetBaseAimingAngleUD(), 0);
		
		TextA.SetText("m_State: " + aimAngles);
		TextB.SetText("m_State: " + currAim);
		TextC.SetText("m_Reloading: " + m_Reloading + " m_Firing: " + m_Firing);
	}
#endif
		
	void OnDebug()
	{
#ifndef SERVER
		for (int i = m_DebugShapeArray.Count() - 1; i >= 0; i--)
			m_DebugShapeArray[i].Destroy();
		m_DebugShapeArray.Clear();

		for (i = 0; i < m_Goals.Count(); i++)
		{
			m_Goals[i].Debug(this, i);
		}
		
		DebugUI();
#endif
	}
	
	ref PGFilter occlusionPGFilter = new PGFilter();
	bool IsViewOccluded(vector pos, float tolerance = 0.05)
	{
		vector hitPos, hitDir;
		return GetGame().GetWorld().GetAIWorld().RaycastNavMesh(m_Unit.GetPosition() + "0 1.5 0", pos, occlusionPGFilter, hitPos, hitDir);
		//return (vector.Distance(hitPos, pos) > tolerance);
	}

	float ComputeThreatCost(Object threat)
	{
		float cost = 0;

		DayZInfected infected;
		if (Class.CastTo(infected, threat))
		{
			cost += 10.0;
			cost += 100.0 / vector.Distance(threat.GetPosition(), m_Unit.GetPosition());
			DayZInfectedInputController infected_IC = infected.GetInputController();
			Object infected_Target = infected_IC.GetTargetEntity();
			if (infected_Target == null) cost -= 10.0;
			else if (infected_Target == m_Unit) cost += 20.0;
			else if (infected_Target == m_Leader) cost += 20.0;
		}

		return cost;
	}

	bool HeadingModel(float pDt, SDayZPlayerHeadingModel pModel)
	{
		return false;
	}

    bool AimingModel(float pDt, SDayZPlayerAimingModel pModel)
    {
        return false;
    }
};