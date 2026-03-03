#include "speech_manager.h"
#include "plugin_util.h"
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
    std::string logPath = GetPluginDir() + "CyberSleuth_speech.log";
    m_speechLog = fopen(logPath.c_str(), "w");
    if (m_speechLog) {
        Logger_Log("Speech", "Speech log opened: %s", logPath.c_str());
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

    // Queue only — never call SRAL from arbitrary hook contexts.
    std::lock_guard<std::mutex> lock(m_queueMutex);
    m_queue.push_back({text, interrupt});
}

void SpeechManager::Flush()
{
    if (!m_initialized) return;

    // Swap queue out under lock, then dispatch without holding it.
    std::vector<SpeechRequest> pending;
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        pending.swap(m_queue);
    }

    if (pending.empty()) return;
    if (SRAL_GetCurrentEngine() == 0) return;

    for (auto& req : pending) {
        LogSpeech(req.text, req.interrupt);
        SRAL_Speak(req.text.c_str(), req.interrupt);
    }
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
