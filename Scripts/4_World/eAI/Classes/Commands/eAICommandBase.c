class eAICommandBase extends HumanCommandScript
{
	protected eAIBase m_Unit;
	protected eAIAnimationST m_Table;

	void eAICommandBase(eAIBase unit, eAIAnimationST st)
	{
		m_Unit = unit;
		m_Table = st;
	}

	void SetLookDirection(vector direction)	{ }

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