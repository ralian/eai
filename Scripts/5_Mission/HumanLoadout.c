class HumanLoadout {
	static void Apply(PlayerBase h) {}
	
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
	static void Apply(PlayerBase h)
	{
		ref TStringArray SoldierLoadoutShirts = {"GorkaEJacket_Autumn", "GorkaEJacket_Flat", "GorkaEJacket_PautRev", "GorkaEJacket_Summer"};
		ref TStringArray SoldierLoadoutPants = {"GorkaPants_Autumn", "GorkaPants_Flat", "GorkaPants_PautRev", "GorkaPants_Summer"}; 							
		ref TStringArray SoldierLoadoutShoes = {"TTSKOBoots", "CombatBoots_Black", "CombatBoots_Brown"};			
		ref TStringArray SoldierLoadoutBackPacks = {"", "", "CoyoteBag_Brown", "CoyoteBag_Green"};					
		ref TStringArray SoldierLoadoutVests = {"HighCapacityVest_Black", "PlateCarrierVest", "UKAssVest_Camo"};		
		ref TStringArray SoldierLoadoutHeadgear = {"GorkaHelmet", "Mich2001Helmet", "MotoHelmet_Black", "SkateHelmet_Black", "DirtBikeHelmet_Black"};
		ref TStringArray SoldierLoadoutGloves = {"WorkingGloves_Beige", "WorkingGloves_Black", "NBCGlovesGray", "OMNOGloves_Gray", "OMNOGloves_Brown"};	
		ref TStringArray SoldierLoadoutMisc = {"", "CivilianBelt", "MilitaryBelt"};																			
		
		ref TStringArray SoldierLoadoutWeaponMelee = {"Pickaxe", "WoodAxe", "FirefighterAxe", "Shovel"}; 	
		ref TStringArray SoldierLoadoutWeaponRifle = {"M4A1", "AKM", "SVD", "M4A1"}; 	
	//	ref TStringArray Loot = {"SodaCan_Cola"};  
	//	ref TStringArray SoldierLoadoutWeaponHandgun = {""}; 	

		h.GetInventory().CreateInInventory(SoldierLoadoutPants.GetRandomElement());
		h.GetInventory().CreateInInventory(SoldierLoadoutShirts.GetRandomElement());
		h.GetInventory().CreateInInventory(SoldierLoadoutShoes.GetRandomElement());
		h.GetInventory().CreateInInventory(SoldierLoadoutHeadgear.GetRandomElement());
		h.GetInventory().CreateInInventory(SoldierLoadoutGloves.GetRandomElement());			
		h.GetInventory().CreateInInventory(SoldierLoadoutBackPacks.GetRandomElement());
		h.GetInventory().CreateInInventory(SoldierLoadoutVests.GetRandomElement());
		h.GetInventory().CreateInInventory(SoldierLoadoutMisc.GetRandomElement());

		string weapon = SoldierLoadoutWeaponRifle.GetRandomElement();
		HumanLoadout.AddWeapon(h, weapon);
		HumanLoadout.AddMagazine(h, weapon, 2);
	}
}	

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
}