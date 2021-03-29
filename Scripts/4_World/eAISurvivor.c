string SurvivorRandom()
{
	int FemaleChance = 50;
	if (Math.RandomInt(0, 100) > FemaleChance)
	{
		return SurvivorRandomFemale();
	}
	else
	{
		return SurvivorRandomMale();
	}
}

string SurvivorRandomFemale()
{
	TStringArray FemaleList = { "SurvivorF_Eva", "SurvivorF_Frida", "SurvivorF_Gabi", "SurvivorF_Helga", 
								"SurvivorF_Irena", "SurvivorF_Judy", "SurvivorF_Keiko", "SurvivorF_Linda", 
								"SurvivorF_Maria", "SurvivorF_Naomi"
							  };
	return FemaleList.GetRandomElement();
}

string SurvivorRandomMale()
{
	TStringArray MaleList = { "SurvivorM_Boris", "SurvivorM_Cyril", "SurvivorM_Denis", "SurvivorM_Elias", 
							  "SurvivorM_Francis", "SurvivorM_Guo", "SurvivorM_Hassan", "SurvivorM_Indar", 
							  "SurvivorM_Jose", "SurvivorM_Kaito", "SurvivorM_Lewis", "SurvivorM_Manua", 
							  "SurvivorM_Mirek", "SurvivorM_Niki", "SurvivorM_Oliver", "SurvivorM_Peter", 
							  "SurvivorM_Quinn", "SurvivorM_Rolf", "SurvivorM_Seth", "SurvivorM_Taiki"
							};
	return MaleList.GetRandomElement();
}
