// SubtitleHandler — accessibility for cutscene subtitles (Vista system).
//
// Polls the subtitle loader singleton (DAT_140f205d0) each frame.
// No hooks needed — reads game state directly from memory.
// Text comes from the game's own LookupText API at runtime.

#include "handlers/subtitle_handler.h"
#include "speech_manager.h"
#include "game_text.h"
#include "offsets.h"
#include "logger.h"

#include <modloader/utils.h>
#include <windows.h>

// ============================================================
// Singleton
// ============================================================

SubtitleHandler* SubtitleHandler::Get()
{
    static SubtitleHandler instance;
    return &instance;
}

// ============================================================
// Install / Uninstall
// ============================================================

void SubtitleHandler::Install()
{
    if (m_installed) return;

    m_moduleBase = reinterpret_cast<uintptr_t>(getBaseOffset());

    m_installed = true;
    Logger_Log("Subtitle", "SubtitleHandler installed — polling loader at DAT 0x%llx",
               (unsigned long long)Offsets::Vista::DAT_SubtitleLoader);
}

void SubtitleHandler::Uninstall()
{
    if (!m_installed) return;

    m_wasPlaying = false;
    m_lastCueIndex = -1;
    m_lastSpokenText.clear();

    m_installed = false;
    Logger_Log("Subtitle", "SubtitleHandler uninstalled");
}

// ============================================================
// Memory access helpers (all SEH-protected)
// ============================================================

void* SubtitleHandler::GetLoader() const
{
    if (!m_moduleBase) return nullptr;

    __try {
        return *reinterpret_cast<void**>(m_moduleBase + Offsets::Vista::DAT_SubtitleLoader);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        return nullptr;
    }
}

int SubtitleHandler::GetLoaderState() const
{
    void* loader = GetLoader();
    if (!loader) return 0;

    __try {
        return *reinterpret_cast<int*>(
            reinterpret_cast<uint8_t*>(loader) + Offsets::Vista::Loader::STATE);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        return 0;
    }
}

int SubtitleHandler::GetCueIndex() const
{
    void* loader = GetLoader();
    if (!loader) return -1;

    __try {
        auto* base = reinterpret_cast<uint8_t*>(loader);
        auto begin = *reinterpret_cast<intptr_t*>(base + Offsets::Vista::Loader::VECTOR_BEGIN);
        auto end   = *reinterpret_cast<intptr_t*>(base + Offsets::Vista::Loader::VECTOR_END);
        auto cursor = *reinterpret_cast<intptr_t*>(base + Offsets::Vista::Loader::CURSOR);

        if (begin == 0 || cursor == 0 || cursor < begin) return -1;
        if (end != 0 && cursor >= end) return -1;

        return static_cast<int>((cursor - begin) / 8);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        return -1;
    }
}

int SubtitleHandler::GetCueTextId(int cueIndex) const
{
    void* loader = GetLoader();
    if (!loader || cueIndex < 0) return -1;

    __try {
        auto* base = reinterpret_cast<uint8_t*>(loader);
        auto begin = *reinterpret_cast<intptr_t*>(base + Offsets::Vista::Loader::VECTOR_BEGIN);
        if (begin == 0) return -1;

        // Vector entry at begin + cueIndex*8 → pointer to cueStruct
        auto* cueStructPtr = *reinterpret_cast<void**>(begin + cueIndex * 8);
        if (!cueStructPtr) return -1;

        // cueStruct[0] = row data pointer (this IS the column data directly)
        auto* rowData = reinterpret_cast<uint8_t*>(*reinterpret_cast<void**>(cueStructPtr));
        if (!rowData) return -1;

        // rowData + 0x0C = SubtitleTextID (int32)
        return *reinterpret_cast<int*>(rowData + Offsets::Vista::CUE_COL_TEXTID);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        return -1;
    }
}

const char* SubtitleHandler::LookupSubtitleText(int textId) const
{
    if (textId < 0) return nullptr;

    // GameText_Lookup returns std::string — we need a stable pointer.
    // Use a thread-local buffer since OnFrame runs on the SwapBuffers thread only.
    static thread_local std::string s_buf;
    s_buf = GameText_Lookup("subtitle_text", textId);
    if (s_buf.empty()) return nullptr;
    return s_buf.c_str();
}

// ============================================================
// Helper: read schedule name (SEH-safe, no C++ objects)
// ============================================================

static const char* ReadLoaderName(void* loader)
{
    if (!loader) return nullptr;

    __try {
        return *reinterpret_cast<const char**>(
            reinterpret_cast<uint8_t*>(loader) + Offsets::Vista::Loader::NAME);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        return nullptr;
    }
}

// ============================================================
// OnFrame — main per-frame logic (SwapBuffers context)
// ============================================================

void SubtitleHandler::OnFrame()
{
    if (!m_installed) return;

    int state = GetLoaderState();

    // Not playing?
    if (state != 2) {
        if (m_wasPlaying) {
            Logger_Log("Subtitle", "Playback ended (state: %d)", state);
            m_wasPlaying = false;
            m_lastCueIndex = -1;
            m_lastSpokenText.clear();
        }
        return;
    }

    // Just started playing?
    if (!m_wasPlaying) {
        m_wasPlaying = true;
        m_lastCueIndex = -1;
        m_lastSpokenText.clear();

        const char* name = ReadLoaderName(GetLoader());
        Logger_Log("Subtitle", "Playback started: \"%s\"", name ? name : "?");
    }

    // Read current cue index
    int cueIndex = GetCueIndex();
    if (cueIndex < 0 || cueIndex == m_lastCueIndex) return;

    // Cue changed — get the SubtitleTextID
    int textId = GetCueTextId(cueIndex);
    if (textId < 0) return;

    // Look up the text via the game's own API
    const char* text = LookupSubtitleText(textId);
    if (!text || !text[0]) return;

    // Dedup: don't re-speak identical text
    if (m_lastSpokenText == text) return;

    m_lastCueIndex = cueIndex;
    m_lastSpokenText = text;

    Logger_Log("Subtitle", "Cue %d: textId=%d \"%s\"", cueIndex, textId, text);
    SpeechManager::Get()->Speak(text, true);
}
