class eAIHFSMType
{
    private static ref map<string, eAIHFSMType> m_Types = new map<string, eAIHFSMType>();

	string m_ClassName;
    ScriptModule m_Module;

    autoptr array<ref eAIStateType> m_States;
    autoptr array<ref eAITransitionType> m_Transitions;

    void eAIHFSMType()
    {
        m_States = new array<ref eAIStateType>();
        m_Transitions = new array<ref eAITransitionType>();
    }

    static bool Contains(string name)
    {
        return m_Types.Contains(name);
    }

    static void Add(eAIHFSMType type)
    {
        m_Types.Insert(type.m_ClassName, type);
    }

    static eAIHFSMType Get(string type)
    {
        return m_Types[type];
    }

    eAIHFSM Spawn(eAIBase unit, eAIState parentState)
    {
        eAIHFSM retValue = null;
        m_Module.CallFunctionParams(null, "Create_" + m_ClassName, retValue, new Param2<eAIBase, eAIState>(unit, parentState));
        return retValue;
    }

    static eAIHFSM Spawn(string type, eAIBase unit, eAIState parentState)
    {
        return m_Types[type].Spawn(unit, parentState);
    }
};

class eAIHFSM
{
    private autoptr array<ref eAIState> m_States;
    private autoptr array<ref eAITransition> m_Transitions;

    private eAIState m_CurrentState;
    private eAIState m_ParentState;

    protected string m_Name;
    private eAIBase m_Unit;

    void eAIHFSM(eAIBase unit, eAIState parentState)
    {
        m_Unit = unit;
        m_ParentState = parentState;

        m_States = new array<ref eAIState>();
        m_Transitions = new array<ref eAITransition>();
    }

    string GetName()
    {
        return m_Name;
    }

    eAIBase GetUnit()
    {
        return m_Unit;
    }

    void AddState(eAIState state)
    {
        m_States.Insert(state);
    }

    void AddTransition(eAITransition transition)
    {
        m_Transitions.Insert(transition);
    }

    eAIState GetState(string type)
    {
        for (int i = 0; i < m_States.Count(); i++) if (m_States[i].ClassName() == type) return m_States[i];

        return null;
    }

    eAIState GetState(typename type)
    {
        for (int i = 0; i < m_States.Count(); i++) if (m_States[i].Type() == type) return m_States[i];

        return null;
    }

    static eAIHFSMType LoadXML(string path)
    {
        CF_XML_Document document;
        CF_XML.ReadDocument(path, document);
		
        string name = document.Get("hfsm")[0].GetAttribute("name").ValueAsString();
        string class_name = "eAI_" + name + "_HFSM";

        if (eAIHFSMType.Contains(class_name)) return eAIHFSMType.Get(class_name);

        eAIHFSMType new_type = new eAIHFSMType();
		new_type.m_ClassName = class_name;

        MakeDirectory("$profile:eAI/");
        string script_path = "$profile:eAI/" + class_name + ".c";
        FileHandle file = OpenFile(script_path, FileMode.WRITE);
        if (!file) return null;
		
        FPrintln(file, "class " + class_name + " extends eAIHFSM {");

        FPrintln(file, "void " + class_name + "(eAIBase unit, eAIState parentState) {");
        FPrintln(file, "m_Name = \"" + name + "\";");
        FPrintln(file, "}");

        FPrintln(file, "void Setup() {");

        auto states = document.Get("hfsm");
        states = states[0].GetTag("states");
        states = states[0].GetTag("state");

        ScriptModule module = GetGame().GetMission().MissionScript;

	    foreach (auto state : states)
        {
            eAIStateType stateType = eAIState.LoadXML(name, state, module);
            if (stateType.m_Module != null)
            {
                module = stateType.m_Module;
                new_type.m_States.Insert(stateType);
                
                FPrintln(file, "AddState(new " + stateType.m_ClassName + "(this));");
            }
        }

        auto transitions = document.Get("hfsm");
        transitions = transitions[0].GetTag("transitions");
        transitions = transitions[0].GetTag("transition");

	    foreach (auto transition : transitions)
        {
            eAITransitionType transitionType = eAITransition.LoadXML(name, transition, module);
            if (transitionType.m_Module != null)
            {
                module = transitionType.m_Module;
                new_type.m_Transitions.Insert(transitionType);

                FPrintln(file, "AddTransition(new " + transitionType.m_ClassName + "(this));");
            }
        }

        FPrintln(file, "}");

        FPrintln(file, "}");

        FPrintln(file, "eAIHFSM Create_" + class_name + "(eAIBase unit, eAIState parentState) {");
        FPrintln(file, "return new " + class_name + "(unit, parentState);");
        FPrintln(file, "}");
		
		CloseFile(file);

		eAIHFSMType.Add(new_type);
        new_type.m_Module = ScriptModule.LoadScript(module, script_path, false);
        if (new_type.m_Module == null)
        {
            Error("There was an error loading in the transition.");
            return null;
        }
		
		return new_type;
    }
};