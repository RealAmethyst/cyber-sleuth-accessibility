// File: include/speech_manager.h
// Purpose: Centralized, thread-safe speech output via SRAL (Screen Reader Abstraction Layer).
//
// IMPORTANT: Speak() is safe to call from ANY thread or hook context (including tick
// detours). It queues requests internally. Call Flush() once per frame from the
// SwapBuffers hook to dispatch the queue to SRAL.
//
// Usage: SpeechManager::Get()->Speak("text", true);

class SpeechManager  // (line 15)
{
public:
    static SpeechManager* Get();  // (line 18) — singleton accessor

    // Initialize SRAL and open the speech log file.
    // Must be called from onEnable() before any Speak() calls.
    bool Initialize();  // (line 22)

    // Shut down SRAL and close the log file. Call from onDisable().
    void Shutdown();  // (line 25)

    // Queue a speech request. Thread-safe, never blocks the game loop.
    // interrupt=true:  clears current speech then speaks (use for cursor navigation)
    // interrupt=false: queues after current speech (use for menu-open sequences)
    void Speak(const std::string& text, bool interrupt = true);  // (line 30)

    // Dispatch all queued speech requests to SRAL. Call once per frame from SwapBuffers.
    void Flush();  // (line 33)

    // Cancel all current and queued speech immediately.
    void Silence();  // (line 36)

    void SetMuted(bool muted);  // (line 39) — atomic, thread-safe
    bool IsMuted() const;       // (line 40) — atomic, thread-safe

    bool IsInitialized() const;  // (line 42)

private:
    SpeechManager() = default;   // (line 45)
    ~SpeechManager();            // (line 46)
    // Non-copyable
    SpeechManager(const SpeechManager&) = delete;             // (line 47)
    SpeechManager& operator=(const SpeechManager&) = delete;  // (line 48)

    // Write a speech event to the speech log (called internally by Flush).
    void LogSpeech(const std::string& text, bool interrupt);  // (line 50)

    // POD aggregate for the speech queue.
    struct SpeechRequest {  // (line 52)
        std::string text;
        bool interrupt;
    }

    FILE* m_speechLog = nullptr;          // (line 57)
    std::mutex m_logMutex;                // (line 58)
    std::mutex m_queueMutex;             // (line 59)
    std::vector<SpeechRequest> m_queue;  // (line 60)
    std::atomic<bool> m_muted{false};    // (line 61)
    bool m_initialized = false;          // (line 62)
}
