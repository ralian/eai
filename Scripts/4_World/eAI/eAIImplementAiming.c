// Copyright 2021 William Bowers
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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