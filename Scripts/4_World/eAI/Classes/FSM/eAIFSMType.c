class eAIFSMType
{
    private static ref map<string, ref eAIFSMType> m_Types = new map<string, ref eAIFSMType>();

	string m_ClassName;
    ScriptModule m_Module;

    ref array<string> m_Variables;
    ref array<ref eAIStateType> m_States;
    ref array<ref eAITransitionType> m_Transitions;

    void eAIFSMType()
    {
        m_States = new array<ref eAIStateType>();
        m_Transitions = new array<ref eAITransitionType>();
    }

    static bool Contains(string name)
    {
        return m_Types.Contains(name);
    }

    static void Add(eAIFSMType type)
    {
        m_Types.Insert(type.m_ClassName, type);
    }

    static eAIFSMType Get(string type)
    {
        return m_Types[type];
    }

    eAIFSM Spawn(eAIBase unit, eAIState parentState)
    {
        eAIFSM retValue = null;
        m_Module.CallFunctionParams(null, "Create_" + m_ClassName, retValue, new Param2<eAIBase, eAIState>(unit, parentState));
        return retValue;
    }

    static eAIFSM Spawn(string type, eAIBase unit, eAIState parentState)
    {
        return m_Types[type].Spawn(unit, parentState);
    }

    static eAIFSMType LoadXML(string path, string fileName)
    {
		//eAITrace trace(null, "eAIFSMType::LoadXML", path, fileName);

        string actualFilePath = path + "/" + fileName + ".xml";
        eAILogger.Debug(actualFilePath);
        if (!FileExist(actualFilePath)) return null;
        
        CF_XML_Document document;
        CF_XML.ReadDocument(actualFilePath, document);
		
        string name = document.Get("fsm")[0].GetAttribute("name").ValueAsString();
        string class_name = "eAI_" + name + "_FSM";

        if (eAIFSMType.Contains(class_name)) return eAIFSMType.Get(class_name);

        auto files = document.Get("fsm");
        files = files[0].GetTag("files");
        if (files.Count() > 0)
        {
            files = files[0].GetTag("file");

            for (int i = 0; i < files.Count(); i++)
            {
                string subFSMName = files[i].GetAttribute("name").ValueAsString();
                Print(subFSMName);
                eAIFSMType.LoadXML(path, subFSMName);
            }
        }

        eAIFSMType new_type = new eAIFSMType();
		new_type.m_ClassName = class_name;

        MakeDirectory("$profile:eAI/");
        string script_path = "$profile:eAI/" + class_name + ".c";
        FileHandle file = OpenFile(script_path, FileMode.WRITE);
        if (!file) return null;
		
        FPrintln(file, "class " + class_name + " extends eAIFSM {");

        auto variables = document.Get("fsm");
        variables = variables[0].GetTag("variables");
        if (variables.Count() > 0)
		{
			variables = variables[0].GetTag("variable");
	        foreach (auto variable : variables)
	        {
	            string variable_name = variable.GetAttribute("name").ValueAsString();
	            string variable_type = variable.GetAttribute("type").ValueAsString();
	            string variable_default = "";
                if (variable.GetAttribute("default")) variable_default = variable.GetAttribute("default").ValueAsString();

	            new_type.m_Variables.Insert(variable_name);

                string variable_line = "" + variable_type + " " + variable_name;
                if (variable_default != "") variable_line = variable_line + " = " + variable_default;
                variable_line = variable_line + ";";

	            FPrintln(file, variable_line);
	        }
		}

        auto states = document.Get("fsm");
        states = states[0].GetTag("states");
        string defaultState = states[0].GetAttribute("default").ValueAsString();
		defaultState = "eAI_" + name + "_" + defaultState + "_State";
        states = states[0].GetTag("state");
			
        FPrintln(file, "void " + class_name + "(eAIBase unit, eAIState parentState) {");
        FPrintln(file, "m_Name = \"" + name + "\";");
        FPrintln(file, "m_DefaultState = \"" + defaultState + "\";");
        FPrintln(file, "Setup();");
        FPrintln(file, "SortTransitions();");
        FPrintln(file, "}");

        FPrintln(file, "void Setup() {");
        FPrintln(file, "//eAITrace trace(this, \"Setup\");");

        ScriptModule module = GetGame().GetMission().MissionScript;

	    foreach (auto state : states)
        {
            eAIStateType stateType = eAIStateType.LoadXML(name, state, module);
            if (stateType.m_Module != null)
            {
                module = stateType.m_Module;
                new_type.m_States.Insert(stateType);
                
                FPrintln(file, "AddState(new " + stateType.m_ClassName + "(this, m_Unit));");
            }
        }

        auto transitions = document.Get("fsm");
        transitions = transitions[0].GetTag("transitions");
        transitions = transitions[0].GetTag("transition");

	    foreach (auto transition : transitions)
        {
            eAITransitionType transitionType = eAITransitionType.LoadXML(name, transition, module);
            if (transitionType.m_Module != null)
            {
                module = transitionType.m_Module;
                new_type.m_Transitions.Insert(transitionType);

                FPrintln(file, "AddTransition(new " + transitionType.m_ClassName + "(this, m_Unit));");
            }
        }

        FPrintln(file, "}");

        FPrintln(file, "}");

        FPrintln(file, "eAIFSM Create_" + class_name + "(eAIBase unit, eAIState parentState) {");
        FPrintln(file, "return new " + class_name + "(unit, parentState);");
        FPrintln(file, "}");
		
		CloseFile(file);
		
        new_type.m_Module = ScriptModule.LoadScript(module, script_path, false);
        if (new_type.m_Module == null)
        {
            Error("There was an error loading in the transition.");
            return null;
        }

		eAIFSMType.Add(new_type);
		
		return new_type;
    }
};