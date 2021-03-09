modded class DayZPlayerImplementAiming {
	
	float aimX, aimY;
	
	// Same as original method, but we need to store the aim x and y values for calculating trajectory
	override bool ProcessAimFilters(float pDt, SDayZPlayerAimingModel pModel, int stance_index) {
		bool returnVal = super.ProcessAimFilters(pDt, pModel, stance_index);
		aimX = pModel.m_fCurrentAimX;
		aimY = pModel.m_fCurrentAimY;
		return returnVal;
	}
	
	// Spawns a new recoil object with no actual recoil
	void SetDummyRecoil( Weapon_Base weapon )
	{
		if( m_ProceduralRecoilEnabled )
		{
			m_CurrentRecoil = RecoilBase.Cast(new eAIDummyRecoil(weapon));
		}
	}
	
	float getAimX() {return aimX;}
	
	float getAimY() {return aimY;}
	
}