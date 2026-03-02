#include "logger.h"
#include <windows.h>
#include <cstdio>
#include <cstdarg>
#include <ctime>
#include <malloc.h>

static FILE* g_logFile = nullptr;
static CRITICAL_SECTION g_logLock;
static bool g_lockInit = false;

void Logger_Init()
{
    InitializeCriticalSection(&g_logLock);
    g_lockInit = true;

    // Find our DLL's directory
    HMODULE hSelf = nullptr;
    GetModuleHandleExA(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (LPCSTR)&Logger_Init,
        &hSelf);

    char dllPath[MAX_PATH] = {};
    if (hSelf) {
        GetModuleFileNameA(hSelf, dllPath, MAX_PATH);
    }

    // Strip filename, append our log filename
    char logPath[MAX_PATH] = {};
    if (dllPath[0]) {
        char* lastSlash = strrchr(dllPath, '\\');
        if (lastSlash) {
            *(lastSlash + 1) = '\0';
            snprintf(logPath, MAX_PATH, "%sCyberSleuthAccessibility.log", dllPath);
        }
    }

    // Fallback: write to current directory
    if (!logPath[0]) {
        snprintf(logPath, MAX_PATH, "CyberSleuthAccessibility.log");
    }

    // Truncate on each launch (fresh log per session)
    g_logFile = fopen(logPath, "w");
    if (g_logFile) {
        // Write BOM-less UTF-8 header
        Logger_Log("Init", "Log opened: %s", logPath);
    }
}

void Logger_Shutdown()
{
    if (g_logFile) {
        Logger_Log("Init", "Log closed.");
        fclose(g_logFile);
        g_logFile = nullptr;
    }
    if (g_lockInit) {
        DeleteCriticalSection(&g_logLock);
        g_lockInit = false;
    }
}

void Logger_Log(const char* tag, const char* fmt, ...)
{
    if (!g_logFile) return;

    EnterCriticalSection(&g_logLock);

    // Timestamp
    SYSTEMTIME st;
    GetLocalTime(&st);
    fprintf(g_logFile, "[%02d:%02d:%02d.%03d] [%s] ",
            st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, tag);

    va_list args;
    va_start(args, fmt);
    vfprintf(g_logFile, fmt, args);
    va_end(args);

    fprintf(g_logFile, "\n");
    fflush(g_logFile);

    LeaveCriticalSection(&g_logLock);
}

void Logger_LogSpeech(const wchar_t* text, bool interrupt)
{
    if (!g_logFile || !text) return;

    // Convert wide string to UTF-8 for the log file
    int needed = WideCharToMultiByte(CP_UTF8, 0, text, -1, nullptr, 0, nullptr, nullptr);
    if (needed <= 0) return;

    char* utf8 = (char*)_alloca(needed);
    WideCharToMultiByte(CP_UTF8, 0, text, -1, utf8, needed, nullptr, nullptr);

    Logger_Log("SR", "%s \"%s\"", interrupt ? "Say" : "SayQueued", utf8);
}
