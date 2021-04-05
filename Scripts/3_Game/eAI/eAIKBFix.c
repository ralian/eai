// This class wraps an UAInput keybind and saves it to a file. This is to combat 
// modded keybinds getting reset when mods are unloaded and reloaded.
class eAIKeybinds {
	string name = "eAICommandMenu";
	int keybinding = -1;
	
	// Call this whenever the keybinding menu is closed
	bool save(string filename) {
		// todo: only works for single binding, use a map instead
		UAInput binding = GetUApi().GetInputByName(name);
		keybinding = binding.GetBindKey(0);
		JsonFileLoader<eAIKeybinds>.JsonSaveFile(filename, this);
		Print("eAIKeybinds: Saved " + filename);
		return true;
	}
	
	// Call this on game launch
	bool load(string filename) {
		if (!FileExist(filename))
			save(filename);
		JsonFileLoader<eAIKeybinds>.JsonLoadFile(filename, this);
		Print("eAIKeybinds: Loaded " + filename);
		if (keybinding < 0)
			return false;
		UAInput binding = GetUApi().GetInputByName(name);
		binding.ClearBinding();
		binding.BindComboByHash(keybinding);
		Print("eAIKeybinds: " + name + " bound to " + keybinding.ToString());
		return true;
	}
};
