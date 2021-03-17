
// This is a global flag which is set to true if any (or multiple) units are killed on a tick
// For performance, the mission only cleans up the AI list if it knows at least one died
bool eAIGlobal_UnitKilled = false;

// Todo move this to a separate child class (like PlayerBaseClient)
modded class PlayerBase {
	
	// this is very bad polymorphic design, don't worry, it will be fixed
	eAIPlayerHandler parent;
	
	// Angle above the horizon we should be aiming, degrees.
	float targetAngle = -5.0;

	bool isAI = false;
	
	// IMPORTANT: Since this class is always constructed in engine, we need some other way to mark a unit as AI outside of the constructor.
	// As such, when spawning AI make sure you ALWAYS CALL MARKAI() ON THE UNIT AFTER TELLING THE ENGINE TO SPAWN THE UNIT.
	// This is used in many functions for workarounds for AI controlled players and it will break things if AI are not properly marked.
	bool isAI() {return isAI;}
	void markAIServer() {
		isAI = true;
		m_ActionManager = new ActionManagerAI(this);
	}
	
	void markOwner(eAIPlayerHandler p) {parent = p;}
	
	// Probably not actually necessary
	void markAIClient() {
		isAI = true;
		//m_ActionManager = new ActionManagerClient(this);
	}
	
	//Eventually we can use this instead of calling the action manager, needs more work though
	override void ReloadWeapon( EntityAI weapon, EntityAI magazine ) {
		// The only reason this is an override is because there is a client-only condition here that I have removed.
		// There is probably a better way to do this.
		Print(this.ToString() + "(DayZPlayerInstanceType." + GetInstanceType().ToString() + ") is trying to reload " + magazine.ToString() + " into " + weapon.ToString());
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
				GetWeaponManager().AttachMagazine(mag);
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
	
	// We should integrate this into ReloadWeapon
	void ReloadWeaponAI( EntityAI weapon, EntityAI magazine ) {
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
	
	override void QuickReloadWeapon( EntityAI weapon )
	{
		EntityAI magazine = GetMagazineToReload( weapon );
		if (isAI())
			ReloadWeaponAI( weapon, magazine );
		else
			ReloadWeapon( weapon, magazine );
	}
	
	// As with many things we do, this is an almagomation of the client and server code
	override void CheckLiftWeapon()
	{
		if (isAI() && GetGame().IsServer()) {	
			bool state = false;
			Weapon_Base weap;
			if ( Weapon_Base.CastTo(weap, GetItemInHands()) )
			{
				state = m_LiftWeapon_player;
				bool limited = weap.LiftWeaponCheck(this);
				if (limited) //(limited && !m_WantWeapRaise)
					state = false;
			}
			
			// Now the server code
			ScriptJunctureData pCtx = new ScriptJunctureData;
			pCtx.Write(state);
			SendSyncJuncture(DayZPlayerSyncJunctures.SJ_WEAPON_LIFT, pCtx);
		}
		else super.CheckLiftWeapon();
	}
	
	
	
	//void eAIUpdateTargeting(float pDt) {
		
	//}
	
	override bool AimingModel(float pDt, SDayZPlayerAimingModel pModel) {
		if (isAI() && GetGame().IsServer() && IsAlive()) {
			if (!GetHumanInventory())
				return false;
			
			// If we are aiming a weapon, we need to do vertical targeting logic.
			if (GetHumanInventory().GetEntityInHands() && parent && parent.WantsWeaponUp()) {			
				float delta = -((GetAimingModel().getAimY()-targetAngle)*Math.DEG2RAD)/8.0;
			
				GetInputController().OverrideAimChangeY(true, delta);
				
				// Ignore the fact that this works. Do not ask why it works. Or how I found out that creating a faux recoil event
				// after updating the input controller updates the aim change. If you ask me about this code or why it is written
				// this way, I will deny its existance.
				//
				// Just kidding :-)   ... or am I?
				GetAimingModel().SetDummyRecoil(Weapon_Base.Cast(GetHumanInventory().GetEntityInHands()));
			} else {
				GetInputController().OverrideAimChangeY(true, 0.0);
			}
		}
		
		// If we are not AI
		return super.AimingModel(pDt, pModel);
	}
	
	Object lookAt = null;
	float headingTarget = 0.0;
	float lastHeadingAngle = 0.0;
	override bool HeadingModel(float pDt, SDayZPlayerHeadingModel pModel)
	{
		if ( isAI() )
		{
				// This should be true anyways, but double check that an AI hasn't been set on a client
			if (!GetGame().IsServer())
				return false;
			
			GetMovementState(m_MovementState);
			
			m_fLastHeadingDiff = 0;
			
			if (pModel.m_fHeadingAngle > Math.PI) pModel.m_fHeadingAngle -= Math.PI2;
			if (pModel.m_fHeadingAngle < -Math.PI) pModel.m_fHeadingAngle += Math.PI2;
			lastHeadingAngle = pModel.m_fHeadingAngle;
			
			if (lookAt) {
				headingTarget = vector.Direction(GetPosition(), lookAt.GetPosition()).VectorToAngles().GetRelAngles()[0];
			} else if (parent.waypoints.Count() > 0) {
				headingTarget = vector.Direction(GetPosition(), parent.waypoints[0]).VectorToAngles().GetRelAngles()[0];
			}
			
			//if (parent.WantsWeaponUp())
			
			float delta = Math.DiffAngle(headingTarget, Math.RAD2DEG * pModel.m_fHeadingAngle);
			
			// this is a workaround for pesky headbug :)
			// Basically, under certain circumstances (seemingly after a full rotation), the m_fHeadingAngle would diverge from the 
			// m_OrientationAngle, and there was little I could do. This bug is rare but debilitating, since it basically disables tracking 
			// of the AI. So, for now... the best workaround seems to be resetting the unit's heading to the desired m_fHeadingAngle. For a 
			// graph explaining it, ask me.
			if (delta < 0.02 && Math.DiffAngle(Math.RAD2DEG * pModel.m_fOrientationAngle, Math.RAD2DEG * pModel.m_fHeadingAngle) > 60.0) {
				SetOrientation(Vector(Math.RAD2DEG * pModel.m_fHeadingAngle, 0, 0));
				pModel.m_fOrientationAngle = pModel.m_fHeadingAngle;
			}

			delta *= (1/360);

			//pModel.m_fOrientationAngle += delta;
			
			// Can be used to make a cool graph
			//Print("HeadingModel - orientation: " + pModel.m_fOrientationAngle + " heading: " + pModel.m_fHeadingAngle + " delta: " + delta);
			
			GetInputController().OverrideAimChangeX(true, delta);
			
			return DayZPlayerImplementHeading.RotateOrient(pDt, pModel, m_fLastHeadingDiff);
			//return DayZPlayerImplementHeading.ClampHeading(pDt, pModel, m_fLastHeadingDiff);
			//return DayZPlayerImplementHeading.NoHeading(pDt, pModel, m_fLastHeadingDiff);
			//return false;

			//return test;
		}
		
		return super.HeadingModel(pDt, pModel);
	}
	
	override void CommandHandler(float pDt, int pCurrentCommandID, bool pCurrentCommandFinished)	
	{
		super.CommandHandler(pDt, pCurrentCommandID, pCurrentCommandFinished);
		if (isAI()) {
			//m_DirectionToCursor = m_CurrentCamera.GetBaseAngles();
			//if (m_CurrentCamera)
				//m_CurrentCamera.OnUpdate(pDt, m_pOutResult);
		}
	}
	
	override void OnCameraChanged(DayZPlayerCameraBase new_camera)
	{
		Print("Camera for " + this.ToString() + " changed. old:" + m_CurrentCamera.ToString() + " new:"+new_camera.ToString());
		m_CameraSwayModifier = new_camera.GetWeaponSwayModifier();
		m_CurrentCamera = new_camera;
	}
	
	override void OnCommandDeathStart() {
		// Tell the server to stop computing AI stuffs
		if (isAI() && GetGame().IsServer()) {
			eAIGlobal_UnitKilled = true;
			parent.markDead();
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