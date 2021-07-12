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

// This class creates some default behavior when AI are created using their specific classname
class eAIBaseIntermediary extends eAIBase {
	void eAIBaseIntermediary() {
		if (GetGame().IsServer()) {
			//eAIGroup ownerGrp = eAIGroup.GetGroupByLeader(this);
			SetAI(null);
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(HumanLoadout.Apply, 500, false, this, "HumanLoadout.json");
		}
	}
}

class eAI_SurvivorM_Mirek extends eAIBaseIntermediary {}
class eAI_SurvivorM_Denis extends eAIBaseIntermediary {}
class eAI_SurvivorM_Boris extends eAIBaseIntermediary {}
class eAI_SurvivorM_Cyril extends eAIBaseIntermediary {}
class eAI_SurvivorM_Elias extends eAIBaseIntermediary {}
class eAI_SurvivorM_Francis extends eAIBaseIntermediary {}
class eAI_SurvivorM_Guo extends eAIBaseIntermediary {}
class eAI_SurvivorM_Hassan extends eAIBaseIntermediary {}
class eAI_SurvivorM_Indar extends eAIBaseIntermediary {}
class eAI_SurvivorM_Jose extends eAIBaseIntermediary {}
class eAI_SurvivorM_Kaito extends eAIBaseIntermediary {}
class eAI_SurvivorM_Lewis extends eAIBaseIntermediary {}
class eAI_SurvivorM_Manua extends eAIBaseIntermediary {}
class eAI_SurvivorM_Niki extends eAIBaseIntermediary {}
class eAI_SurvivorM_Oliver extends eAIBaseIntermediary {}
class eAI_SurvivorM_Peter extends eAIBaseIntermediary {}
class eAI_SurvivorM_Quinn extends eAIBaseIntermediary {}
class eAI_SurvivorM_Rolf extends eAIBaseIntermediary {}
class eAI_SurvivorM_Seth extends eAIBaseIntermediary {}
class eAI_SurvivorM_Taiki extends eAIBaseIntermediary {}
class eAI_SurvivorF_Linda extends eAIBaseIntermediary {}
class eAI_SurvivorF_Maria extends eAIBaseIntermediary {}
class eAI_SurvivorF_Frida extends eAIBaseIntermediary {}
class eAI_SurvivorF_Gabi extends eAIBaseIntermediary {}
class eAI_SurvivorF_Helga extends eAIBaseIntermediary {}
class eAI_SurvivorF_Irena extends eAIBaseIntermediary {}
class eAI_SurvivorF_Judy extends eAIBaseIntermediary {}
class eAI_SurvivorF_Keiko extends eAIBaseIntermediary {}
class eAI_SurvivorF_Lina extends eAIBaseIntermediary {}
class eAI_SurvivorF_Naomi extends eAIBaseIntermediary {}