class HumanLoadout {	
	ref TStringArray Shirts = {"PoliceJacket", "PoliceJacketOrel"};
	ref TStringArray Pants = {"PolicePants", "PolicePantsOrel"}; 							
	ref TStringArray Shoes = {"CombatBoots_Black"};			
	ref TStringArray BackPacks = {""};					
	ref TStringArray Vests = {"PoliceVest"};		
	ref TStringArray Headgear = {"", "DirtBikeHelmet_Police", "PoliceCap", "OfficerHat"};
	ref TStringArray Gloves = {"LeatherGloves_Black", "TacticalGloves_Black", "WorkingGloves_Black"};	
	ref TStringArray Misc = {"", "MilitaryBelt"};																			
	
	ref TStringArray WeaponMelee = {"Pickaxe", "WoodAxe", "FirefighterAxe", "Shovel"}; 	
	ref TStringArray WeaponRifle = {"M4A1", "AKM", "SVD", "M4A1"}; 	
	ref TStringArray Loot = {"Rope", "Screwdriver"};  
	ref TStringArray WeaponHandgun = {""}; 		
	
	static void Apply(PlayerBase h) {}

	static void AddClothes(PlayerBase h, SoldierLoadout Loadout) {
		h.GetInventory().CreateInInventory(Loadout.Pants.GetRandomElement());
		h.GetInventory().CreateInInventory(Loadout.Shirts.GetRandomElement());
		h.GetInventory().CreateInInventory(Loadout.Shoes.GetRandomElement());
		h.GetInventory().CreateInInventory(Loadout.Headgear.GetRandomElement());
		h.GetInventory().CreateInInventory(Loadout.Gloves.GetRandomElement());			
		h.GetInventory().CreateInInventory(Loadout.BackPacks.GetRandomElement());
		h.GetInventory().CreateInInventory(Loadout.Vests.GetRandomElement());
		h.GetInventory().CreateInInventory(Loadout.Misc.GetRandomElement());

		Print("HumanLoadout: Added clothes");
	}
		
	static void AddWeapon(PlayerBase h, string weapon) {
		EntityAI gun = h.GetHumanInventory().CreateInHands(weapon);
		
		Print("HumanLoadout: Add weapon: " + weapon);
	}
	
	static void AddMagazine(PlayerBase h, string weapon, int count) {
        TStringArray magazines = {};
        GetGame().ConfigGetTextArray("CfgWeapons " + weapon + " magazines", magazines);		
		string mag = magazines.GetRandomElement();

		int i;
		for( i = 0; i < count; i++)
		{
			h.GetHumanInventory().CreateInInventory(mag);
		}

		Print("HumanLoadout: Add " + count + " of " + mag + " magazines for weapon " + weapon);
	}
	
    static HumanLoadout LoadData(string FileName)
    {
        ref HumanLoadout data = new ref HumanLoadout;
        Print("HumanLoadout: LoadData: Looking for " + FileName);

        if(FileExist(FileName))
        {
            Print("HumanLoadout:" + FileName + " exists, loading!");
            JsonFileLoader<HumanLoadout>.JsonLoadFile(FileName, data);
        }
        else
        {
            Print("HumanLoadout:" + FileName + " doesn't exist, creating file!");
            SaveData(FileName, data);
        }

        return data;
    }

    static void SaveData(string FileName, ref HumanLoadout data)
    {
		Print("HumanLoadout: Saving loadout to " + FileName);
        JsonFileLoader<HumanLoadout>.JsonSaveFile(FileName, data);
    }	
	
};

class SoldierLoadout : HumanLoadout {
	static string SoldierLoadoutSave = "$profile:SoldierLoadout.json";

/*	ref TStringArray Shirts = {"GorkaEJacket_Autumn", "GorkaEJacket_Flat", "GorkaEJacket_PautRev", "GorkaEJacket_Summer"};
	ref TStringArray Pants = {"GorkaPants_Autumn", "GorkaPants_Flat", "GorkaPants_PautRev", "GorkaPants_Summer"}; 							
	ref TStringArray Shoes = {"TTSKOBoots", "CombatBoots_Black", "CombatBoots_Brown"};			
	ref TStringArray BackPacks = {"", "", "CoyoteBag_Brown", "CoyoteBag_Green"};					
	ref TStringArray Vests = {"HighCapacityVest_Black", "PlateCarrierVest", "UKAssVest_Camo"};		
	ref TStringArray Headgear = {"GorkaHelmet", "Mich2001Helmet", "MotoHelmet_Black", "SkateHelmet_Black", "DirtBikeHelmet_Black"};
	ref TStringArray Gloves = {"WorkingGloves_Beige", "WorkingGloves_Black", "NBCGlovesGray", "OMNOGloves_Gray", "OMNOGloves_Brown"};	
	ref TStringArray Misc = {"", "CivilianBelt", "MilitaryBelt"};																			
	
	ref TStringArray WeaponMelee = {"Pickaxe", "WoodAxe", "FirefighterAxe", "Shovel"}; 	
	ref TStringArray WeaponRifle = {"M4A1", "AKM", "SVD"}; 	
	ref static TStringArray WeaponHandgun = {""}; 	
	ref static TStringArray Loot = {"SodaCan_Cola"};  */

	static void Apply(PlayerBase h)
	{
		HumanLoadout Loadout = LoadData(SoldierLoadoutSave);
		HumanLoadout.AddClothes(h, Loadout);

		string weapon = Loadout.WeaponRifle.GetRandomElement();
		HumanLoadout.AddWeapon(h, weapon);
		HumanLoadout.AddMagazine(h, weapon, 2);
	}
}	

class PoliceLoadout : HumanLoadout {
	static string PoliceLoadoutSave = "$profile:PoliceLoadout.json";

/*	ref TStringArray PoliceLoadoutShirts = {"PoliceJacket", "PoliceJacketOrel"};
	ref TStringArray PoliceLoadoutPants = {"PolicePants", "PolicePantsOrel"}; 							
	ref TStringArray PoliceLoadoutShoes = {"CombatBoots_Black"};			
	ref TStringArray PoliceLoadoutBackPacks = {""};					
	ref TStringArray PoliceLoadoutVests = {"PoliceVest"};		
	ref TStringArray PoliceLoadoutHeadgear = {"", "DirtBikeHelmet_Police", "PoliceCap", "OfficerHat"};
	ref TStringArray PoliceLoadoutGloves = {"LeatherGloves_Black", "TacticalGloves_Black", "WorkingGloves_Black"};	
	ref TStringArray PoliceLoadoutMisc = {"", "MilitaryBelt"};																			
	
	ref TStringArray PoliceLoadoutWeaponMelee = {"Pickaxe", "WoodAxe", "FirefighterAxe", "Shovel"}; 	
	ref TStringArray PoliceLoadoutWeaponRifle = {"M4A1", "AKM", "SVD", "M4A1"}; 	
	ref TStringArray Loot = {"Rope", "Screwdriver"};  
	ref TStringArray PoliceLoadoutWeaponHandgun = {""};	*/

	static void Apply(PlayerBase h)
	{
		HumanLoadout Loadout = LoadData(PoliceLoadoutSave);
		HumanLoadout.AddClothes(h, Loadout);

		string weapon = Loadout.WeaponRifle.GetRandomElement();
		HumanLoadout.AddWeapon(h, weapon);
		HumanLoadout.AddMagazine(h, weapon, 2);
	}
}