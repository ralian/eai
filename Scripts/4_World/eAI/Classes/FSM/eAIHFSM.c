class eAIHFSMType
{
    private static ref map<string, ref eAIHFSMType> m_Types = new map<string, ref eAIHFSMType>();

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
    static const int EXIT = 0;
    static const int CONTINUE = 1;

    private autoptr array<ref eAIState> m_States;
    private autoptr array<ref eAITransition> m_Transitions;

    private eAIState m_CurrentState;
    private eAIState m_ParentState;
    private bool m_Running;

    protected string m_Name;
    protected string m_DefaultState;
    protected eAIBase m_Unit;

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

    eAIState GetParent()
    {
        return m_ParentState;
    }

    void AddState(eAIState state)
    {
        m_States.Insert(state);
    }

    void AddTransition(eAITransition transition)
    {
        m_Transitions.Insert(transition);
    }

    void SortTransitions()
    {
        //TODO: if the source transition is null, push to the back of the array
        //TODO: if the destination transition is null, remove from the array
    }
	
	eAIState GetState()
    {
        return m_CurrentState;
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
	
	bool StartDefault()
	{
        eAITrace trace("eAIFSM::StartDefault");

        eAIState src = m_CurrentState;
        eAIState dst = GetState(m_DefaultState);

        if (m_Running && src)
        {
            eAILogger.Debug("Exiting state: " + src);
            src.OnExit("", true, dst);
        }
	
		m_CurrentState = dst;
		
        if (m_CurrentState)
        {
            eAILogger.Debug("Starting state: " + m_CurrentState);
            m_CurrentState.OnEntry("", src);
            return true;
        }
		
        eAILogger.Warn("No valid state found.");
		
        return false;
	}

    bool Start(string e = "")
    {
        eAITrace trace("eAIFSM::Start e=" + e);

        Param2<eAIState, bool> new_state = FindSuitableTransition(m_CurrentState, "");

        eAIState src = m_CurrentState;
        eAIState dst = new_state.param1;

        if (dst == null)
        {
            eAILogger.Warn("No valid state found. Aborting.");

            return false;
        }

        if (m_Running && m_CurrentState && m_CurrentState != dst)
        {
            eAILogger.Debug("Exiting state: " + m_CurrentState);
            m_CurrentState.OnExit(e, true, dst);
        }

        m_CurrentState = dst;

        if (m_CurrentState && src != m_CurrentState)
        {
            eAILogger.Debug("Starting state: " + m_CurrentState);
            m_CurrentState.OnEntry(e, src);
            return true;
        }

        eAILogger.Warn("No valid state found.");

        return false;
    }

    bool Abort(string e = "")
    {
        eAITrace trace("eAIFSM::Abort e=" + e);

        if (m_Running && m_CurrentState)
        {
            eAILogger.Debug("Exiting state: " + m_CurrentState);
            m_CurrentState.OnExit(e, true, null);
            return true;
        }

        return false;
    }

    /**
     * @return true Tell the parent FSM that the child FSM is complete
     * @return false Tell the parent FSM that the child FSM is still running
     */
    int Update(float pDt, int pSimulationPrecision)
    {
        if (m_CurrentState && m_CurrentState.OnUpdate(pDt, pSimulationPrecision) == CONTINUE) return CONTINUE;

        Param2<eAIState, bool> new_state = FindSuitableTransition(m_CurrentState, "");
        if (!new_state.param2 || (new_state.param2 && m_CurrentState == new_state.param1))
        {	
            if (!m_CurrentState) return EXIT;

            return CONTINUE;
        }

        eAIState src = m_CurrentState;

        if (m_CurrentState) m_CurrentState.OnExit("", false, new_state.param1);

        m_CurrentState = new_state.param1;

        if (new_state.param1 == null) return EXIT;
		
		eAILogger.Debug("State transition " + src.GetName() + " -> " + m_CurrentState.GetName());

        m_CurrentState.OnEntry("", src);

        return CONTINUE;
    }
	
	Param2<eAIState, bool> FindSuitableTransition(eAIState s, string e = "")
	{
        // returns tuple as a valid destination can still be null

        //TODO: store a reference to the transitions inside the state for that state

		eAIState curr_state = s;

		int count = m_Transitions.Count();
		for (int i = 0; i < count; ++i)
		{
			auto t = m_Transitions.Get(i);
			if ((t.GetSource() == curr_state || t.GetSource() == null) && (e == "" || (e != "" && t.GetEvent() == e)))
			{
				int guard = t.Guard();
                switch (guard)
                {
                case eAITransition.SUCCESS:
				    return new Param2<eAIState, bool>(t.GetDestination(), true);
                case eAITransition.FAIL:
				    break;
                }
			}
		}

		return new Param2<eAIState, bool>(null, false);
	}

    static eAIHFSMType LoadXML(string path, string fileName)
    {
        string actualFilePath = path + "/" + fileName + ".xml";
        eAILogger.Debug(actualFilePath);
        if (!FileExist(actualFilePath)) return null;
        
        CF_XML_Document document;
        CF_XML.ReadDocument(actualFilePath, document);
		
        string name = document.Get("fsm")[0].GetAttribute("name").ValueAsString();
        string class_name = "eAI_" + name + "_HFSM";

        if (eAIHFSMType.Contains(class_name)) return eAIHFSMType.Get(class_name);

        auto files = document.Get("fsm");
        files = files[0].GetTag("files");
        if (files.Count() > 0)
        {
            files = files[0].GetTag("file");

            for (int i = 0; i < files.Count(); i++)
            {
                string subFSMName = files[i].GetAttribute("name").ValueAsString();
                Print(subFSMName);
                eAIHFSM.LoadXML(path, subFSMName);
            }
        }

        eAIHFSMType new_type = new eAIHFSMType();
		new_type.m_ClassName = class_name;

        MakeDirectory("$profile:eAI/");
        string script_path = "$profile:eAI/" + class_name + ".c";
        FileHandle file = OpenFile(script_path, FileMode.WRITE);
        if (!file) return null;
		
        FPrintln(file, "class " + class_name + " extends eAIHFSM {");

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

        ScriptModule module = GetGame().GetMission().MissionScript;

	    foreach (auto state : states)
        {
            eAIStateType stateType = eAIState.LoadXML(name, state, module);
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
            eAITransitionType transitionType = eAITransition.LoadXML(name, transition, module);
            if (transitionType.m_Module != null)
            {
                module = transitionType.m_Module;
                new_type.m_Transitions.Insert(transitionType);

                FPrintln(file, "AddTransition(new " + transitionType.m_ClassName + "(this, m_Unit));");
            }
        }

        FPrintln(file, "}");

        FPrintln(file, "}");

        FPrintln(file, "eAIHFSM Create_" + class_name + "(eAIBase unit, eAIState parentState) {");
        FPrintln(file, "return new " + class_name + "(unit, parentState);");
        FPrintln(file, "}");
		
		CloseFile(file);
		
        new_type.m_Module = ScriptModule.LoadScript(module, script_path, false);
        if (new_type.m_Module == null)
        {
            Error("There was an error loading in the transition.");
            return null;
        }

		eAIHFSMType.Add(new_type);
		
		return new_type;
    }
};