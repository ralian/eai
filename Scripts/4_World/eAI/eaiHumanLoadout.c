class HumanLoadout {	
	static int LOADOUT_VERSION = 001;
	
	ref TIntArray 	 Version = {000};								//!!DO NOT CHANGE!!				
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
	ref TStringArray WeaponAttachment = {"any"}; 	
	ref TStringArray WeaponOptic = {"any"}; 	
	
	ref TStringArray WeaponHandgun = {"MakarovIJ70"}; 		
	ref TIntArray	 WeaponHandgunMagCount = {1,3}; 	
	ref TStringArray WeaponHandgunAttachment = {"any"}; 	
	ref TStringArray WeaponHandgunOptic = {"any"}; 	

	ref TIntArray	 WeaponHealth = {10,100}; 						//Weapon health given. 10%->100%
	ref TIntArray	 AttachmentHealth = {10,100}; 						//Weapon health given. 10%->100%

	ref TStringArray Loot = {"SodaCan_Cola"};  						//These are added always
	ref TStringArray LootRandom = {"Screwdriver"};  				//Added with a LootRandomChance%
	ref TIntArray 	 LootRandomChance = {30};						//Add item from Loot array
	ref TIntArray	 LootHealth = {10,100}; 						//Item health given. 10%->100%
		
	//---------------------
	static string LoadoutSaveDir = "$profile:eAI/Loadout/";
	static string LoadoutDataDir = "eAI/Scripts/Data/Loadout/";
	//---------------------
	
	//----------------------------------------------------------------
	//	HumanLoadout.Apply
	//
	//	Usage: HumanLoadout.Apply(pb_AI, "LoadOut.json");
	//	       HumanLoadout.Apply(pb_AI, "$profile:myloadouts/SoldierLoadout.json");
	//
	//	The order of searching for loadouts:
	//	1) From full path under profile. You can define the path to you mod's loadout files "\mymod\loadout.json"
	//	2) From default eAI config dir under profile "profile\eAI\loadout\*".
	//	3) Copied from the mod (Scripts/Data/Loadout) to "profile\eAI\loadout\*". 
	//	   Two default loadouts exists: "SoldierLoadout.json" , "PoliceLoadout.json".
	//	4) Create a dummy loadout (blue clothes) with the given LoadoutFile filename under "profile\eAI\loadout\".
	
	static void Apply(PlayerBase h, string LoadoutFile) {
		HumanLoadout Loadout = LoadData(LoadoutFile);
		HumanLoadout.AddClothes(h, Loadout);

		string weapon;
		
		weapon = Loadout.WeaponRifle.GetRandomElement();
		HumanLoadout.AddWeapon(h, Loadout, weapon);
		HumanLoadout.AddMagazine(h, weapon, Loadout.WeaponRifleMagCount[0], Loadout.WeaponRifleMagCount[1]);
		weapon = Loadout.WeaponHandgun.GetRandomElement();
		HumanLoadout.AddWeapon(h, Loadout, weapon);
		HumanLoadout.AddMagazine(h, weapon, Loadout.WeaponHandgunMagCount[0], Loadout.WeaponHandgunMagCount[1]);

		HumanLoadout.AddLoot(h, Loadout);		
	}

	//----------------------------------------------------------------
	//	HumanLoadout.AddLoot
	//
	//	Adds loot.
	
	static void AddLoot(PlayerBase h, HumanLoadout Loadout) {
		EntityAI item;
		int minhealth = Loadout.LootHealth[0];
		int maxhealth = Loadout.LootHealth[1];		
		float HealthModifier;
		string loot;
		int i;
		
		for( i = 0; i < Loadout.Loot.Count(); i++)
		{
			loot = Loadout.Loot[i];
			item = h.GetHumanInventory().CreateInInventory(loot);
			HealthModifier = (Math.RandomInt(minhealth, maxhealth)) / 100;
			item.SetHealth(item.GetMaxHealth() * HealthModifier);
			Print("HumanLoadout: Add loot: " + loot + " (" + HealthModifier + ")" );
		}

		for( i = 0; i < Loadout.LootRandom.Count(); i++)
		{
			loot = Loadout.LootRandom[i];
			if (Loadout.LootRandomChance < Math.RandomInt(0, 100))
			{
				item = h.GetHumanInventory().CreateInInventory(loot);
				HealthModifier = (Math.RandomInt(minhealth, maxhealth)) / 100;
				item.SetHealth(item.GetMaxHealth() * HealthModifier);
				Print("HumanLoadout: Add random loot: " + loot + " (" + HealthModifier + ")" );
			}
		}
	}	
	
	//----------------------------------------------------------------
	//	HumanLoadout.AddClothes
	//
	//	Adds a clothes to.
	
	static void AddClothes(PlayerBase h, HumanLoadout Loadout) {
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

	//----------------------------------------------------------------
	//	HumanLoadout.AddWeapon
	//
	//	Adds a weapon to AI. First weapon is put to hands. The rest go to inventory.
	//
	//	Usage: HumanLoadout.AddWeapon(pb_AI, Loadout, "AKM");		//An AKM is added
	//		   

	static void AddWeapon(PlayerBase h, HumanLoadout Loadout, string weapon = "") {
		EntityAI gun;
		float HealthModifier;
		
		int minhealth = 100;
		int maxhealth = 100;
		
		if (weapon != "")
		{
			//If there is a weapon already in hand, create the next one in inventory		
			if (h.GetHumanInventory().GetEntityInHands() == null) 
			{
				gun = h.GetHumanInventory().CreateInHands(weapon);
			}
			else
			{
				gun = h.GetHumanInventory().CreateInInventory(weapon);
			}
			
			HealthModifier = (Math.RandomInt(minhealth, maxhealth)) / 100;
			gun.SetHealth(gun.GetMaxHealth() * HealthModifier);
	
			//Add optics
			string optic = "";
			if (Loadout.WeaponRifle.Find(weapon) > -1)
			{
				optic = Loadout.WeaponOptic.GetRandomElement();
			}
			if (Loadout.WeaponHandgun.Find(weapon) > -1)			
			{
				optic = Loadout.WeaponHandgunOptic.GetRandomElement();
			}
			TStringArray optics = darc_ListOptics(weapon);
			minhealth = Loadout.AttachmentHealth[0];
			maxhealth = Loadout.AttachmentHealth[1];
			AddWeapon_Helper_attachment(gun, optic, optics, minhealth, maxhealth);		
			
			//Add attachment
			string attachment = "";
			if (Loadout.WeaponRifle.Find(weapon) > -1)
			{
				attachment = Loadout.WeaponAttachment.GetRandomElement();
			}
			if (Loadout.WeaponHandgun.Find(weapon) > -1)			
			{
				attachment = Loadout.WeaponHandgunAttachment.GetRandomElement();
			}
			TStringArray attachments = darc_ListAttachments(weapon);
			minhealth = Loadout.AttachmentHealth[0];
			maxhealth = Loadout.AttachmentHealth[1];
			AddWeapon_Helper_attachment(gun, attachment, attachments, minhealth, maxhealth);
			
			Print("HumanLoadout: Added weapon: " + weapon + " (" + HealthModifier + ")" );
		}
	}

	//----------------------------------------------------------------
	//	HumanLoadout.AddWeapon_Helper_attachment
	
	static bool AddWeapon_Helper_attachment(EntityAI gun, string attachment, TStringArray attachments, int minhealth = 100, int maxhealth = 100) {
		EntityAI att;
		float HealthModifier;
		
		if (attachment == "any")
		{
			 attachment = attachments.GetRandomElement();
		}
		att = gun.GetInventory().CreateAttachment(attachment);
		HealthModifier = (Math.RandomInt(minhealth, maxhealth)) / 100;
		att.SetHealth(att.GetMaxHealth() * HealthModifier);	
		
		return true; //TBD: return true/false if succeeded
	}
	
	//----------------------------------------------------------------
	//	HumanLoadout.AddMagazine
	//
	//	Adds magazines to AI inventory.
	//
	//	Usage: HumanLoadout.AddMagazine(pb_AI, "AKM", 3);		//Three random AKM magazines are added
	//	       HumanLoadout.AddMagazine(pb_AI, "AKM", 1, 4);	//1 to 4 random AKM magazines are added

	static void AddMagazine(PlayerBase h, string weapon, int mincount = 1, int maxcount = 0) {
        TStringArray magazines = {};
		magazines = darc_ListMagazines(weapon);
		string mag = magazines.GetRandomElement();

		int i;
		int count = mincount;
		
		if (maxcount > 0)
		{
			//Give random amount of mags from mincount to maxcount. +1 is needed as maxcount is excluded in RandomInt
			count = Math.RandomInt(mincount, maxcount + 1);	
		}
		
		if (count < 1)
		{
			//Maxcount probably was larger than mincount
			count = 1;
			Print("HumanLoadout: ERROR: Please check you Weapon___MagCount. Giving 1 mag.");
		}
		
		for( i = 0; i < count; i++)
		{
			h.GetHumanInventory().CreateInInventory(mag);
		}

		Print("HumanLoadout: Add " + count + " of " + mag + " magazines for weapon " + weapon);
	}
	
	//----------------------------------------------------------------
	//	HumanLoadout.LoadData

	static HumanLoadout LoadData(string FileName) {
		string LoadoutFileName = "";
		string LoadoutDefaultFileName = LoadoutDataDir + FileName;
		
        ref HumanLoadout data = new ref HumanLoadout;
		
		//Check if the given Filename is with full patch to existing file.
        if (FileExist(FileName))
		{
			LoadoutFileName = FileName;
		}
		else //Nope, let's use LoadoutSaveDir
		{
			LoadoutFileName = LoadoutSaveDir + FileName;			
		}
		
        Print("HumanLoadout: LoadData: Looking for " + FileName);

        if (!FileExist(LoadoutFileName))
        {
			//No Loadout file exists. Check if a default under Data\Loadout with same name exists.
			if(FileExist(LoadoutDefaultFileName))
			{
				//Profile does not have the loadouts. Copy them from Data\Loadout. 
	            Print("HumanLoadout: " + LoadoutFileName + " doesn't exist, copying default file!");
				CopyFile(LoadoutDefaultFileName, LoadoutFileName);
			}
			else
			{
				//If the files under Data\Loadout in mod does not exist, create a default from the class. 
				//This is an error situation but useful if you need to create a clean and working json
	            Print("HumanLoadout: " + LoadoutDefaultFileName + " doesn't exist. Creating a default file: " + LoadoutFileName);
	            SaveData(LoadoutFileName, data);
			}
		}

		if (FileExist(LoadoutFileName))
        {
            Print("HumanLoadout: " + LoadoutFileName + " exists, loading!");
            JsonFileLoader<HumanLoadout>.JsonLoadFile(LoadoutFileName, data);
			
			//If the version does not match, show an error. The JSON loader loads what it is able to load, but does not completely fail.
			if (data.Version[0] != LOADOUT_VERSION)
			{
	            Print("HumanLoadout: ERROR : Incorrect version (" + data.Version[0] + ") in " + LoadoutFileName + ". Should be " + LOADOUT_VERSION);
	            Print("HumanLoadout: ERROR : A fixed version has been saved: " + LoadoutFileName + "_fixed");
	            SaveData(LoadoutFileName + "_fixed", data);
			}			
        }
        else
        {
			//We should never come here.
            Print("HumanLoadout: ERROR : Could not find " + LoadoutFileName);
		}

        return data;
    }

	//----------------------------------------------------------------
	//	HumanLoadout.SaveData
	
    static void SaveData(string FileName, ref HumanLoadout data) {
		Print("HumanLoadout: Saving loadout to " + FileName);
		data.Version[0] = LOADOUT_VERSION;
        JsonFileLoader<HumanLoadout>.JsonSaveFile(FileName, data);
    }	
};