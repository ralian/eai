string GetRandomAI()
{
	int FemaleChance = 50;
	if (Math.RandomInt(0, 100) > FemaleChance)
	{
		return GetRandomAIFemale();
	}
	else
	{
		return GetRandomAIMale();
	}
}

string GetRandomAIFemale()
{
	TStringArray FemaleList = { "SurvivorF_Eva", "SurvivorF_Frida", "SurvivorF_Gabi", "SurvivorF_Helga", 
								"SurvivorF_Irena", "SurvivorF_Judy", "SurvivorF_Keiko", "SurvivorF_Linda", 
								"SurvivorF_Maria", "SurvivorF_Naomi"
							  };
	return "eAI_" + FemaleList.GetRandomElement();
}

string GetRandomAIMale()
{
	TStringArray MaleList = { "SurvivorM_Boris", "SurvivorM_Cyril", "SurvivorM_Denis", "SurvivorM_Elias", 
							  "SurvivorM_Francis", "SurvivorM_Guo", "SurvivorM_Hassan", "SurvivorM_Indar", 
							  "SurvivorM_Jose", "SurvivorM_Kaito", "SurvivorM_Lewis", "SurvivorM_Manua", 
							  "SurvivorM_Mirek", "SurvivorM_Niki", "SurvivorM_Oliver", "SurvivorM_Peter", 
							  "SurvivorM_Quinn", "SurvivorM_Rolf", "SurvivorM_Seth", "SurvivorM_Taiki"
							};
	return "eAI_" + MaleList.GetRandomElement();
}

string GetRandomName()
{
	//Names inspired from https://www.fantasynamegenerators.com . The are neutral names that fit both genders
		
	TStringArray firstname = {	"Billy", "Jessie", "Jaime", "Charlie", "Blake", "Jackie", "Caden", "Vic", "Casey", "Taylor", "Ash", "Addison", "Danny", "Angel", 
								"Jesse", "Rudy", "Blair", "Skyler", "Nicky", "Mell", "Dane", "Bret", "Clem", "Gabe", "River", "Kit", "Reggie", "Drew", "Steff", 
								"Gabe", "Gene", "Jaden", "Sam", "Rory", "Hayden", "Mel", "Phoenix", "Gale", "Caden", "Angel", "Nicky", "Bennie", "Aiden", "Taylor", 
								"Sam", "Ali", "Jess", "Nicky", "Harper", "Angel"
							};
	TStringArray lastname = {	"Allen", "Dixon", "Carter", "Byrne", "Richards", "Houghton", "Bennett", "Morgan", "West", "Rees", "Morris", "Phillips", "Johnston", 
								"Smith", "Jenkins", "May", "Price", "James", "Gardner", "Gibson", "Hill", "Harris", "Mccarthy", "Graham", "Knight", "Gardner", "White", 
								"Hopkins", "Mcdonald", "Green", "Williamson", "Francis", "Brown", "Read", "Anderson", "Elliott", "Powell", "Young", "Shaw", "Woods", 
								"Hunter", "Thomson", "Ryan", "Walker", "Foster", "Hunt", "Pearce", "Cooke", "Palmer", "Parker"
							};	
	
	return firstname.GetRandomElement() + " " + lastname.GetRandomElement();
}

class eAI_SurvivorM_Mirek extends eAIBase {}
class eAI_SurvivorM_Denis extends eAIBase {}
class eAI_SurvivorM_Boris extends eAIBase {}
class eAI_SurvivorM_Cyril extends eAIBase {}
class eAI_SurvivorM_Elias extends eAIBase {}
class eAI_SurvivorM_Francis extends eAIBase {}
class eAI_SurvivorM_Guo extends eAIBase {}
class eAI_SurvivorM_Hassan extends eAIBase {}
class eAI_SurvivorM_Indar extends eAIBase {}
class eAI_SurvivorM_Jose extends eAIBase {}
class eAI_SurvivorM_Kaito extends eAIBase {}
class eAI_SurvivorM_Lewis extends eAIBase {}
class eAI_SurvivorM_Manua extends eAIBase {}
class eAI_SurvivorM_Niki extends eAIBase {}
class eAI_SurvivorM_Oliver extends eAIBase {}
class eAI_SurvivorM_Peter extends eAIBase {}
class eAI_SurvivorM_Quinn extends eAIBase {}
class eAI_SurvivorM_Rolf extends eAIBase {}
class eAI_SurvivorM_Seth extends eAIBase {}
class eAI_SurvivorM_Taiki extends eAIBase {}
class eAI_SurvivorF_Linda extends eAIBase {}
class eAI_SurvivorF_Maria extends eAIBase {}
class eAI_SurvivorF_Frida extends eAIBase {}
class eAI_SurvivorF_Gabi extends eAIBase {}
class eAI_SurvivorF_Helga extends eAIBase {}
class eAI_SurvivorF_Irena extends eAIBase {}
class eAI_SurvivorF_Judy extends eAIBase {}
class eAI_SurvivorF_Keiko extends eAIBase {}
class eAI_SurvivorF_Lina extends eAIBase {}
class eAI_SurvivorF_Naomi extends eAIBase {}