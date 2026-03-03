// File: src/logger.cpp
// Purpose: Thread-safe file logger. Writes timestamped lines to
//          CyberSleuthAccessibility.log next to the plugin DLL.
//          Log file is truncated (fresh) on each plugin load.

static FILE* g_logFile  (line 8)
static CRITICAL_SECTION g_logLock  (line 9)
static bool g_lockInit  (line 10)

// Opens the log file next to the DLL (resolved via GetModuleHandleEx on
// Logger_Init's own address). Falls back to CWD if DLL path unavailable.
void Logger_Init()  (line 12)

void Logger_Shutdown()  (line 52)

// Printf-style logger. Prepends [HH:MM:SS.mmm] [tag] to each line.
// Thread-safe via CRITICAL_SECTION.
void Logger_Log(const char* tag, const char* fmt, ...)  (line 65)

// Converts a wide string to UTF-8 and logs it via Logger_Log with tag "SR".
// Used to log speech output. interrupt=true logs as "Say", false as "SayQueued".
void Logger_LogSpeech(const wchar_t* text, bool interrupt)  (line 88)
