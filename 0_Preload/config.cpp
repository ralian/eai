class CfgPatches
{
	class eAI_Preload
	{
		units[]={};
		weapons[]={};
		requiredVersion=0.1;
		requiredAddons[]=
		{
		};
	};
};

class CfgMods
{
	class DZ_eAI_Preload
	{
		type = "mod";

		dependencies[]= 
		{
			"Game",
			"World",
			"Mission"
		};

		class defs
		{
			class engineScriptModule
			{
				files[] =
				{
					"0_eAI_Preload/Common",
					"eAI/0_Preload/Common",
					"0_eAI_Preload/1_Core",
					"eAI/0_Preload/1_Core"
				};
			};
			class gameLibScriptModule
			{
				files[] =
				{
					"0_eAI_Preload/Common",
					"eAI/0_Preload/Common",
					"0_eAI_Preload/2_GameLib",
					"eAI/0_Preload/2_GameLib"
				};
			};
			class gameScriptModule
			{
				files[] =
				{
					"0_eAI_Preload/Common",
					"eAI/0_Preload/Common",
					"0_eAI_Preload/3_Game",
					"eAI/0_Preload/3_Game"
				};
			};
			class worldScriptModule
			{
				files[] =
				{
					"0_eAI_Preload/Common",
					"eAI/0_Preload/Common",
					"0_eAI_Preload/4_World",
					"eAI/0_Preload/4_World"
				};
			};
			class missionScriptModule
			{
				files[] =
				{
					"0_eAI_Preload/Common",
					"eAI/0_Preload/Common",
					"0_eAI_Preload/5_Mission",
					"eAI/0_Preload/5_Mission"
				};
			};
		};
	};
};
