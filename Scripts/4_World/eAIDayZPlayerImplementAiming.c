modded class DayZPlayerImplementAiming {
	
	float aimX, aimY;
	
	bool ProcessAimFilters(float pDt, SDayZPlayerAimingModel pModel, int stance_index) {
		bool returnVal = super.ProcessAimFilters(pDt, pModel, stance_index);
		aimX = pModel.m_fCurrentAimX;
		aimY = pModel.m_fCurrentAimY;
		return returnVal;
	}
	
	float getAimX() {return aimX;}
	
	float getAimY() {return aimY;}
	
}