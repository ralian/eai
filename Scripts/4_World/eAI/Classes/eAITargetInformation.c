typedef Param4<eAIGroup, int, int, autoptr array<eAIBase>> eAITargetGroup;

class eAITargetInformation
{
    private EntityAI m_Target;

    // in most circumstances an entity should only be in 1 group
    private autoptr map<int, ref eAITargetGroup> m_Groups;

    void eAITargetInformation(EntityAI target)
    {
        m_Target = target;
        m_Groups = new map<int, ref eAITargetGroup>();
    }

    EntityAI GetEntity()
    {
        return m_Target;
    }

    void Process(int group_id)
    {
        eAITargetGroup params;
        if (!m_Groups.Find(group_id, params))
        {
            Error("eAITargetInformation::Process called when target is not in group specified | m_Target=" + Object.GetDebugName(m_Target) + " group_id=" + group_id)
            return;
        }

        if (params.param2 + params.param3 <= GetGame().GetTime())
        {
            Remove(group_id);
        }
    }
    
    eAITargetGroup Insert(eAIGroup group, int max_time = 10000)
    {
        eAITargetGroup params;
        params = new eAITargetGroup(group, GetGame().GetTime(), max_time, new array<eAIBase>());
        if (!m_Groups.Insert(group.GetID(), params)) return null;

        group.OnTargetAdded(this);

        return params;
    }

    eAITargetGroup Insert(eAIBase ai, int max_time = 10000)
    {
        eAITargetGroup params;
        params = Insert(ai.GetGroup(), max_time);
        params.param4.Insert(ai);
        return params;
    }

    bool IsTargetted()
    {
        return m_Groups.Count() > 0;
    }

    bool IsTargetted(int group_id)
    {
        return m_Groups.Contains(group_id);
    }

    bool IsTargetted(eAIGroup group)
    {
        return m_Groups.Contains(group.GetID());
    }

    bool IsTargettedBy(eAIBase ai)
    {
        eAITargetGroup params;
        if (!m_Groups.Find(ai.GetGroup().GetID(), params)) return false;

        return params.param4.Find(ai) != -1;
    }

    void Update(eAIGroup group, int max_time = -2147483647 /*-int.MIN*/)
    {
        eAITargetGroup params;
        if (!m_Groups.Find(group.GetID(), params)) return;

        params.param2 = GetGame().GetTime();
        if (max_time != int.MIN) params.param3 = max_time;
    }

    void Remove(int group_id)
    {
        eAITargetGroup params;
        if (!m_Groups.Find(group_id, params)) return;

        params.param1.OnTargetRemoved(this);

        m_Groups.Remove(group_id);
    }

    void Remove(eAIGroup group)
    {
        int group_id = group.GetID();
        
        if (m_Groups.Contains(group_id))
        {
            group.OnTargetRemoved(this);

            m_Groups.Remove(group_id);
        }
    }

    void OnDeath()
    {
        // the target died, notify FSM to find a new branch and take new action
        foreach (int id, eAITargetGroup info : m_Groups) foreach (eAIBase ai : info.param4) ai.GetFSM().Start("TargetDeath");
    }

    void OnHit()
    {
        // the target was hit, notify FSM to find a new branch and take new action
        foreach (int id, eAITargetGroup info : m_Groups) foreach (eAIBase ai : info.param4) ai.GetFSM().Start("TargetHit");
    }

    // this is really bad but unfortunately we can't reasonably mod the EntityAI class
    // and for some circumstances using templates just won't work.
    //
    // wherever possible, please use 
    //    'eAIEntity<DayZPlayerImplement>.GetTargetInformation(GetGame().GetPlayer())`
    static eAITargetInformation GetTargetInformation(EntityAI entity)
    {
        DayZPlayerImplement player;
        if (Class.CastTo(player, entity)) return player.GetTargetInformation();

        ZombieBase zombie;
        if (Class.CastTo(zombie, entity)) return zombie.GetTargetInformation();
        
        AnimalBase animal;
        if (Class.CastTo(animal, entity)) return animal.GetTargetInformation();

        ItemBase item;
        if (Class.CastTo(item, entity)) return item.GetTargetInformation();

        CarScript car;
        if (Class.CastTo(car, entity)) return car.GetTargetInformation();

        return null;
    }
};

class eAIEntity<Class T>
{
    static eAITargetInformation GetTargetInformation(T entity)
    {
        return entity.GetTargetInformation();
    }
};