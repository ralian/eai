class eAIDummyRecoil : RecoilBase {
	void eAIDummyRecoil(Weapon_Base weapon) {
		m_Weapon = weapon;
		//m_DebugMode = false;
		m_DebugMode = GetDayZGame().IsAimLogEnabled();
		m_Player = PlayerBase.Cast(weapon.GetHierarchyRootPlayer());
		m_HandsCurvePoints = new array<vector>;
		Init();
		PostInit(weapon);
	}
	
	override void Init() {
		m_HandsCurvePoints.Insert("0 0 0");
		m_HandsOffsetRelativeTime = 1;
		
		m_MouseOffsetRangeMin = 50;//in degrees min
		m_MouseOffsetRangeMax = 120;//in degrees max
		m_MouseOffsetDistance = 0.0001;//how far should the mouse travel
		m_MouseOffsetRelativeTime = 0.5;//[0..1] a time it takes to move the mouse the required distance relative to the reload time of the weapon(firing mode)
	
		m_CamOffsetDistance = 0.0001;
		m_CamOffsetRelativeTime = 1;
	}
}