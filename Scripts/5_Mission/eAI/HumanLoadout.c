class HumanLoadout {	
	ref TStringArray Shirts = {"HikingJacket_Blue"};
	ref TStringArray Pants = {"SlacksPants_Blue"}; 							
	ref TStringArray Shoes = {"HikingBootsLow_Blue"};			
	ref TStringArray BackPacks = {"TaloonBag_Blue"};					
	ref TStringArray Vests = {"PressVest_Blue"};		
	ref TStringArray Headgear = {"BaseballCap_Blue"};
	ref TStringArray Gloves = {"SurgicalGloves_Blue"};	
	ref TStringArray Misc = {"CivilianBelt"};																			
	ref TIntArray	 ClothesHealth = {10,100}; 						//Item health given. 10%->100%
	
	ref TStringArray WeaponMelee = {"MeleeBat"}; 	
	ref TStringArray WeaponRifle = {"Ruger1022"}; 	
	ref TIntArray	 WeaponRifleMagCount = {1,3}; 	
	ref TStringArray WeaponHandgun = {"MakarovIJ70"}; 		
	ref TIntArray	 WeaponHandgunMagCount = {1,3}; 	
	ref TIntArray	 WeaponHealth = {10,100}; 						//Weapon health given. 10%->100%

	ref TStringArray Loot = {"SodaCan_Cola"};  						//These are added always
	ref TStringArray LootRandom = {"Screwdriver"};  				//Added with a LootRandomChance%
//	ref TIntSet	 	 LootRandomChance = {30};						//Add item from Loot array
	ref TIntArray	 LootHealth = {10,100}; 						//Item health given. 10%->100%
		
	//---------------------
	static string LoadoutSaveDir = "$profile:";
	static string LoadoutDataDir = "eAI/Scripts/Data/Loadout/";
	
	static void Apply(PlayerBase h) {}

	static void AddClothes(PlayerBase h, SoldierLoadout Loadout) {
		EntityAI item;
		int minhealth = Loadout.ClothesHealth[0];
		int maxhealth = Loadout.ClothesHealth[1];
		float HealthModifier;
		
		item = h.GetInventory().CreateInInventory(Loadout.Pants.GetRandomElement());
			HealthModifier = (Math.RandomInt(minhealth, maxhealth)) / 100;
			item.SetHealth(item.GetMaxHealth() * HealthModifier);
//			Print("HumanLoadout: Add pants: " + item.GetMaxHealth() + "/" + HealthModifier + "/" + item.GetMaxHealth() * HealthModifier + ")" );
		item = h.GetInventory().CreateInInventory(Loadout.Shirts.GetRandomElement());
			HealthModifier = (Math.RandomInt(minhealth, maxhealth)) / 100;
			item.SetHealth(item.GetMaxHealth() * HealthModifier);
		item = h.GetInventory().CreateInInventory(Loadout.Shoes.GetRandomElement());
			HealthModifier = (Math.RandomInt(minhealth, maxhealth)) / 100;
			item.SetHealth(item.GetMaxHealth() * HealthModifier);
		item = h.GetInventory().CreateInInventory(Loadout.Headgear.GetRandomElement());
			HealthModifier = (Math.RandomInt(minhealth, maxhealth)) / 100;
			item.SetHealth(item.GetMaxHealth() * HealthModifier);
		item = h.GetInventory().CreateInInventory(Loadout.Gloves.GetRandomElement());			
			HealthModifier = (Math.RandomInt(minhealth, maxhealth)) / 100;
			item.SetHealth(item.GetMaxHealth() * HealthModifier);
		item = h.GetInventory().CreateInInventory(Loadout.BackPacks.GetRandomElement());
			HealthModifier = (Math.RandomInt(minhealth, maxhealth)) / 100;
			item.SetHealth(item.GetMaxHealth() * HealthModifier);
		item = h.GetInventory().CreateInInventory(Loadout.Vests.GetRandomElement());
			HealthModifier = (Math.RandomInt(minhealth, maxhealth)) / 100;
			item.SetHealth(item.GetMaxHealth() * HealthModifier);
		item = h.GetInventory().CreateInInventory(Loadout.Misc.GetRandomElement());
			HealthModifier = (Math.RandomInt(minhealth, maxhealth)) / 100;
			item.SetHealth(item.GetMaxHealth() * HealthModifier);

		Print("HumanLoadout: Added clothes");
	}
		
	static void AddWeapon(PlayerBase h, string weapon, int minhealth = 100, int maxhealth = 100) {
		EntityAI gun = h.GetHumanInventory().CreateInHands(weapon);
		float HealthModifier = (Math.RandomInt(minhealth, maxhealth)) / 100;
		gun.SetHealth(gun.GetMaxHealth() * HealthModifier);
		Print("HumanLoadout: Add weapon: " + weapon + " (" + HealthModifier + ")" );
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
//		HumanLoadout.AddWeapon(h, weapon);
		HumanLoadout.AddWeapon(h, weapon, Loadout.WeaponHealth[0], Loadout.WeaponHealth[1]);
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
//		HumanLoadout.AddWeapon(h, weapon);
		HumanLoadout.AddWeapon(h, weapon, Loadout.WeaponHealth[0], Loadout.WeaponHealth[1]);
		HumanLoadout.AddMagazine(h, weapon, 2);
	}
}