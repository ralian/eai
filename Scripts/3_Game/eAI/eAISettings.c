class eAISettings : JsonApiStruct
{
	static ref ScriptInvoker ON_UPDATE = new ScriptInvoker();

	private static const string PATH = "$profile:eAI/eAISettings.json";

	private static ref eAISettings m_Instance = new eAISettings();

	private string m_LoadingArray;

	private LogLevel m_LogLevel = LogLevel.INFO;
	private bool m_LogLevelSavedAsString = true;

	private float m_Accuracy = 0.5;

	private ref array<string> m_LoadoutDirectories = { "$profile:" };
	private bool m_LoadoutDirectoriesSavedAsArray = false;

	private ref array<string> m_Admins = { "" };

	private int m_MaxDynamicPatrols = -1;

	void SetLogLevel(LogLevel logLevel)
	{
		m_LogLevel = logLevel;
	}

	static LogLevel GetLogLevel()
	{
		return m_Instance.m_LogLevel;
	}

	void SetMaximumDynamicPatrols(int num)
	{
		m_MaxDynamicPatrols = num;
	}

	static int GetMaximumDynamicPatrols()
	{
		return m_Instance.m_MaxDynamicPatrols;
	}

	void SetAccuracy(float accuracy)
	{
		m_Accuracy = accuracy;
	}

	static float GetAccuracy()
	{
		return m_Instance.m_Accuracy;
	}

	void AddLoadoutDirectory(string path)
	{
		m_LoadoutDirectories.Insert(path);
	}
	
	void ClearLoadoutDirectories()
	{
		m_LoadoutDirectories.Clear();
	}

	static array<string> GetLoadoutDirectories()
	{
		return m_Instance.m_LoadoutDirectories;
	}

	void AddAdmin(string admin)
	{
		m_Admins.Insert(admin);

		array<PlayerIdentity> identities();
		GetGame().GetPlayerIndentities(identities);
		foreach (auto identity : identities)
		{
			if (identity.GetPlainId() == admin)
			{
        		GetRPCManager().SendRPC("eAI", "RPC_SetAdmin", new Param1<bool>(true), true, identity);
			}
		}
	}
	
	void ClearAdmins()
	{
		m_Admins.Clear();

        GetRPCManager().SendRPC("eAI", "RPC_SetAdmin", new Param1<bool>(false), true, null);
	}

	static array<string> GetAdmins()
	{
		return m_Instance.m_Admins;
	}

	override void OnInteger(string name, int value)
	{
		if (name == "LogLevel")
		{
			m_LogLevelSavedAsString = false;
			SetLogLevel(value);
			return;
		}

		if (name == "MaxDynamicPatrols")
		{
			SetMaximumDynamicPatrols(value);
			return;
		}
	}

	override void OnString(string name, string value)
	{
		if (name == "LogLevel")
		{
			m_LogLevelSavedAsString = true;
			if (value == "TRACE") SetLogLevel(LogLevel.TRACE);
			if (value == "DEBUG") SetLogLevel(LogLevel.DEBUG);
			if (value == "INFO") SetLogLevel(LogLevel.INFO);
			if (value == "WARNING") SetLogLevel(LogLevel.WARNING);
			if (value == "ERROR") SetLogLevel(LogLevel.ERROR);
			if (value == "NONE") SetLogLevel(LogLevel.NONE);
			return;
		}

		if (name == "LoadoutDirectories")
		{
			ClearLoadoutDirectories();
			AddLoadoutDirectory(value);
			return;
		}
	}

	override void OnFloat(string name, float value)
	{
		if (name == "Accuracy")
		{
			SetAccuracy(value);
			return;
		}
	}

	override void OnStartArray(string name)
	{
		m_LoadingArray = name;
		
		if (m_LoadingArray == "LoadoutDirectories")
		{
			ClearLoadoutDirectories();
			return;
		}
		
		if (m_LoadingArray == "Admins")
		{
			ClearAdmins();
			return;
		}
	}

	override void OnEndArray(int itemCount)
	{
		m_LoadingArray = "";
	}

	override void OnItemString(int index, string value)
	{
		if (m_LoadingArray == "LoadoutDirectories")
		{
			AddLoadoutDirectory(value);
			return;
		}

		if (m_LoadingArray == "Admins")
		{
			AddAdmin(value);
			return;
		}
	}

	override void OnPack()
	{
		if (m_LogLevelSavedAsString)
		{
			StoreString("LogLevel", typename.EnumToString(LogLevel, m_LogLevel));
		}
		else
		{
			StoreInteger("LogLevel", m_LogLevel);
		}

		StoreInteger("MaxDynamicPatrols", m_MaxDynamicPatrols);
		StoreFloat("Accuracy", m_Accuracy);

		if (m_LoadoutDirectoriesSavedAsArray || m_LoadoutDirectories.Count() > 1)
		{
			StartArray("LoadoutDirectories");
			foreach (string loadoutDirectory : m_LoadoutDirectories)
			{
				ItemString(loadoutDirectory);
			}
			EndArray();
		}
		else if (m_LoadoutDirectories.Count() == 1)
		{
			StoreString("LoadoutDirectories", m_LoadoutDirectories[0]);
		}

		StartArray("Admins");
		foreach (string admin : m_Admins)
		{
			ItemString(admin);
		}
		EndArray();
	}

	override void OnSuccess(int errorCode)
	{
		ON_UPDATE.Invoke();
	}

	override void OnError(int errorCode)
	{
		Print(errorCode);
		ON_UPDATE.Invoke();
	}

	static void Init()
	{
		if (!GetJsonApi())
		{
			CreateJsonApi();
		}

		if (!FileExist(PATH))
		{
			m_Instance.Save();
			ON_UPDATE.Invoke();
			return;
		}

		m_Instance.Load();
	}

	static eAISettings Get()
	{
		return m_Instance;
	}

	private void eAISettings() {}

	void Load()
	{
		FileHandle file_handle = OpenFile(PATH, FileMode.READ);
		string content;
		string line_content;
		while (FGets(file_handle, line_content) >= 0)
		{
			content += line_content;
		}

		CloseFile(file_handle);

		ExpandFromRAW(content);
	}

	void Save()
	{
		InstantPack();
		SaveToFile(PATH);
	}
};