// A list of functions to list weapons, attachments, optics
//
// This code is heavily inspired from Hlynge's mod DumpAttatch
// https://steamcommunity.com/sharedfiles/filedetails/?id=2372233619

//--------------------------------------------------------------
// darc_ListWeapons
//
// List all weapons available. 
//
// NOTE: I have not tested this with any mod created weapon packs. In theory should list also them.

TStringArray darc_ListWeapons(string cfg) {
	
	TStringArray weapons = {};
	TStringArray notaweapon = {"access", "DefaultWeapon"};
	
    for (int i = 0; i < GetGame().ConfigGetChildrenCount("CfgWeapons"); i++)
    {
        string name;
        GetGame().ConfigGetChildName("CfgWeapons", i, name);

		Print("Weapon name: " + name);
 
		//Ignore possible empty ones
        if (name == string.Empty)
            continue;
		
		//CfgWeapons include some weird entries - ignore them
		if (notaweapon.Find(name) > -1)
			continue;
        
		if (endsWith(name, "_Base"))
		{
			//Not a weapon (for example: Rilfe_Base)
			continue;
		}
		else
		{
			weapons.Insert(name);
		}	
	}

	return weapons;
}

//--------------------------------------------------------------
// darc_ListMagazines
//
// List all compatible magazines for weapon. If magazines are not found, returns compatible ammo.
//
// NOTE: This function needs more testing for the ammo case. Might work for launchers too.

TStringArray darc_ListMagazines(string weapon) {
	TStringArray magazines = {};
	GetGame().ConfigGetTextArray("CfgWeapons " + weapon + " magazines", magazines);
	
	if(magazines.Count() == 0)
	{
		GetGame().ConfigGetTextArray("CfgWeapons " + weapon + " chamberableFrom", magazines);
	}
	
	return magazines;
}

//--------------------------------------------------------------
// darc_ListOptics
//
// List all compatible optics for weapon.
//
// NOTE: Attachments could be listed also with the same method as darc_ListAttachments, but for some reason
//       the results are not the same. This seems to provide _all_ while the other method does not include 
//		 for example ACOGOptic for M4A1.  

TStringArray darc_ListOptics(string weapon) {
	TStringArray optics = {};
	GetGame().ConfigGetTextArray("CfgWeapons " + weapon + " ironsightsExcludingOptics", optics);			
	return optics;
}

//--------------------------------------------------------------
// darc_ListAttachments
//
// List all compatible attachments without optics for weapon.

TStringArray darc_ListAttachments(string weapon) {
	
	TStringArray arr_compatible = {};
	TStringArray attachments = {};
	TStringArray optics = darc_ListOptics(weapon);
	
//	Print("Looking for " + weapon + " attachments");
	
	GetGame().ConfigGetTextArray("CfgWeapons " + weapon + " attachments", arr_compatible);			
	
    for (int j = 0; j < GetGame().ConfigGetChildrenCount("CfgVehicles"); j++)
    {
        string item_name;
        GetGame().ConfigGetChildName("CfgVehicles", j, item_name);
		
        TStringArray inventory_slots = {};
			
        switch (GetGame().ConfigGetType("CfgVehicles " + item_name + " inventorySlot"))
        {
            case CT_ARRAY:
            {
				GetGame().ConfigGetTextArray("CfgVehicles " + item_name + " inventorySlot", inventory_slots);
                foreach (string inv : inventory_slots)
                {
					if ( (arr_compatible.Find(inv) > -1) && (inv != string.Empty) && (optics.Find(inv) == -1) )
					{
						attachments.Insert(item_name);
					}
                }
				break;
			}		
            case CT_STRING:
            {
				Print("CT_STRING");
				
				string att;
				GetGame().ConfigGetText("CfgVehicles " + item_name + " inventorySlot", att);
				
				if ( (arr_compatible.Find(att) > -1) & (att != string.Empty) && (optics.Find(inv) == -1) )
					{
						attachments.Insert(item_name);
					}
				
				break;
			}		
		}
	}

	return attachments;	
}

//--------------------------------------------------------------
// endsWith
//
// Credits: Hlynge

bool endsWith(string str, string suffix) 
{
//	Print("endsWith: " + str);
	
	if (str.Length() < suffix.Length())
    	return false;
	return str.Substring(str.Length()-suffix.Length(),suffix.Length()) == suffix;
} 