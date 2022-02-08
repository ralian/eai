modded class CarScript
{
    private autoptr eAITargetInformation m_TargetInformation;

    void CarScript()
    {
        m_TargetInformation = CreateTargetInformation();

		//SetEventMask(EntityEvent.ALL);
    }

    protected eAITargetInformation CreateTargetInformation()
    {
		//auto trace = CF_Trace_0(this, "CreateTargetInformation");
        return new eAIEntityTargetInformation(this);
    }

    eAITargetInformation GetTargetInformation()
    {
		//auto trace = CF_Trace_0(this, "GetTargetInformation");
        return m_TargetInformation;
    }

	override void EEKilled(Object killer)
	{
        m_TargetInformation.OnDeath();

        super.EEKilled(killer);
    }

	override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
        m_TargetInformation.OnHit();

		super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
    }
/*
    override void EOnSimulate(IEntity other, float dt)
	{
		//super.EOnSimulate(other, dt);
		
        Control(dt);
	}

	override void EOnPostSimulate(IEntity other, float timeSlice)
	{
		super.EOnPostSimulate(other, timeSlice);
		
        Control(timeSlice);
	}

	override void EOnFrame(IEntity other, float timeSlice) //!EntityEvent.FRAME
	{
        Control(timeSlice);
	}

	override void EOnPostFrame(IEntity other, int extra) //!EntityEvent.POSTFRAME
	{
        Control(0.025);
	}

	override void EOnWorldProcess(IEntity other, int extra) //!EntityEvent.WORLDPROCESS
	{
        Control(0.025);
	}

	override void EOnUser0(IEntity other, int extra) //!EntityEvent.EV_USER+0
	{
        Control(0.025);
	}
	override void EOnUser1(IEntity other, int extra) //!EntityEvent.EV_USER+1
	{
        Control(0.025);
	}
	override void EOnUser2(IEntity other, int extra) //!EntityEvent.EV_USER+2
	{
        Control(0.025);
	}
	override void EOnUser3(IEntity other, int extra) //!EntityEvent.EV_USER+3
	{
        Control(0.025);
	}
	override void EOnUser4(IEntity other, int extra) //!EntityEvent.EV_USER+4
	{
        Control(0.025);
	}

	override void OnUpdate(float dt)
    {
        super.OnUpdate(dt);
		
        Control(dt);
    }

    void Control(float pDt)
    {
        CarController controller = GetController();
        controller.SetSteering(1.0, false);
        controller.SetThrust(1.0, 0.0, 1.0);
        controller.SetBrake(0.0);
        controller.ShiftTo(CarGear.FIRST);
    }
*/
};