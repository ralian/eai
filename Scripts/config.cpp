class CfgPatches
{
    class eAI_Scripts
    {
        units[]={};
        weapons[]={};
        requiredVersion=0.1;
        requiredAddons[]=
        {
            "DZ_Data", "JM_CF_Scripts"
        };
    };
};

class CfgMods
{
	class eAI
	{
		dir = "eAI";
		picture = "eAI\Data\eAI_logo.edds";
		logo = "eAI\Data\eAI_logo.edds";
		logoSmall = "eAI\Data\eAI_logo.edds";
		logoOver = "eAI\Data\eAI_logo.edds";
		action = "https://discord.gg/jQffkP76cD";
		hideName = 0;
		hidePicture = 0;
		name = "Enfusion AI System";
		credits = "Ralian";
		creditsJson = "eAI\Scripts\Data\Credits.json";
		author = "Ralian";
		authorID = "0"; 
		version = "0.1"; 
		extra = 0;
		type = "mod";
		inputs = "eAI\Scripts\inputs.xml";
		
		dependencies[] = { "Game", "World", "Mission" };
		
		class defs
		{
			class gameScriptModule
			{
				value = "";
				files[] = {"eAI/Scripts/3_Game"};
			};
			class worldScriptModule
			{
				value = "";
				files[] = {"eAI/Scripts/4_World"};
			};
			class missionScriptModule
			{
				value = "";
				files[] = {"eAI/Scripts/5_Mission"};
			};
		};
	};
};

class CfgVehicles
{
	
	class SurvivorM_Mirek;
	class SurvivorM_Denis;
	class SurvivorM_Boris;
	class SurvivorM_Cyril;
	class SurvivorM_Elias;
	class SurvivorM_Francis;
	class SurvivorM_Guo;
	class SurvivorM_Hassan;
	class SurvivorM_Indar;
	class SurvivorM_Jose;
	class SurvivorM_Kaito;
	class SurvivorM_Lewis;
	class SurvivorM_Manua;
	class SurvivorM_Niki;
	class SurvivorM_Oliver;
	class SurvivorM_Peter;
	class SurvivorM_Quinn;
	class SurvivorM_Rolf;
	class SurvivorM_Seth;
	class SurvivorM_Taiki;
	class SurvivorF_Linda;
	class SurvivorF_Maria;
	class SurvivorF_Frida;
	class SurvivorF_Gabi;
	class SurvivorF_Helga;
	class SurvivorF_Irena;
	class SurvivorF_Judy;
	class SurvivorF_Keiko;
	class SurvivorF_Lina;
	class SurvivorF_Naomi;
	class eAI_SurvivorM_Mirek: SurvivorM_Mirek
	{
		scope = 2;
	};

	class eAI_SurvivorM_Denis: SurvivorM_Denis
	{
		scope = 2;
	};
	
	class eAI_SurvivorM_Boris: SurvivorM_Boris
	{
		scope = 2;
	};

	class eAI_SurvivorM_Cyril: SurvivorM_Cyril
	{
		scope = 2;
	};

	class eAI_SurvivorM_Elias: SurvivorM_Elias
	{
		scope = 2;
	};

	class eAI_SurvivorM_Francis: SurvivorM_Francis
	{
		scope = 2;
	};

	class eAI_SurvivorM_Guo: SurvivorM_Guo
	{
		scope = 2;
	};

	class eAI_SurvivorM_Hassan: SurvivorM_Hassan
	{
		scope = 2;
	};

	class eAI_SurvivorM_Indar: SurvivorM_Indar
	{
		scope = 2;
	};

	class eAI_SurvivorM_Jose: SurvivorM_Jose
	{
		scope = 2;
	};

	class eAI_SurvivorM_Kaito: SurvivorM_Kaito
	{
		scope = 2;
	};

	class eAI_SurvivorM_Lewis: SurvivorM_Lewis
	{
		scope = 2;
	};

	class eAI_SurvivorM_Manua: SurvivorM_Manua
	{
		scope = 2;
	};

	class eAI_SurvivorM_Niki: SurvivorM_Niki
	{
		scope = 2;
	};

	class eAI_SurvivorM_Oliver: SurvivorM_Oliver
	{
		scope = 2;
	};

	class eAI_SurvivorM_Peter: SurvivorM_Peter
	{
		scope = 2;
	};

	class eAI_SurvivorM_Quinn: SurvivorM_Quinn
	{
		scope = 2;
	};

	class eAI_SurvivorM_Rolf: SurvivorM_Rolf
	{
		scope = 2;
	};

	class eAI_SurvivorM_Seth: SurvivorM_Seth
	{
		scope = 2;
	};

	class eAI_SurvivorM_Taiki: SurvivorM_Taiki
	{
		scope = 2;
	};
	
	class eAI_SurvivorF_Linda: SurvivorF_Linda
	{
		scope = 2;
	};
	
	class eAI_SurvivorF_Maria: SurvivorF_Maria
	{
		scope = 2;
	};

	class eAI_SurvivorF_Frida: SurvivorF_Frida
	{
		scope = 2;
	};

	class eAI_SurvivorF_Gabi: SurvivorF_Gabi
	{
		scope = 2;
	};

	class eAI_SurvivorF_Helga: SurvivorF_Helga
	{
		scope = 2;
	};

	class eAI_SurvivorF_Irena: SurvivorF_Irena
	{
		scope = 2;
	};

	class eAI_SurvivorF_Judy: SurvivorF_Judy
	{
		scope = 2;
	};

	class eAI_SurvivorF_Keiko: SurvivorF_Keiko
	{
		scope = 2;
	};

	class eAI_SurvivorF_Lina: SurvivorF_Lina
	{
		scope = 2;
	};

	class eAI_SurvivorF_Naomi: SurvivorF_Naomi
	{
		scope = 2;
	};
};