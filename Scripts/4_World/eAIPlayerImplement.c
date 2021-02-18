enum eAIBehaviorGlobal { // Global states that dictate each unit's actions
	BRAINDEAD, // Do absolutely nothing. For optimization of units outside a certain radius (or unconcious, etc.)
	RELAXED, // Idle and watch surroundings loosely. Will leisurely walk to waypoint.
	ALERT, // Scan surroundings carefully. Will run to waypoint.
	PANIC, // Unused, temp state if shot or injured from unknown direction
	COMBAT // Will engage the threats in the threat list in order. Will ignore waypoints to seek out threats.
}

modded class FirearmActionAttachMagazineQuick : FirearmActionBase
{	
	override bool SetupAction(PlayerBase player, ActionTarget target, ItemBase item, out ActionData action_data, Param extra_data = NULL)
	{
		if( super.SetupAction( player, target, item, action_data, extra_data))
		{
			ActionTarget newTarget;
			if(player.isAI()) // Modded this condition to enable work on AI
			{
				// The only reason we need this instead of GetPreparedMags is because the latter is only updated every tick.
				// If an AI tries to reload on the same tick it spawns, the suitable mags will be null.
				newTarget = new ActionTarget(player.GetMagazineToReload(item), null, -1, vector.Zero, -1);
				action_data.m_Target = newTarget;
			}
			return true;
		}
		if( super.SetupAction( player, target, item, action_data, extra_data))
		{
			if( !GetGame().IsMultiplayer() || GetGame().IsClient()) // Modded this condition to enable work on AI
			{
				newTarget = new ActionTarget(player.GetWeaponManager().GetPreparedMagazine(), null, -1, vector.Zero, -1);
				action_data.m_Target = newTarget;
			}
			return true;
		}
		return false;
	}
}

modded class WeaponManager {
	override bool StartAction(int action, Magazine mag, InventoryLocation il, ActionBase control_action = NULL)
	{
		//if it is controled by action inventory reservation and synchronization provide action itself
		if(control_action) {
			m_ControlAction = ActionBase.Cast(control_action);
			Print("WeaponManager::StartAction control action " + m_ControlAction.ToString() + " triggered on " + m_player.ToString());
			m_PendingWeaponAction = action;
			m_InProgress = true;
			m_IsEventSended = false;
			m_PendingTargetMagazine = mag;
			m_PendingInventoryLocation = il;
			StartPendingAction();
			
			return true;
		}
		
		// Note that we added a condition to continue if server exec for an AI unit.
		if (GetGame().IsMultiplayer() && GetGame().IsServer() && !m_player.isAI())
			return false;
		
		if ( !ScriptInputUserData.CanStoreInputUserData() )
			return false;
		if ( !InventoryReservation(mag, il) )
			return false;

		m_PendingWeaponAction = action;
		m_InProgress = true;
		m_IsEventSended = false;
		
		
		if ( !GetGame().IsMultiplayer() ) {
			m_readyToStart = true;
		} else if (m_player.isAI()) {
			m_PendingTargetMagazine = mag;
			m_PendingInventoryLocation = il;
			SynchronizeServer(mag, il);
		} else {
			Synchronize();
		}
		
		return true;
	}
	
	// This is a meshing of the Synchronize() function and OnInputUserDataProcess() functions.
	// It is meant to do the job of the RPC sent by synchronize and the processing done by the latter function
	// since we are already on the server.
	// This function is only used by the WeaponManager of AI units and executes server-side.
	private bool SynchronizeServer(Magazine mag, InventoryLocation il){
		if(GetGame().IsServer() && m_player.isAI()) {
			Print("WeaponManager::SynchronizeServer for AI unit " + m_player.ToString() + " started.");
			
			m_PendingWeaponActionAcknowledgmentID = ++m_LastAcknowledgmentID;
			
			// When I was modifying this function , I realized you cannot re-instantiate a reference parameter even if it 
			// has been passed as null (as is the case for AT_WPN_ATTACH_MAGAZINE). So, I made a new InventoryLocation for this 
			// case only. I think it needs to be in-scope at the end of this function when the junctures are sent?
			InventoryLocation il2;
			
			Weapon_Base wpn;
			int mi;
			
			int slotID;
			bool accepted = false;
          //////////////////////////////////////////////////////////				

			m_InProgress = true;
			m_IsEventSended = false;
			
			Weapon_Base.CastTo( wpn, m_player.GetItemInHands() );
			if ( wpn )
				mi = wpn.GetCurrentMuzzle();
			
			switch (m_PendingWeaponAction)
			{
				case AT_WPN_ATTACH_MAGAZINE: {
					if ( !mag || !wpn )
						break;

					slotID = wpn.GetSlotFromMuzzleIndex(mi);
					il2 = new InventoryLocation;
					il2.SetAttachment(wpn,mag,slotID);
					if( GetGame().AddInventoryJuncture(m_player, mag, il2, false, 10000) )
						accepted = true;
					m_PendingTargetMagazine = mag;
					break;
				}
				case AT_WPN_SWAP_MAGAZINE: {
					if ( !mag || !wpn )
						break;

					if ( !wpn.GetMagazine(mi) )
						break;
					
					if ( GetGame().AddActionJuncture(m_player,mag,10000) )
						accepted = true;
					
					m_PendingTargetMagazine = mag;
					break;
				}
				case AT_WPN_DETACH_MAGAZINE:
				{
					if ( !il.IsValid() )
						break;
					
					if ( !wpn )
						break;
					
					Magazine det_mag = wpn.GetMagazine(mi);
					mag = Magazine.Cast(il.GetItem());
					if ( !det_mag || ( mag != det_mag) )
						break;
					
					if( GetGame().AddInventoryJuncture(m_player, il.GetItem(), il, true, 10000))
						accepted = true;
					
					m_PendingInventoryLocation = il;
					m_PendingTargetMagazine = mag;
					break;
				}
				case AT_WPN_LOAD_BULLET:
				{
					if ( !mag )
						break;
					
					if( GetGame().AddActionJuncture(m_player,mag,10000) )
						accepted = true;
					m_PendingTargetMagazine = mag;
					break;
				}
				case AT_WPN_LOAD_MULTI_BULLETS_START:
				{
					if ( !mag )
						break;
					
					if( GetGame().AddActionJuncture(m_player,mag,10000) )
						accepted = true;
					m_PendingTargetMagazine = mag;
					break;
				}
				case AT_WPN_UNJAM:
				{
					accepted = true;
					//Unjam();
					break;
				}
				case AT_WPN_EJECT_BULLET:
				{
					accepted = true;
					break;
				}
				case AT_WPN_SET_NEXT_MUZZLE_MODE:
				{
					accepted = true;
					break;
				}
				default:
					Error("unknown actionID=" + m_PendingWeaponAction);
					break;
			}
			DayZPlayerSyncJunctures.SendWeaponActionAcknowledgment(m_player, m_PendingWeaponActionAcknowledgmentID, accepted);
			
			return accepted;
		}
		
		Print("WeaponManager::SynchronizeServer failed! IsServer? " + GetGame().IsServer().ToString() + ". IsAI? " + m_player.isAI().ToString());
		
		return false; // This case is only executed if the conditions are not met
	}
	
	
	override void Update( float deltaT )
	{

		if (m_WeaponInHand != m_player.GetItemInHands())
		{
			if( m_WeaponInHand )
			{
				m_SuitableMagazines.Clear();
				OnWeaponActionEnd();
			}
			m_WeaponInHand = Weapon_Base.Cast(m_player.GetItemInHands());
			if ( m_WeaponInHand )
			{
				m_MagazineInHand = null;
				//SET new magazine
				SetSutableMagazines();
				m_WeaponInHand.SetSyncJammingChance(0);
			}
			m_AnimationRefreshCooldown = 0;
		}
		
		if (m_WeaponInHand)
		{
			if(m_AnimationRefreshCooldown)
			{
				m_AnimationRefreshCooldown--;
			
				if( m_AnimationRefreshCooldown == 0)
				{
					RefreshAnimationState();
				}
			}
		
			if (!GetGame().IsMultiplayer())
			{
				m_WeaponInHand.SetSyncJammingChance(m_WeaponInHand.GetChanceToJam());
			}
			else
			{
				if ( m_NewJamChance >= 0)
				{
					m_WeaponInHand.SetSyncJammingChance(m_NewJamChance);
					m_NewJamChance = -1;
					m_WaitToSyncJamChance = false;
				}
				if (GetGame().IsServer() && !m_WaitToSyncJamChance )
				{
					float actual_chance_to_jam;
					actual_chance_to_jam = m_WeaponInHand.GetChanceToJam();
					if ( Math.AbsFloat(m_WeaponInHand.GetSyncChanceToJam() - m_WeaponInHand.GetChanceToJam()) > 0.001 )
					{
						DayZPlayerSyncJunctures.SendWeaponJamChance(m_player, m_WeaponInHand.GetChanceToJam());
						m_WaitToSyncJamChance = true;
					}
				}
			}
			
			if(m_readyToStart)
			{
				Print("WeaponManager: Unit " + m_player.ToString() + " is about to start queued action.");
				StartPendingAction();
				m_readyToStart = false;
				return;
			}
		
			if( !m_InProgress || !m_IsEventSended )
				return;
		
			if(m_canEnd)
			{
			
				if(m_WeaponInHand.IsIdle())
				{
					OnWeaponActionEnd();
				}
				else if(m_justStart)
				{
					m_InIronSight = m_player.IsInIronsights();
					m_InOptic = m_player.IsInOptics();
		
					if(m_InIronSight || m_InOptic)
					{
						m_player.GetInputController().ResetADS();
						m_player.ExitSights();
					}
				
					m_justStart = false;
				}
			
			}
			else
			{
				m_canEnd = true;
				m_justStart = true;
			}
		}
		else
		{
			if ( m_MagazineInHand != m_player.GetItemInHands() )
			{
				m_MagazineInHand = MagazineStorage.Cast(m_player.GetItemInHands());
				if ( m_MagazineInHand )
				{
					SetSutableMagazines();
				}
			}
		
		
		}
	}
	
	override void StartPendingAction() {			
		m_WeaponInHand = Weapon_Base.Cast(m_player.GetItemInHands());
		if(!m_WeaponInHand)
		{
			OnWeaponActionEnd();
			return;
		}
		
		if (m_PendingTargetMagazine)
			Print("WeaponManager::StartPendingAction - Unit: " + m_player.ToString() + " Weapon: " + m_WeaponInHand.ToString() + " Mag: " + m_PendingTargetMagazine.ToString());
		else
			Print("WeaponManager::StartPendingAction - Unit: " + m_player.ToString() + " Weapon: " + m_WeaponInHand.ToString() + " Mag: None");
		
		switch (m_PendingWeaponAction)
		{
			case AT_WPN_ATTACH_MAGAZINE:
			{
				m_player.GetDayZPlayerInventory().PostWeaponEvent( new WeaponEventAttachMagazine(m_player, m_PendingTargetMagazine) );
				break;
			}
			case AT_WPN_SWAP_MAGAZINE:
			{
				m_player.GetDayZPlayerInventory().PostWeaponEvent( new WeaponEventSwapMagazine(m_player, m_PendingTargetMagazine) );
				break;
			}
			case AT_WPN_DETACH_MAGAZINE:
			{
				Magazine mag = Magazine.Cast(m_PendingInventoryLocation.GetItem());
				m_player.GetDayZPlayerInventory().PostWeaponEvent( new WeaponEventDetachMagazine(m_player, mag, m_PendingInventoryLocation) );
				break;
			}
			case AT_WPN_LOAD_BULLET:
			{
				m_WantContinue = false;
				m_player.GetDayZPlayerInventory().PostWeaponEvent( new WeaponEventLoad1Bullet(m_player, m_PendingTargetMagazine) );
				break;
			}
			case AT_WPN_LOAD_MULTI_BULLETS_START:
			{
				m_player.GetDayZPlayerInventory().PostWeaponEvent( new WeaponEventLoad1Bullet(m_player, m_PendingTargetMagazine) );
				break;
			}
			case AT_WPN_LOAD_MULTI_BULLETS_END:
			{
				m_player.GetDayZPlayerInventory().PostWeaponEvent( new WeaponEventContinuousLoadBulletEnd(m_player) );
				break;
			}
			case AT_WPN_UNJAM:
			{
				m_player.GetDayZPlayerInventory().PostWeaponEvent( new WeaponEventUnjam(m_player, NULL) );
				break;
			}
			case AT_WPN_EJECT_BULLET:
			{
				m_player.GetDayZPlayerInventory().PostWeaponEvent( new WeaponEventMechanism(m_player, NULL) );
				break;
			}
			case AT_WPN_SET_NEXT_MUZZLE_MODE:
			{
				m_player.GetDayZPlayerInventory().PostWeaponEvent( new WeaponEventSetNextMuzzleMode(m_player, NULL) );
				break;
			}
			default:
				m_InProgress = false;
				Error("unknown actionID=" + m_PendingWeaponAction);
		}	
		m_IsEventSended = true;
		m_canEnd = false;
	}
	
	override void OnSyncJuncture(int pJunctureID, ParamsReadContext pCtx)
	{
		Print("WeaponManager::OnSyncJuncture #" + pJunctureID.ToString() + " ("+pCtx+") received for unit: " + m_player.ToString());
		if (pJunctureID == DayZPlayerSyncJunctures.SJ_WEAPON_SET_JAMMING_CHANCE )
		{
			pCtx.Read(m_NewJamChance);
		}
		else
		{
			int AcknowledgmentID;
			pCtx.Read(AcknowledgmentID);
			Print("WeaponManager::OnSyncJuncture ACK ID: " + AcknowledgmentID.ToString());
			if ( AcknowledgmentID == m_PendingWeaponActionAcknowledgmentID)
			{
				if (pJunctureID == DayZPlayerSyncJunctures.SJ_WEAPON_ACTION_ACK_ACCEPT)
				{ 
					Print("WeaponManager::OnSyncJuncture ACK Accept.");
					m_readyToStart = true;
				}
				else if (pJunctureID == DayZPlayerSyncJunctures.SJ_WEAPON_ACTION_ACK_REJECT)
				{
					Print("WeaponManager::OnSyncJuncture ACK Rejected.");
					if(m_PendingWeaponAction >= 0 )
					{
						if(!(GetGame().IsServer() && GetGame().IsMultiplayer()))
						{
							InventoryLocation ilWeapon = new InventoryLocation;
							ItemBase weapon = m_player.GetItemInHands();
							weapon.GetInventory().GetCurrentInventoryLocation(ilWeapon);
							m_player.GetInventory().ClearInventoryReservation(m_player.GetItemInHands(),ilWeapon);
							
							if( m_PendingTargetMagazine )
							{
								m_PendingTargetMagazine.GetInventory().ClearInventoryReservation(m_PendingTargetMagazine, m_TargetInventoryLocation );
							}
							
							if( m_PendingInventoryLocation )
							{
								m_player.GetInventory().ClearInventoryReservation( NULL, m_PendingInventoryLocation );
							}
						}
						m_PendingWeaponActionAcknowledgmentID = -1;
						m_PendingTargetMagazine = NULL;
						m_PendingWeaponAction = -1;
						m_PendingInventoryLocation = NULL;
						m_InProgress = false;
					}
				}
			}
		}
	}
}


modded class PlayerBase {
	ref array<Entity> threats = new array<Entity>();
	
	Entity m_FollowOrders = null; // If this is a non-null reference to an entity, we follow it.
	float m_FollowDistance = 5.0;
	vector m_NextWaypoint = "0 0 0"; // otherwise, default to waypoint.
	
	bool isAI = false;
	
	// IMPORTANT: Since this class is always constructed in engine, we need some other way to mark a unit as AI outside of the constructor.
	// As such, when spawning AI make sure you ALWAYS CALL MARKAI() ON THE UNIT AFTER TELLING THE ENGINE TO SPAWN THE UNIT.
	// This is used in many functions for workarounds for AI controlled players and it will break things if AI are not properly marked.
	bool isAI() {return isAI;}
	void markAI() {
		m_ActionManager = new ActionManagerClient(this);
		isAI = true;
	}
	
	//Reload weapon with given magazine
	override void ReloadWeapon( EntityAI weapon, EntityAI magazine ) {
		// The only reason this is an override is because there is a client-only condition here that I have removed.
		// There is probably a better way to do this.
		Print(this.ToString() + " is trying to reload " + magazine.ToString() + " into " + weapon.ToString());
		ActionManagerClient mngr_client;
		CastTo(mngr_client, GetActionManager());
		
		if (mngr_client && FirearmActionLoadMultiBulletRadial.Cast(mngr_client.GetRunningAction()))
		{
			mngr_client.Interrupt();
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
				GetWeaponManager().AttachMagazine( mag );
			}
			else if ( GetWeaponManager().CanSwapMagazine( wpn, mag ) )
			{
				GetWeaponManager().SwapMagazine( mag );
			}
			else if ( GetWeaponManager().CanLoadBullet( wpn, mag ) )
			{
				//GetWeaponManager().LoadMultiBullet( mag );

				ActionTarget atrg = new ActionTarget(mag, this, -1, vector.Zero, -1.0);
				if ( mngr_client && !mngr_client.GetRunningAction() && mngr_client.GetAction(FirearmActionLoadMultiBulletRadial).Can(this, atrg, wpn) )
					mngr_client.PerformActionStart(mngr_client.GetAction(FirearmActionLoadMultiBulletRadial), atrg, wpn);
			}
		}
	}
	
	void eAIFollow(DayZPlayer p, float distance = 5.0) {
		m_FollowOrders = p;
		m_FollowDistance = distance;
	}
	
	float targetAngle, heading, delta;
	
	void eAIDebugMovement() {
		//Print("targetAngle: " + targetAngle);
		//Print("heading: " + heading);
		//Print("delta: " + delta);
		
		GetWeaponManager().Fire(Weapon_Base.Cast(GetHumanInventory().GetEntityInHands()));
	}
	
	void eAIUpdateMovement() {
		
		if (m_FollowOrders) {m_NextWaypoint = m_FollowOrders.GetPosition();} // move to brain
		
		targetAngle = vector.Direction(GetPosition(), m_NextWaypoint).VectorToAngles().GetRelAngles()[0];// * Math.DEG2RAD;
		heading = -GetInputController().GetHeadingAngle() / Math.DEG2RAD; // This seems to be CCW is positive unlike relative angles.
														// ALSO THIS ISN'T CAPPED AT +-180. I HAVE SEEN IT IN THE THOUSANDS. WHY BOHEMIA?
		//heading = GetDirection()[0]*180 + 90; // The documentation does not cover this but the angle this spits out is between (-1..1). WHAT?! (also seems to be angle of feet)
		//while (heading > 180) {heading -= 360;} // Also, the basis seems to be off by 90 degrees
		//if (heading < 0) {heading = 360+heading;}
		
		delta = targetAngle - heading;
		while (delta > 180) {delta -= 360;} // There's no remainder function so I had to do this retarded BS
		while (delta < -180) {delta += 360;}
		
		GetInputController().OverrideAimChangeX(true, delta/1000.0);
		
		if (vector.Distance(GetPosition(), m_NextWaypoint) >  2 * m_FollowDistance) {
			GetInputController().OverrideMovementSpeed(true, 2.0);
		} else if (vector.Distance(GetPosition(), m_NextWaypoint) > m_FollowDistance) {
			GetInputController().OverrideMovementSpeed(true, 1.0);
		} else {
			GetInputController().OverrideMovementSpeed(true, 0.0);
		}
		
		GetInputController().OverrideMovementAngle(true, targetAngle*Math.DEG2RAD);//Math.PI/2.0);
		//m_MovementState.m_CommandTypeId = DayZPlayerConstants.COMMANDID_MOVE ;
		//m_MovementState.m_iMovement = 2;
	}
	
	void eAIUpdateBrain() { // This update needs to be done way less frequent than Movement; Default is 1 every 10 update ticks.
		// Todo we need to stagger this so not all AI are updated at once
	}
	
	/*override HumanInputController GetInputController() {
		return m_eAIController;
	}*/
};