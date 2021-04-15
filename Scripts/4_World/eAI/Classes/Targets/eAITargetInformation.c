/**
 * @class eAITargetInformation
 * 
 * @brief Is an abstract class
 */
class eAITargetInformation
{
    // in most circumstances an entity should only be in 1 group
    private autoptr map<int, ref eAITarget> m_Groups;

    void eAITargetInformation()
    {
        m_Groups = new map<int, ref eAITarget>();
    }

    void ~eAITargetInformation()
    {
        foreach (int id, eAITarget info : m_Groups) Remove(id);
    }

	/**
	 * @brief Get the entity that this target belongs to, if it does so
	 *
	 * @return EntityAI
	 */
    EntityAI GetEntity()
    {
        return null;
    }

	/**
	 * @brief Abstract function. Get the position for the AI within the target. Each AI could have their own position for the target. 
	 *
     * @param ai null default, gets the position for the AI if specified, otherwise returns a default value
	 * @return EntityAI
	 */
    vector GetPosition(eAIBase ai = null)
    {
        return "0 0 0";
    }

	/**
	 * @brief Processes the group
	 *
     * @param group_id group id of an eAIGroup
	 */
    void Process(int group_id)
    {
        eAITarget target;
        if (!m_Groups.Find(group_id, target))
        {
            Error("eAITargetInformation::Process called when target is not in group specified | group_id=" + group_id);
            return;
        }

        if (target.found_at_time + target.max_time <= GetGame().GetTime())
        {
            Remove(group_id);
        }
    }

	/**
	 * @brief Refreshes the target time for the group 
	 *
     * @param group the eAIGroup
     * @param max_time the new maximum time it should be targetting for, if int.MIN then not updated
     */
    void Update(eAIGroup group, int max_time = -2147483647 /*-int.MIN*/)
    {
        eAITarget target;
        if (!m_Groups.Find(group.GetID(), target)) return;

        target.found_at_time = GetGame().GetTime();
        if (max_time != int.MIN) target.max_time = max_time;
    }
    
	/**
	 * @brief Inserts the group into the target
	 *
     * @param group_id group id of an eAIGroup
	 */
    eAITarget Insert(eAIGroup group, int max_time = -2147483647 /*-int.MIN*/)
    {
        eAITarget target;
        target = new eAITarget(group, GetGame().GetTime(), max_time, this);
        if (!m_Groups.Insert(group.GetID(), target)) return null;

        group.OnTargetAdded(this);

        return target;
    }

	/**
	 * @brief Tells the target that the AI is targeting it, inserting the group 
	 *
     * @param ai eAIBase object
     * @param max_time time the eAIBase will be targetting this target for
	 */
    eAITarget Insert(eAIBase ai, int max_time = -2147483647 /*-int.MIN*/)
    {
        eAITarget target;
        target = Insert(ai.GetGroup(), max_time);
        if (!target) return null; 
        target.AddAI(ai);
        ai.OnAddTarget(target);
        return target;
    }

	/**
	 * @brief Tells the target that the AI is targetting it. If the group didn't know about the target, insert the group
	 *
     * @param ai eAIBase object
     * @param max_time time the eAIBase will be targetting this target for, if int.MIN then the time of the group isn't updated
	 */
    eAITarget AddAI(eAIBase ai, int max_time = -2147483647 /*-int.MIN*/)
    {
        eAITarget target;
        int group_id = ai.GetGroup().GetID();
        if (!m_Groups.Find(group_id, target)) return Insert(ai, max_time);

        if (max_time != int.MIN) target.max_time = max_time;

        target.AddAI(ai);
        ai.OnAddTarget(target);

        return target;
    }

	/**
	 * @brief Tells the target that the AI is no longer targetting it
	 *
     * @param ai eAIBase object
     */
    bool RemoveAI(eAIBase ai)
    {
        eAITarget target;
        int group_id = ai.GetGroup().GetID();
        if (!m_Groups.Find(group_id, target)) return false;

        if (!target.RemoveAI(ai)) return false;

        ai.OnRemoveTarget(target);

        if (target.CountAI() == 0)
        {
            target.group.OnTargetRemoved(this);

            m_Groups.Remove(group_id);
        }

        return true;
    }

	/**
	 * @brief Tells the target that the group and it's AI is no longer targetting it
	 *
     * @param group_id the ID of the eAIGroup
     */
    void Remove(int group_id)
    {
        eAITarget target;
        if (!m_Groups.Find(group_id, target)) return;
		
		if (target.group && target.ai_list)
		{
	        target.group.OnTargetRemoved(this);
	
	        foreach (eAIBase ai : target.ai_list)
				if (ai)	ai.OnRemoveTarget(target);
		}

        m_Groups.Remove(group_id);
        delete target;
    }

	/**
	 * @brief Tells the target that the group and it's AI is no longer targetting it
	 *
     * @param group the eAIGroup
     */
    void Remove(eAIGroup group)
    {
        int group_id = group.GetID();
        
        Remove(group_id);
    }

	/**
	 * @brief Checks to see if any group/ai is currently targetting this target
	 *
     * @return bool true if being targetted, false otherwise
     */
    bool IsTargetted()
    {
        return m_Groups.Count() > 0;
    }

	/**
	 * @brief Checks to see if the group specified is currently targetting this target
	 *
     * @param group_id the group id of the eAIGroup
     * @return bool true if being targetted, false otherwise
     */
    bool IsTargetted(int group_id)
    {
        return m_Groups.Contains(group_id);
    }

	/**
	 * @brief Checks to see if the group specified is currently targetting this target
	 *
     * @param group the eAIGroup
     * @return bool true if being targetted, false otherwise
     */
    bool IsTargetted(eAIGroup group)
    {
        return m_Groups.Contains(group.GetID());
    }

	/**
	 * @brief Checks to see if the AI specified is currently targetting this target
	 *
     * @param group the eAIGroup
     * @return bool true if being targetted, false otherwise
     */
    bool IsTargettedBy(eAIBase ai)
    {
        eAITarget target;
        if (!m_Groups.Find(ai.GetGroup().GetID(), target)) return false;

        return target.FindAI(ai) != -1;
    }

    //! entity specific implementations for abstracted call in eAIEntityTargetInformation
    void OnDeath()
    {
        // the target died, notify FSM to find a new branch and take new action
        foreach (int id, eAITarget target : m_Groups) foreach (eAIBase ai : target.ai_list) ai.GetFSM().Start("TargetDeath");
    }

    void OnHit()
    {
        // the target was hit, notify FSM to find a new branch and take new action
        foreach (int id, eAITarget target : m_Groups) foreach (eAIBase ai : target.ai_list) ai.GetFSM().Start("TargetHit");
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