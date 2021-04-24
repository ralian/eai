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
		picture = "eAI/gfx/eAI_logo.edds";
		action = "";
		hideName = 0;
		hidePicture = 0;
		name = "Enfusion AI System";
		credits = "Ralian";
		creditsJson = "eAI/Scripts/Data/Credits.json";
		author = "Ralian";
		authorID = "0"; 
		version = "0.1"; 
		extra = 0;
		type = "mod";
		inputs = "eAI/Scripts/inputs.xml";
		
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