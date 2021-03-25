modded class AnimalBase
{
    private autoptr eAITargetInformation m_TargetInformation = new eAITargetInformation(this);

    eAITargetInformation GetTargetInformation()
    {
        return m_TargetInformation;
    }
};