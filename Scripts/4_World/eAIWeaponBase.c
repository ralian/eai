modded class Weapon_Base {
	override bool IsRemoteWeapon ()
	{
		Print("IsRemoteWeapon called");
		InventoryLocation il = new InventoryLocation;
		if (GetInventory().GetCurrentInventoryLocation(il))
		{
			EntityAI parent = il.GetParent();
			DayZPlayer dayzp = DayZPlayer.Cast(parent);
			if (il.GetType() == InventoryLocationType.HANDS && dayzp)
			{
				bool remote = dayzp.GetInstanceType() == DayZPlayerInstanceType.INSTANCETYPE_REMOTE || dayzp.GetInstanceType() == DayZPlayerInstanceType.INSTANCETYPE_AI_REMOTE;
				return remote;
			}
		}
		return true;
	}
}