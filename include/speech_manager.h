#pragma once

#include <atomic>
#include <mutex>
#include <string>
#include <vector>

// Centralized speech output via SRAL (Screen Reader Abstraction Layer).
//
// IMPORTANT: Speak() is safe to call from ANY thread/hook context (tick
// detours, etc.). It queues requests internally. Call Flush() once per
// frame from the SwapBuffers hook to actually dispatch to SRAL.
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

    // Queue a speech request. Thread-safe, never blocks the game.
    // interrupt=true: clears current speech (cursor navigation)
    // interrupt=false: queues after current speech (menu name + first item)
    void Speak(const std::string& text, bool interrupt = true);

    // Dispatch queued speech to SRAL. Call once per frame from SwapBuffers.
    void Flush();

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

    struct SpeechRequest {
        std::string text;
        bool interrupt;
    };

    FILE* m_speechLog = nullptr;
    std::mutex m_logMutex;
    std::mutex m_queueMutex;
    std::vector<SpeechRequest> m_queue;
    std::atomic<bool> m_muted{false};
    bool m_initialized = false;
};
