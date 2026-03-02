#pragma once

#include <atomic>
#include <mutex>
#include <string>

// Centralized speech output via SRAL (Screen Reader Abstraction Layer).
// Replaces Tolk with a more robust multi-engine approach.
//
// Usage: SpeechManager::Get()->Speak("text", true);
class SpeechManager
{
public:
    static SpeechManager* Get();

    // Initialize SRAL and open speech log.
    // Call once from onEnable() before any speech.
    bool Initialize();

    // Shut down SRAL and close log.
    void Shutdown();

    // Main speech output — all announcements go through here.
    // interrupt=true: clears current speech (cursor navigation)
    // interrupt=false: queues after current speech (menu name + first item)
    void Speak(const std::string& text, bool interrupt = true);

    // Stop all current speech.
    void Silence();

    // Mute/unmute (thread-safe).
    void SetMuted(bool muted) { m_muted.store(muted, std::memory_order_relaxed); }
    bool IsMuted() const { return m_muted.load(std::memory_order_relaxed); }

    bool IsInitialized() const { return m_initialized; }

private:
    SpeechManager() = default;
    ~SpeechManager();
    SpeechManager(const SpeechManager&) = delete;
    SpeechManager& operator=(const SpeechManager&) = delete;

    void LogSpeech(const std::string& text, bool interrupt);

    FILE* m_speechLog = nullptr;
    std::mutex m_logMutex;
    std::atomic<bool> m_muted{false};
    bool m_initialized = false;
};
