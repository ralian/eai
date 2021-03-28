class HumanLoadout {
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
};

class SoldierLoadout : HumanLoadout {
	static string SoldierLoadoutSave = "$profile:SoldierLoadout.json";

	ref TStringArray Shirts = {"GorkaEJacket_Autumn", "GorkaEJacket_Flat", "GorkaEJacket_PautRev", "GorkaEJacket_Summer"};
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
	ref static TStringArray Loot = {"SodaCan_Cola"};  

	static void Apply(PlayerBase h)
	{
		SoldierLoadout Loadout = LoadData();
		HumanLoadout.AddClothes(h, Loadout);

		string weapon = Loadout.WeaponRifle.GetRandomElement();
		HumanLoadout.AddWeapon(h, weapon);
		HumanLoadout.AddMagazine(h, weapon, 2);
	}
	
    static SoldierLoadout LoadData()
    {
        ref SoldierLoadout data = new ref SoldierLoadout;

        if(FileExist(SoldierLoadout.SoldierLoadoutSave))
        {
            Print("HumanLoadout:" + SoldierLoadoutSave + " exists, loading!");
            JsonFileLoader<SoldierLoadout>.JsonLoadFile(SoldierLoadoutSave, data);
        }
        else
        {
            Print("HumanLoadout:" + SoldierLoadoutSave + " doesn't exist, creating file!");
            SaveData(data);
        }

        return data;
    }

    static void SaveData(ref SoldierLoadout data)
    {
        JsonFileLoader<SoldierLoadout>.JsonSaveFile(SoldierLoadoutSave, data);
    }
}	

/*
class PoliceLoadout : HumanLoadout {
	static void Apply(PlayerBase h)
	{
		ref TStringArray PoliceLoadoutShirts = {"PoliceJacket", "PoliceJacketOrel"};
		ref TStringArray PoliceLoadoutPants = {"PolicePants", "PolicePantsOrel"}; 							
		ref TStringArray PoliceLoadoutShoes = {"CombatBoots_Black"};			
		ref TStringArray PoliceLoadoutBackPacks = {""};					
		ref TStringArray PoliceLoadoutVests = {"PoliceVest"};		
		ref TStringArray PoliceLoadoutHeadgear = {"", "DirtBikeHelmet_Police", "PoliceCap", "OfficerHat"};
		ref TStringArray PoliceLoadoutGloves = {"LeatherGloves_Black", "TacticalGloves_Black", "WorkingGloves_Black"};	
		ref TStringArray PoliceLoadoutMisc = {"", "MilitaryBelt"};																			
		
		ref TStringArray PoliceLoadoutWeaponMelee = {"Pickaxe", "WoodAxe", "FirefighterAxe", "Shovel"}; 	
		ref TStringArray PoliceLoadoutWeaponRifle = {"M4A1", "AKM", "SVD", "M4A1"}; 	
	//	ref TStringArray Loot = {"Rope", "Screwdriver"};  
	//	ref TStringArray PoliceLoadoutWeaponHandgun = {""}; 	

		h.GetInventory().CreateInInventory(PoliceLoadoutPants.GetRandomElement());
		h.GetInventory().CreateInInventory(PoliceLoadoutShirts.GetRandomElement());
		h.GetInventory().CreateInInventory(PoliceLoadoutShoes.GetRandomElement());
		h.GetInventory().CreateInInventory(PoliceLoadoutHeadgear.GetRandomElement());
		h.GetInventory().CreateInInventory(PoliceLoadoutGloves.GetRandomElement());			
		h.GetInventory().CreateInInventory(PoliceLoadoutBackPacks.GetRandomElement());
		h.GetInventory().CreateInInventory(PoliceLoadoutVests.GetRandomElement());
		h.GetInventory().CreateInInventory(PoliceLoadoutMisc.GetRandomElement());
	}
}*/