class eAITrace
{
	private string m_StackName;
	private int m_TickCount;

	void eAITrace(string stackName)
	{
		m_TickCount = TickCount(m_TickCount);
		m_StackName = stackName;

		eAILogger.Trace("-%1", m_StackName);
	}

	void ~eAITrace()
	{
		m_TickCount = TickCount(m_TickCount);

		eAILogger.Trace("+%1 CPU Ticks: %2", m_StackName, m_TickCount.ToString());
	}
};

class eAILogger
{
	private static LogLevel m_LogLevel = LogLevel.NONE;

	static void Init()
	{
		eAISettings.ON_UPDATE.Insert(OnUpdate);
	}

	static void OnUpdate()
	{
		m_LogLevel = eAISettings.GetLogLevel();
	}

	static void Trace(string message, string param1 = "", string param2 = "", string param3 = "", string param4 = "", string param5 = "", string param6 = "", string param7 = "", string param8 = "", string param9 = "")
	{
		if (m_LogLevel > LogLevel.TRACE) return;

		PrintFormat("[TRACE] %1", string.Format(message, param1, param2, param3, param4, param5, param6, param7, param8, param9));
	}

	static void Debug(string message, string param1 = "", string param2 = "", string param3 = "", string param4 = "", string param5 = "", string param6 = "", string param7 = "", string param8 = "", string param9 = "")
	{
		if (m_LogLevel > LogLevel.DEBUG) return;

		PrintFormat("[DEBUG] %1", string.Format(message, param1, param2, param3, param4, param5, param6, param7, param8, param9));
	}

	static void Info(string message, string param1 = "", string param2 = "", string param3 = "", string param4 = "", string param5 = "", string param6 = "", string param7 = "", string param8 = "", string param9 = "")
	{
		if (m_LogLevel > LogLevel.INFO) return;

		PrintFormat("[INFO] %1", string.Format(message, param1, param2, param3, param4, param5, param6, param7, param8, param9));
	}

	static void Warn(string message, string param1 = "", string param2 = "", string param3 = "", string param4 = "", string param5 = "", string param6 = "", string param7 = "", string param8 = "", string param9 = "")
	{
		if (m_LogLevel > LogLevel.WARNING) return;

		PrintFormat("[WARNING] %1", string.Format(message, param1, param2, param3, param4, param5, param6, param7, param8, param9));
	}

	static void Error(string message, string param1 = "", string param2 = "", string param3 = "", string param4 = "", string param5 = "", string param6 = "", string param7 = "", string param8 = "", string param9 = "")
	{
		if (m_LogLevel > LogLevel.ERROR) return;

		PrintFormat("[ERROR] %1", string.Format(message, param1, param2, param3, param4, param5, param6, param7, param8, param9));
	}
};