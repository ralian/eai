enum eAICommands
{
	// Movement
	MOV_STOP,
	MOV_GOTO,
	MOV_RTF, // Return to formation
	MOV_GETIN,
	// Formation 
	FOR_VEE,
	FOR_FILE,
	FOR_WALL,
	FOR_COL,
	// Status
	STA_SITREP, // report health and ammo level
	STA_POSITION, // report position
	STA_THREATS,
	STA_UNUSED,
	// Debug
	DEB_SPAWNALLY,
	DEB_SPAWNZOM,
	DEB_CLEARALL,
	DEB_RELOAD,
	DEB_AIMAP
}