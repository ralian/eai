//typedef Param5<eAIGroup, int, int, ref set<eAIBase>, eAITargetInformation> eAITargetGroup;


/*
First one is the group the target was found in
Second is the time the target was found in the group
Third is the max time the target will be in the group for before auto removal
Fourth is the AI in the group that found the target. Usually 1, can sometimes be more than 1
5th is a reference to the actual target info for a look up optimization
*/
class eAITargetGroup
{
    eAIGroup param1;
    int param2;
    int param3;
    autoptr set<eAIBase> param4;
    eAITargetInformation param5;

    void eAITargetGroup(eAIGroup _param1, int _param2, int _param3, set<eAIBase> _param4, eAITargetInformation _param5)
    {
        param1 = _param1;
        param2 = _param2;
        param3 = _param3;
        param4 = _param4;
        param5 = _param5;
    }
};

class eAITargetInformation
{
    // in most circumstances an entity should only be in 1 group
    private autoptr map<int, ref eAITargetGroup> m_Groups;

    void eAITargetInformation()
    {
        m_Groups = new map<int, ref eAITargetGroup>();
    }

    void ~eAITargetInformation()
    {
        foreach (int id, eAITargetGroup info : m_Groups) Remove(id);
    }

    EntityAI GetEntity()
    {
        return null;
    }

    vector GetPosition(eAIBase ai = null)
    {
        return "0 0 0";
    }

    void Process(int group_id)
    {
        eAITargetGroup params;
        if (!m_Groups.Find(group_id, params))
        {
            Error("eAITargetInformation::Process called when target is not in group specified | group_id=" + group_id);
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
        params = new eAITargetGroup(group, GetGame().GetTime(), max_time, new set<eAIBase>(), this);
        if (!m_Groups.Insert(group.GetID(), params)) return null;

        group.OnTargetAdded(this);

        return params;
    }

    eAITargetGroup Insert(eAIBase ai, int max_time = 10000)
    {
        eAITargetGroup params;
        params = Insert(ai.GetGroup(), max_time);
        if (!params) return null; 
        params.param4.Insert(ai);
        ai.OnAddTarget(params);
        return params;
    }

    eAITargetGroup AddAI(eAIBase ai)
    {
        eAITargetGroup params;
        int group_id = ai.GetGroup().GetID();
        if (!m_Groups.Find(group_id, params)) return Insert(ai);

        params.param4.Insert(ai);
        ai.OnAddTarget(params);

        return params;
    }

    bool RemoveAI(eAIBase ai)
    {
        eAITargetGroup params;
        int group_id = ai.GetGroup().GetID();
        if (!m_Groups.Find(group_id, params)) return false;

        int idx = params.param4.Find(ai);
        if (idx == -1) return false;

        params.param4.Remove(idx);
        ai.OnRemoveTarget(params);

        if (params.param4.Count() == 0)
        {
            params.param1.OnTargetRemoved(this);

            m_Groups.Remove(group_id);
        }

        return true;
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
		
		if (params.param1 && params.param4) {
	        params.param1.OnTargetRemoved(this);
	
	        foreach (eAIBase ai : params.param4)
				if (ai)
					ai.OnRemoveTarget(params);
		}

        m_Groups.Remove(group_id);
        delete params;
    }

    void Remove(eAIGroup group)
    {
        int group_id = group.GetID();
        
        Remove(group_id);
    }

    //! entity specific implementations

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