#pragma once

// File-based debug logger for Cyber Sleuth Accessibility.
// Logs to CyberSleuthAccessibility.log next to the plugin DLL.
// All screen reader output is also logged here.

void Logger_Init();
void Logger_Shutdown();

// Log a tagged message: [HH:MM:SS.mmm] [tag] message
void Logger_Log(const char* tag, const char* fmt, ...);
