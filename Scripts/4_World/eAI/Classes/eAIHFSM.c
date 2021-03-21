class eAIHFSM
{
    private autoptr array<ref eAIState> m_States;
    private eAIState m_CurrentState;
    private eAIState m_ParentState;

    private string m_Name;
    private eAIBase m_Unit;

    void eAIHFSM(ref eAIState parentState)
    {
        m_ParentState = parentState;
        m_States = new array<ref eAIState>();
    }

    string GetName()
    {
        return m_Name;
    }

    void LoadXML(string path)
    {
        ref CF_XML_Document document;
        CF_XML.ReadDocument(path, document);

        m_Name = document.Get("hfsm")[0].GetAttribute("name").ValueAsString();

        auto states = document.Get("hfsm");
        states = states[0].GetTag("states");
        states = states[0].GetTag("state");

	    foreach (auto state : states) m_States.Insert(eAIState.LoadXML(this, state));
    }
};