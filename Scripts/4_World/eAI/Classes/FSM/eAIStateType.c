class eAIStateType
{
    private static ref map<string, ref eAIStateType> m_Types = new map<string, ref eAIStateType>();

	string m_ClassName;
    ref array<string> m_Variables;

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

    static eAIStateType LoadXML(string fsmName, CF_XML_Tag xml_root_tag, FileHandle file)
    {
		//eAITrace trace(null, "eAIStateType::LoadXML", fsmName);

        string name = xml_root_tag.GetAttribute("name").ValueAsString();
        string child_fsm;
        if (xml_root_tag.GetAttribute("fsm")) child_fsm = xml_root_tag.GetAttribute("fsm").ValueAsString();
        string class_name = "eAI_" + fsmName + "_" + name + "_State";
        
        //if (child_fsm != "") class_name = "eAI_FSM_" + child_fsm + "_State";

        if (eAIStateType.Contains(class_name)) return eAIStateType.Get(class_name);

        eAIStateType new_type = new eAIStateType();
		new_type.m_ClassName = class_name;

        FPrintln(file, "class " + class_name + " extends eAIState {");

        if (child_fsm != "") FPrintln(file, "eAI_" + child_fsm + "_FSM sub_fsm;");
        FPrintln(file, "eAI_" + fsmName + "_FSM fsm;");

        auto variables = xml_root_tag.GetTag("variables");
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

        FPrintln(file, "void " + class_name + "(eAIFSM _fsm, eAIBase _unit) {");
        FPrintln(file, "Class.CastTo(fsm, _fsm);");
        FPrintln(file, "m_ClassName = \"" + class_name + "\";");
        if (child_fsm != "")
        {
            FPrintln(file, "m_Name = \"" + child_fsm + "\";");
            FPrintln(file, "m_SubFSM = new eAI_" + child_fsm + "_FSM(_unit, this);");
            FPrintln(file, "Class.CastTo(sub_fsm, m_SubFSM);");
        }
        else
        {
            FPrintln(file, "m_Name = \"" + name + "\";");
        }

        FPrintln(file, "}");

        if (child_fsm != "")
        {
            FPrintln(file, "override void OnEntry(string Event, eAIState From) {");
        	FPrintln(file, "//eAITrace trace(this, \"OnEntry\", Event, \"(\" + From + \")\");");
            FPrintln(file, "if (Event != \""+"\") m_SubFSM.Start(Event);");
            FPrintln(file, "else m_SubFSM.StartDefault();");
            FPrintln(file, "}");

            FPrintln(file, "override void OnExit(string Event, bool Aborted, eAIState To) {");
        	FPrintln(file, "//eAITrace trace(this, \"OnExit\", Event, Aborted.ToString(), \"(\" + To + \")\");");
            FPrintln(file, "if (Aborted) m_SubFSM.Abort(Event);");
            FPrintln(file, "}");

            FPrintln(file, "override int OnUpdate(float DeltaTime, int SimulationPrecision) {");
        	FPrintln(file, "//eAITrace trace(this, \"OnUpdate\", DeltaTime.ToString(), SimulationPrecision.ToString());");
            FPrintln(file, "return m_SubFSM.Update(DeltaTime, SimulationPrecision);");
            FPrintln(file, "}");
        }
        else
        {
            auto event_entry = xml_root_tag.GetTag("event_entry");
            if (event_entry.Count() > 0)
            {
                FPrintln(file, "override void OnEntry(string Event, eAIState From) {");
        		FPrintln(file, "//eAITrace trace(this, \"OnEntry\", Event, \"(\" + From + \")\");");
                FPrintln(file, event_entry[0].GetContent().GetContent());
                FPrintln(file, "}");
            }

            auto event_exit = xml_root_tag.GetTag("event_exit");
            if (event_exit.Count() > 0)
            {
                FPrintln(file, "override void OnExit(string Event, bool Aborted, eAIState To) {");
        		FPrintln(file, "//eAITrace trace(this, \"OnExit\", Event, Aborted.ToString(), \"(\" + To + \")\");");
                FPrintln(file, event_exit[0].GetContent().GetContent());
                FPrintln(file, "}");
            }

            auto event_update = xml_root_tag.GetTag("event_update");
            if (event_update.Count() > 0)
            {
                FPrintln(file, "override int OnUpdate(float DeltaTime, int SimulationPrecision) {");
        		FPrintln(file, "//eAITrace trace(this, \"OnUpdate\", DeltaTime.ToString(), SimulationPrecision.ToString());");
                FPrintln(file, event_update[0].GetContent().GetContent());
                FPrintln(file, "}");
            }
        }

        FPrintln(file, "}");
		
		eAIStateType.Add(new_type);
		
		return new_type;
    }
};