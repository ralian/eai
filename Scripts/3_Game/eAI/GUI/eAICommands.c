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
	#ifndef EAI_COMMAND_DEBUG_DISABLE
	DEB_SPAWNALLY,
	DEB_SPAWNZOM,
	DEB_CLEARALL,
	DEB_AIMAP, // TODO
	DEB_GRPMGR, // TODO
	#endif
}