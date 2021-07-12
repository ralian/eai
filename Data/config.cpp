class CfgPatches
{
	class eAIShirt
	{
		units[]=
		{};
		weapons[]={};
		requiredVersion=0.1;
		requiredAddons[]=
		{
			"DZ_Characters_Tops",
			"DZ_Data"
		};
	};	
};

class CfgVehicles
{
	class TShirt_ColorBase;
	class eAIShirt : TShirt_ColorBase
	{
		scope=2;
		displayName="eAI Credits Shirt";
		descriptionShort="eAI Credits Shirt";
		hiddenSelectionsTextures[]=
		{
			"eAI\Data\eAIShirt.paa",
			"eAI\Data\eAIShirt.paa",
			"eAI\Data\eAIShirt.paa"
		};
	};
};