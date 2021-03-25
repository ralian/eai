class eAITargetInformation
{
    private EntityAI m_Target;

    private autoptr array<ref eAIGroup> m_Groups;

    void eAITargetInformation(EntityAI target)
    {
        m_Target = target;
        m_Groups = new array<ref eAIGroup>();
    }
};

class eAIEntity<Class T>
{
    static eAITargetInformation GetTargetInformation(T entity)
    {
        return entity.GetTargetInformation();
    }
};