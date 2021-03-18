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

modded class DayZPlayerImplementHeading
{

	//-------------------------------------------------------------
	//!
	//! This HeadingModel - Clamps heading
	//! 

	//!
	static bool	ClampHeading (float pDt, SDayZPlayerHeadingModel pModel, out float pLastHeadingDiff)
	{
		float		aDiff = pModel.m_fHeadingAngle - pModel.m_fOrientationAngle;
		if (aDiff < -Math.PI)
		{
			aDiff += Math.PI2;
		}
		else if (aDiff > Math.PI)
		{
			aDiff -= Math.PI2;
		}

		// Print("Heading model: or: " + pModel.m_fOrientationAngle.ToString() + " head:" + pModel.m_fHeadingAngle.ToString() + " dif:" + aDiff.ToString());

		if (pLastHeadingDiff < -Math.PI_HALF && aDiff > 0)
		{
			aDiff					= -Math.PI + 0.01;
			pLastHeadingDiff		= aDiff;
			pModel.m_fHeadingAngle 	= pModel.m_fOrientationAngle + aDiff;
		
        	// Print("-APA- : or: " + pModel.m_fOrientationAngle.ToString() + " head:" + pModel.m_fHeadingAngle.ToString() + " dif:" + aDiff.ToString());

			return true;		// modify heading
		}
		else if (pLastHeadingDiff > Math.PI_HALF && aDiff < 0)
		{
			aDiff 					= Math.PI - 0.01;
			pLastHeadingDiff		= aDiff;
			pModel.m_fHeadingAngle 	= pModel.m_fOrientationAngle + aDiff;

        	// Print("-APA- : or: " + pModel.m_fOrientationAngle.ToString() + " head:" + pModel.m_fHeadingAngle.ToString() + " dif:" + aDiff.ToString());

			return true;		// modify heading
		}

		pLastHeadingDiff	= aDiff;
		// Print("Heading model diff " + aDiff.ToString());
		return false;
	}

	//-------------------------------------------------------------
	//!
	//! This HeadingModel - Rotates orientations - so player slides 
	//! 

	static float 	CONST_ROTLIMIT = Math.PI * 0.95;

	//!
	static bool	RotateOrient (float pDt, SDayZPlayerHeadingModel pModel, out float pLastHeadingDiff)
	{
		float		aDiff = pModel.m_fHeadingAngle - pModel.m_fOrientationAngle;

		if (aDiff < -Math.PI)
		{
			aDiff += Math.PI2;
		}
		else if (aDiff > Math.PI)
		{
			aDiff -= Math.PI2;
		}

		// Print("Heading model: or: " + pModel.m_fOrientationAngle.ToString() + " head:" + pModel.m_fHeadingAngle.ToString() + " dif:" + aDiff.ToString());

		if (pLastHeadingDiff < -Math.PI_HALF && aDiff > 0)
		{
			aDiff -= Math.PI2;
		}
		else if (pLastHeadingDiff > Math.PI_HALF && aDiff < 0)
		{
			aDiff += Math.PI2;
		}

		pLastHeadingDiff	= aDiff;		

		if (aDiff < -CONST_ROTLIMIT)
		{
			pModel.m_fOrientationAngle += aDiff +  CONST_ROTLIMIT;
			return true;
		}
		else if (aDiff > CONST_ROTLIMIT)
		{
			pModel.m_fOrientationAngle += aDiff - CONST_ROTLIMIT;
			return true;
		}

		// Print("Heading model diff " + aDiff.ToString());
		return false;

	}
	
	static bool NoHeading(float pDt, SDayZPlayerHeadingModel pModel, out float pLastHeadingDiff)
	{
		pLastHeadingDiff = 0;
		pModel.m_fHeadingAngle = pModel.m_fOrientationAngle;
		return true;
	}



}

