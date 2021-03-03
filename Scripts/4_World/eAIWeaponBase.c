modded class Weapon_Base {
	override bool LiftWeaponCheck (PlayerBase player)
	{
		int idx;
		float distance;
		float hit_fraction; //junk
		vector start, end;
		vector direction;
		vector usti_hlavne_position;
		vector trigger_axis_position;
		vector hit_pos, hit_normal; //junk
		Object obj;
		ItemBase attachment;
		
		m_LiftWeapon = false;
		// not a gun, no weap.raise for now
		if ( HasSelection("Usti hlavne") )
			return false;
		
		if (!player)
		{
			Print("Error: No weapon owner, returning");
			return false;
		}
		
		// add a separate check here since AI cannot use that part of the HIC
		if ( !player.isAI() && player.GetInputController() && !player.GetInputController().IsWeaponRaised() )
			return false;
		
		// if the AI is not targeting, we don't want a raise anyways
		if ( player.isAI() && !player.AIWeaponRaise() )
			return false;
		
		usti_hlavne_position = GetSelectionPositionLS( "Usti hlavne" ); 	// Usti hlavne
		trigger_axis_position = GetSelectionPositionLS("trigger_axis");
		
		// freelook raycast
		if (player.GetInputController().CameraIsFreeLook())
		{
			if (player.m_DirectionToCursor != vector.Zero)
			{
				direction = player.m_DirectionToCursor;
			}
			// if player raises weapon in freelook
			else
			{
				direction = MiscGameplayFunctions.GetHeadingVector(player);
			}
		}
		else
		{
			direction = GetGame().GetCurrentCameraDirection(); // exception for freelook. Much better this way!
		}
		
		idx = player.GetBoneIndexByName("Neck"); //RightHandIndex1
		if ( idx == -1 )
			{ start = player.GetPosition()[1] + 1.5; }
		else
			{ start = player.GetBonePositionWS(idx); }
		
		//! snippet below measures distance from "RightHandIndex1" bone for lifting calibration
		/*usti_hlavne_position = ModelToWorld(usti_hlavne_position);
		distance = vector.Distance(start,usti_hlavne_position);*/
		distance = m_WeaponLength;// - 0.05; //adjusted raycast length

		// if weapon has battel attachment, does longer cast
		if (ItemBase.CastTo(attachment,FindAttachmentBySlotName("weaponBayonet")) || ItemBase.CastTo(attachment,FindAttachmentBySlotName("weaponBayonetAK")) || ItemBase.CastTo(attachment,FindAttachmentBySlotName("weaponBayonetMosin")) || ItemBase.CastTo(attachment,FindAttachmentBySlotName("weaponBayonetSKS")) || ItemBase.CastTo(attachment,GetAttachedSuppressor()))
		{
			distance += attachment.m_ItemModelLength;
		}
		end = start + (direction * distance);
		DayZPhysics.RayCastBullet(start, end, hit_mask, player, obj, hit_pos, hit_normal, hit_fraction);
		
		// something is hit
		if (hit_pos != vector.Zero)
		{
			//Print(distance);
			m_LiftWeapon = true;
			return true;
		}
		return false;
	}
}