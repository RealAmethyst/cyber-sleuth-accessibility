// File: src/speech_manager.cpp
// Purpose: Thread-safe SRAL wrapper. Speak() queues requests; Flush() dispatches
//          them from the SwapBuffers context (game thread), never from hook contexts.
//          Also maintains a separate CyberSleuth_speech.log next to the DLL.

// Singleton accessor.
SpeechManager* SpeechManager::Get()  (line 8)

SpeechManager::~SpeechManager()  (line 14)

// Calls SRAL_Initialize(0) (0 = enable all engines). Opens speech log file
// next to the DLL. Safe to call multiple times (guarded by m_initialized).
bool SpeechManager::Initialize()  (line 19)

// Closes speech log and calls SRAL_Uninitialize().
void SpeechManager::Shutdown()  (line 63)

// Queues a speech request (never calls SRAL directly — safe from any thread/context).
// Drops silently if muted or not initialized.
void SpeechManager::Speak(const std::string& text, bool interrupt)  (line 80)

// Drains the queue and dispatches each request to SRAL_Speak().
// Must only be called from SwapBuffers context (game thread).
// Skips dispatch if no SRAL engine is active (engine == 0).
void SpeechManager::Flush()  (line 91)

// Calls SRAL_StopSpeech() immediately (not queued).
void SpeechManager::Silence()  (line 111)

// Writes a timestamped entry to the speech log and mirrors it to Logger_Log("SR").
// Thread-safe via m_logMutex.
void SpeechManager::LogSpeech(const std::string& text, bool interrupt)  (line 117)
