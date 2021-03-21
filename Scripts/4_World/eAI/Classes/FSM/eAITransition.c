class eAITransitionType
{
    private static ref map<string, eAITransitionType> m_Types = new map<string, eAITransitionType>();

	string m_ClassName;
    ScriptModule m_Module;

    void eAITransitionType()
    {
    }

    static bool Contains(string name)
    {
        return m_Types.Contains(name);
    }

    static void Add(eAITransitionType type)
    {
        m_Types.Insert(type.m_ClassName, type);
    }

    static eAITransitionType Get(string type)
    {
        return m_Types[type];
    }

    eAITransition Spawn(eAIHFSM fsm)
    {
        eAITransition retValue = null;
        m_Module.CallFunction(null, "Create_" + m_ClassName, retValue, fsm);
        return retValue;
    }

    static eAITransition Spawn(string type, eAIHFSM fsm)
    {
        return m_Types[type].Spawn(fsm);
    }
};

class eAITransition
{
    static const int FAIL = 0;
    static const int SUCCESS = 1; 

    protected string m_ClassName;

    protected ref eAIHFSM m_FSM;

    /* STATE VARIABLES */
    protected eAIBase unit;

    eAIState GetSource()
    {
        return null;
    }

    eAIState GetDestination()
    {
        return null;
    }

    bool Guard()
    {
        return false;
    }

    static eAITransitionType LoadXML(string fsm, CF_XML_Tag xml_root_tag, ScriptModule module)
    {
        string from_state_name;
        auto from_state = xml_root_tag.GetTag("from_state");
        if (from_state.Count() > 0) from_state_name = from_state[0].GetAttribute("name").ValueAsString();
        string from_state_class = "eAI_" + fsm + "_" + from_state_name + "_State";

        string to_state_name;
        auto to_state = xml_root_tag.GetTag("to_state");
        if (to_state.Count() > 0) to_state_name = to_state[0].GetAttribute("name").ValueAsString();
        string to_state_class = "eAI_" + fsm + "_" + to_state_name + "_State";

        string class_name = "eAI_" + fsm + "_" + from_state_name + "_" + to_state_name + "_Transition";

        if (eAITransitionType.Contains(class_name)) return eAITransitionType.Get(class_name);

        eAITransitionType new_type = new eAITransitionType();
		new_type.m_ClassName = class_name;

        MakeDirectory("$profile:eAI/");
        string script_path = "$profile:eAI/" + class_name + ".c";
        FileHandle file = OpenFile(script_path, FileMode.WRITE);
        if (!file) return null;

        FPrintln(file, "class " + class_name + " extends eAITransition {");

        FPrintln(file, "private " + from_state_class + " m_Source;");
        FPrintln(file, "private " + to_state_class + " m_Destination;");

        FPrintln(file, "void " + class_name + "(eAIHFSM fsm) {");
        FPrintln(file, "m_ClassName = \"" + class_name + "\";");
        FPrintln(file, "m_Source = fsm.GetState(\"" + from_state_class + "\");");
        FPrintln(file, "m_Destination = fsm.GetState(\"" + to_state_class + "\");");
        FPrintln(file, "}");

        auto guard = xml_root_tag.GetTag("guard");
        if (guard.Count() > 0)
        {
            FPrintln(file, "override int Guard() {");
            FPrintln(file, guard[0].GetContent().GetContent());
            FPrintln(file, "}");
        }
        
        FPrintln(file, "eAIState GetSource() { return m_Source; }");
        FPrintln(file, "eAIState GetDestination() { return m_Destination; }");

        FPrintln(file, "}");

        FPrintln(file, "eAITransition Create_" + class_name + "(eAIHFSM fsm) {");
        FPrintln(file, "return new " + class_name + "(fsm);");
        FPrintln(file, "}");

        CloseFile(file);
        
		eAITransitionType.Add(new_type);
        new_type.m_Module = ScriptModule.LoadScript(module, script_path, false);
        if (new_type.m_Module == null)
        {
            Error("There was an error loading in the transition.");
            return null;
        }
		
		return new_type;
    }
};