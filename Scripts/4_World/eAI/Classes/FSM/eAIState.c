class eAIStateType
{
    private static ref map<string, ref eAIStateType> m_Types = new map<string, ref eAIStateType>();

	string m_ClassName;
    ScriptModule m_Module;
    autoptr array<string> m_Variables;

    void eAIStateType()
    {
        m_Variables = new array<string>();
    }

    static bool Contains(string name)
    {
        return m_Types.Contains(name);
    }

    static void Add(eAIStateType type)
    {
        m_Types.Insert(type.m_ClassName, type);
    }

    static eAIStateType Get(string type)
    {
        return m_Types[type];
    }

    eAIState Spawn(eAIHFSM fsm)
    {
        eAIState retValue = null;
        m_Module.CallFunction(null, "Create_" + m_ClassName, retValue, fsm);
        return retValue;
    }

    static eAIState Spawn(string type, eAIHFSM fsm)
    {
        return m_Types[type].Spawn(fsm);
    }
};

class eAIState
{
    static const int EXIT = 0;
    static const int CONTINUE = 1;
    static const int REEVALUTATE = 2;

    protected string m_Name;
    protected string m_ClassName;

    protected ref eAIHFSM m_FSM;

    /* STATE VARIABLES */
    protected eAIBase unit;

    void eAIState(eAIBase _unit)
    {
        unit = _unit;
    }

    static eAIStateType LoadXML(string fsm, CF_XML_Tag xml_root_tag, ScriptModule module)
    {
        string name = xml_root_tag.GetAttribute("name").ValueAsString();
        string class_name = "eAI_" + fsm + "_" + name + "_State";

        if (eAIStateType.Contains(class_name)) return eAIStateType.Get(class_name);

        eAIStateType new_type = new eAIStateType();
		new_type.m_ClassName = class_name;

        MakeDirectory("$profile:eAI/");
        string script_path = "$profile:eAI/" + class_name + ".c";
        FileHandle file = OpenFile(script_path, FileMode.WRITE);
        if (!file) return null;

        FPrintln(file, "class " + class_name + " extends eAIState {");

        auto variables = xml_root_tag.GetTag("variables");
        if (variables.Count() > 0)
		{
			variables = variables[0].GetTag("variable");
	        foreach (auto variable : variables)
	        {
	            string variable_name = variable.GetAttribute("name").ValueAsString();
	            string variable_type = variable.GetAttribute("type").ValueAsString();
	            new_type.m_Variables.Insert(variable_name);
	            FPrintln(file, "private " + variable_type + " " + variable_name + ";");
	        }
		}

        FPrintln(file, "void " + class_name + "(eAIBase _unit) {");
        FPrintln(file, "m_Name = \"" + name + "\";");
        FPrintln(file, "m_ClassName = \"" + class_name + "\";");
        FPrintln(file, "}");

        auto event_entry = xml_root_tag.GetTag("event_entry");
        if (event_entry.Count() > 0)
        {
            FPrintln(file, "override void OnEntry() {");
            FPrintln(file, event_entry[0].GetContent().GetContent());
            FPrintln(file, "}");
        }

        auto event_exit = xml_root_tag.GetTag("event_exit");
        if (event_exit.Count() > 0)
        {
            FPrintln(file, "override void OnExit() {");
            FPrintln(file, event_exit[0].GetContent().GetContent());
            FPrintln(file, "}");
        }

        auto event_update = xml_root_tag.GetTag("event_update");
        if (event_update.Count() > 0)
        {
            FPrintln(file, "override int OnUpdate(float DeltaTime) {");
            FPrintln(file, event_update[0].GetContent().GetContent());
            FPrintln(file, "}");
        }

        FPrintln(file, "}");

        FPrintln(file, "eAIState Create_" + class_name + "(eAIBase _unit) {");
        FPrintln(file, "return new " + class_name + "(_unit);");
        FPrintln(file, "}");

        CloseFile(file);
        
        new_type.m_Module = ScriptModule.LoadScript(module, script_path, false);
        if (new_type.m_Module == null)
        {
            Error("There was an error loading in the state.");
            return null;
        }
		
		eAIStateType.Add(new_type);
		
		return new_type;
    }

    /* IMPLEMENTED IN XML */
    void OnEntry()
    {

    }

    void OnExit()
    {

    }

    int OnUpdate(float DeltaTime)
    {
        return REEVALUTATE;
    }
};