modded class DayZGame
{
    private ref eAIManagerBase m_eAI_Manager;
	private ref eAICommandManager m_eAI_CommandManager;
	
	///////////////////////////////////////////////////////////////////////////////
	/// THIS WILL EVENTUALLY BE IN CF, THEN WE NEED TO REMOVE IT
	autoptr map<string, int> m_CFPersistentKeybinds;
	
	void CFPopulateKeybinds() {
		m_CFPersistentKeybinds = new map<string, int>();
		string modname;
		for (int i = 0; i < ConfigGetChildrenCount("CfgMods"); i++) {
			ConfigGetChildName("CfgMods", i, modname);
			string xmlpath = "";
			ConfigGetText("CfgMods " + modname + " inputs", xmlpath);
			if (xmlpath.Length() > 0) {
				if (!FileExist(xmlpath)) {
					Print("(CF) Warning: inputs file not existing: " + xmlpath);
					continue;
				}
				
				autoptr CF_XML_Document document = new CF_XML_Document();
				Print("(CF) Loading inputs file: " + xmlpath);
        		CF_XML.ReadDocument(xmlpath, document);
				array<CF_XML_Tag> inputTags = document.Get("modded_inputs");
				inputTags = inputTags[0].GetTag("inputs");
				inputTags = inputTags[0].GetTag("actions");
				inputTags = inputTags[0].GetTag("input");
				foreach (CF_XML_Tag tag : inputTags) {
					m_CFPersistentKeybinds.Insert(tag.GetAttribute("name").GetValue(), 0);
				}
			}
		}
	}
	
	void CFLoadKeybinds(string filename) {
		if (!FileExist(filename))
			CFSaveKeybinds(filename);
		
		JsonFileLoader<map<string, int>>.JsonLoadFile(filename, m_CFPersistentKeybinds);
		Print("(CF) Persistent Keybinds: Loaded " + filename + ", " + m_CFPersistentKeybinds.Count() + " entries!");
		
		for (int i = 0; i < m_CFPersistentKeybinds.Count(); i++) {
			string key = m_CFPersistentKeybinds.GetKey(i);
			int value = m_CFPersistentKeybinds.Get(key);
			if (value == 0)
				continue;
			UAInput binding = GetUApi().GetInputByName(key);
			if (binding) {
				binding.ClearBinding();
				binding.BindComboByHash(value);
				Print("(CF) Persistent Keybinds: " + key + " bound to " + value.ToString());
			}
		}
	}
	
	void CFSaveKeybinds(string filename) {
		for (int i = 0; i < m_CFPersistentKeybinds.Count(); i++) {
			string inputname = m_CFPersistentKeybinds.GetKey(i);
			UAInput binding = GetUApi().GetInputByName(inputname);
			m_CFPersistentKeybinds.Set(inputname, binding.GetBindKey(0));
		}
		
		JsonFileLoader<map<string, int>>.JsonSaveFile(filename, m_CFPersistentKeybinds);
		Print("(CF) Persistent Keybinds: Saved " + filename);
	}
	///////////////////////////////////////////////////////////////////////////////
	
	void DayZGame() {
		m_eAI_CommandManager = new eAICommandManagerClient();
	}

    void eAICreateManager()
    {
        // fancy way to spawn the manager from a submodule
        m_eAI_Manager = eAIManagerBase.Cast("eAIManager".ToType().Spawn());
    }

    eAIManagerBase eAIManagerGet()
    {
        return m_eAI_Manager;
    }
    
    override void OnUpdate(bool doSim, float timeslice)
    {
        super.OnUpdate(doSim, timeslice);
		
		if (m_eAI_Manager)
			m_eAI_Manager.OnUpdate(doSim, timeslice);
    }
	
	eAICommandManager GetEAICommandManager() {
		return m_eAI_CommandManager;
	}
};