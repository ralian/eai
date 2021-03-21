class eAICommandBase extends HumanCommandScript
{
	//! Helper functions
	void AnglesToQuat(vector angles, out float[] quat)
	{
		vector rotationMatrix[3];
		Math3D.YawPitchRollMatrix(angles, rotationMatrix);
		Math3D.MatrixToQuat(rotationMatrix, quat);
	}
	
	void PrePhys_SetAngles(vector angles)
	{
		float rotation[4];
		AnglesToQuat(angles, rotation);
		PrePhys_SetRotation(rotation);
	}
};