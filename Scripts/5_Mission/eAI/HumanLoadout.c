class HumanLoadout {	
	ref TStringArray Shirts = {"HikingJacket_Blue "};
	ref TStringArray Pants = {"SlacksPants_Blue"}; 							
	ref TStringArray Shoes = {"HikingBootsLow_Blue"};			
	ref TStringArray BackPacks = {"TaloonBag_Blue"};					
	ref TStringArray Vests = {"PressVest_Blue"};		
	ref TStringArray Headgear = {"BaseballCap_Blue "};
	ref TStringArray Gloves = {"SurgicalGloves_Blue"};	
	ref TStringArray Misc = {"CivilianBelt"};																			
	
	ref TStringArray WeaponMelee = {"Pickaxe", "WoodAxe", "FirefighterAxe", "Shovel"}; 	
	ref TStringArray WeaponRifle = {"M4A1", "AKM", "SVD", "M4A1"}; 	
	ref TIntArray	 WeaponRifleMagCount = {1,3}; 	
	ref TStringArray Loot = {"SodaCan_Cola"};  								//These are added always
	ref TStringArray LootRandom = {"Rope", "Screwdriver"};  				//Added with a LootRandomChance%
//	ref TIntSet	 	 LootRandomChance = {30};									//Add item from Loot array
	ref TStringArray WeaponHandgun = {""}; 		
	ref TIntArray	 WeaponHandgunMagCount = {1,3}; 	
	
	static string LoadoutSaveDir = "$profile:";
	static string LoadoutDataDir = "eAI/Scripts/Data/Loadout/";
	
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
//		ItemBase itemBs;
//		itemBs = ItemBase.Cast(gun);
//        SetHealth(itemBs, 20);		
//		gun.SetHealth("","",100);
		gun.SetHealth(gun.GetMaxHealth()*0.5);
		Print("HumanLoadout: Add weapon: " + weapon);
	}
	
	static void AddMagazine(PlayerBase h, string weapon, int mincount, int maxcount = 0) {
        TStringArray magazines = {};
        GetGame().ConfigGetTextArray("CfgWeapons " + weapon + " magazines", magazines);		
		string mag = magazines.GetRandomElement();

		int i;
		int count = 3;
		if (maxcount > 0)
		{
			//TBD: count = random value between mincount and maxcount
		}
	
		for( i = 0; i < count; i++)
		{
			h.GetHumanInventory().CreateInInventory(mag);
		}

		Print("HumanLoadout: Add " + count + " of " + mag + " magazines for weapon " + weapon);
	}
	
    static HumanLoadout LoadData(string FileName)
    {
		string LoadoutFileName = LoadoutSaveDir + FileName;
		string LoadoutDefaultFileName = LoadoutDataDir + FileName;
		
        ref HumanLoadout data = new ref HumanLoadout;
        Print("HumanLoadout: LoadData: Looking for " + FileName);

        if(!FileExist(LoadoutFileName))
        {
			if(FileExist(LoadoutDefaultFileName))
			{
				//Profile does not have the loadouts. Copy them from mod. 
	            Print("HumanLoadout: " + LoadoutFileName + " doesn't exist, copying default file!");
				CopyFile(LoadoutDefaultFileName, LoadoutFileName);
			}
			else
			{
				//If the files under Data\Loadout in mod does not exist, create a default from the class. 
				//This is an error situation but useful if you need to create a clean and working json
	            Print("HumanLoadout: " + LoadoutDefaultFileName + " doesn't exist, creating a default file: " + LoadoutFileName);
	            SaveData(LoadoutFileName, data);
			}
		}

		if(FileExist(LoadoutFileName))
        {
            Print("HumanLoadout: " + LoadoutFileName + " exists, loading!");
            JsonFileLoader<HumanLoadout>.JsonLoadFile(FileName, data);
        }
        else
        {
            Print("HumanLoadout: ERROR : Coult not find " + LoadoutFileName);
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
	static string SoldierLoadoutSave = "SoldierLoadout.json";
	
	override static void Apply(PlayerBase h)	
	{
		HumanLoadout Loadout = LoadData(SoldierLoadoutSave);
		HumanLoadout.AddClothes(h, Loadout);

		string weapon = Loadout.WeaponRifle.GetRandomElement();
		HumanLoadout.AddWeapon(h, weapon);
		HumanLoadout.AddMagazine(h, weapon, Loadout.WeaponRifleMagCount[0], Loadout.WeaponRifleMagCount[1]);
	}
}	

class PoliceLoadout : HumanLoadout {
	static string PoliceLoadoutSave = "PoliceLoadout.json";
	
	override static void Apply(PlayerBase h)
	{
		HumanLoadout Loadout = LoadData(PoliceLoadoutSave);
		HumanLoadout.AddClothes(h, Loadout);

		string weapon = Loadout.WeaponRifle.GetRandomElement();
		HumanLoadout.AddWeapon(h, weapon);
		HumanLoadout.AddMagazine(h, weapon, 2);
	}
}