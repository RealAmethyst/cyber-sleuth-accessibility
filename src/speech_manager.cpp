#include "speech_manager.h"
#include "logger.h"

#include <SRAL.h>
#include <windows.h>
#include <cstdio>

SpeechManager* SpeechManager::Get()
{
    static SpeechManager instance;
    return &instance;
}

SpeechManager::~SpeechManager()
{
    Shutdown();
}

bool SpeechManager::Initialize()
{
    if (m_initialized) return true;

    // SRAL_Initialize takes an exclude mask — pass 0 to enable all engines
    if (!SRAL_Initialize(0)) {
        Logger_Log("Speech", "SRAL_Initialize failed");
        return false;
    }

    // Open speech log next to our DLL
    HMODULE hSelf = nullptr;
    GetModuleHandleExA(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (LPCSTR)&SpeechManager::Get,
        &hSelf);

    char dllPath[MAX_PATH] = {};
    if (hSelf) GetModuleFileNameA(hSelf, dllPath, MAX_PATH);

    char logPath[MAX_PATH] = {};
    if (dllPath[0]) {
        char* lastSlash = strrchr(dllPath, '\\');
        if (lastSlash) {
            *(lastSlash + 1) = '\0';
            snprintf(logPath, MAX_PATH, "%sCyberSleuth_speech.log", dllPath);
        }
    }
    if (!logPath[0]) {
        snprintf(logPath, MAX_PATH, "CyberSleuth_speech.log");
    }

    m_speechLog = fopen(logPath, "w");
    if (m_speechLog) {
        Logger_Log("Speech", "Speech log opened: %s", logPath);
    }

    int engine = SRAL_GetCurrentEngine();
    Logger_Log("Speech", "SRAL initialized (engine=%d)", engine);

    m_initialized = true;
    return true;
}

void SpeechManager::Shutdown()
{
    if (!m_initialized) return;

    {
        std::lock_guard<std::mutex> lock(m_logMutex);
        if (m_speechLog) {
            fclose(m_speechLog);
            m_speechLog = nullptr;
        }
    }

    SRAL_Uninitialize();
    m_initialized = false;
    Logger_Log("Speech", "SRAL shut down");
}

void SpeechManager::Speak(const std::string& text, bool interrupt)
{
    if (text.empty()) return;
    if (!m_initialized) return;
    if (m_muted.load(std::memory_order_relaxed)) return;
    if (SRAL_GetCurrentEngine() == 0) return;

    LogSpeech(text, interrupt);
    SRAL_Speak(text.c_str(), interrupt);
}

void SpeechManager::Silence()
{
    if (!m_initialized) return;
    SRAL_StopSpeech();
}

void SpeechManager::LogSpeech(const std::string& text, bool interrupt)
{
    std::lock_guard<std::mutex> lock(m_logMutex);
    if (!m_speechLog) return;

    SYSTEMTIME st;
    GetLocalTime(&st);
    fprintf(m_speechLog, "[%02d:%02d:%02d.%03d] %s \"%s\"\n",
            st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
            interrupt ? "Say" : "SayQueued", text.c_str());
    fflush(m_speechLog);

    // Also mirror to main log
    Logger_Log("SR", "%s \"%s\"", interrupt ? "Say" : "SayQueued", text.c_str());
}
