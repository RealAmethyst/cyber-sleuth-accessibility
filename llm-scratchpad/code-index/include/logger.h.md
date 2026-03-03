// File: include/logger.h
// Purpose: File-based debug logger. Writes timestamped entries to
//          CyberSleuthAccessibility.log next to the plugin DLL.
//          Screen reader speech output is also recorded here.

void Logger_Init();     // (line 7)
void Logger_Shutdown(); // (line 8)

// Write a tagged, timestamped log line: [HH:MM:SS.mmm] [tag] message
// Accepts printf-style format string and variadic args.
void Logger_Log(const char* tag, const char* fmt, ...);  // (line 11)

// Record a screen reader speech event. Called automatically by the speech layer.
// Note: comment in file says "Tolk_Say" — this is a legacy name; SRAL is the actual backend.
void Logger_LogSpeech(const wchar_t* text, bool interrupt);  // (line 14)
