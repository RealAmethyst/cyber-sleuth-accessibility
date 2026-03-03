#pragma once

#include "hooks.h"

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_set>
#include <mutex>

// Hooks the game's core text lookup function (FUN_1401b9260) to capture
// every text string the game fetches from MBE tables each frame.
//
// Instead of reverse-engineering each CUi* class individually, this captures
// text at the source — when the game actually needs it for display.
//
// Per-frame diffing detects NEW text (appeared this frame but not last),
// which corresponds to menu transitions, cursor moves, dialog advances, etc.
//
// Usage:
//   TextCapture::Get()->Install();     // after hooks_init()
//   RegisterFrameHandler(TextCapture::Get());
//   // ... game runs, text lookups are captured ...
//   TextCapture::Get()->Uninstall();   // in onDisable()

class TextCapture : public IFrameHandler
{
public:
    static TextCapture* Get();

    // Install MinHook on the LookupText function.
    // Call AFTER hooks_init() (MinHook must be initialized).
    void Install();
    void Uninstall();

    bool IsInstalled() const { return m_installed; }

    // IFrameHandler — called every frame from SwapBuffers.
    // Performs frame diffing: compares current frame's text lookups against
    // previous frame, logs any NEW entries.
    void OnFrame() override;

    // A single captured text lookup
    struct TextEntry {
        std::string tableName;
        int rowId;
        std::string text;

        bool operator==(const TextEntry& other) const {
            return tableName == other.tableName && rowId == other.rowId;
        }
    };

    struct TextEntryHash {
        size_t operator()(const TextEntry& e) const {
            size_t h1 = std::hash<std::string>{}(e.tableName);
            size_t h2 = std::hash<int>{}(e.rowId);
            return h1 ^ (h2 << 32);
        }
    };

    // Returns all subtitle_text entries captured this session, in load order.
    // Each entry has {rowId, text}. Order matches the subtitle schedule order.
    struct SubtitleTextEntry {
        int rowId;
        std::string text;
    };
    std::vector<SubtitleTextEntry> GetSubtitleTexts() const;

    // Clear cached subtitle texts (call when subtitle player is destroyed)
    void ClearSubtitleTexts();

    // Latest yes_no_message lookup — cached when the game looks up dialog text.
    // YesNoHandler reads this to know what message the dialog is showing.
    struct YesNoEvent {
        int rowId = 0;
        std::string message;
    };
    YesNoEvent ConsumeYesNoMessage();  // Returns and clears the cached message

    // Scenario select events — cached when the game looks up scenario_select text.
    // ScenarioSelectHandler reads these to know prompt/description changes.
    struct ScenarioSelectEvent {
        int rowId = 0;
        std::string text;
    };
    std::vector<ScenarioSelectEvent> ConsumeScenarioSelectEvents();

private:
    TextCapture() = default;

    // Subtitle text entries captured during bulk load, in order
    mutable std::mutex m_subtitleMutex;
    std::vector<SubtitleTextEntry> m_subtitleTexts;

    // Latest yes_no_message lookup
    mutable std::mutex m_yesNoMutex;
    YesNoEvent m_latestYesNo;

    // Scenario select events (accumulated per frame, consumed by handler)
    mutable std::mutex m_scenarioMutex;
    std::vector<ScenarioSelectEvent> m_scenarioEvents;

    bool m_installed = false;

    // Original function pointer (trampoline)
    using LookupTextFunc = const char* (__fastcall*)(
        void* manager, const char* tableName, int rowId, unsigned int language);
    static inline LookupTextFunc s_originalLookupText = nullptr;
    static inline void* s_hookTarget = nullptr;

    // Per-frame capture buffer (written by hook, read by OnFrame)
    static inline std::mutex s_captureMutex;
    static inline std::vector<TextEntry> s_currentFrameCaptures;

    // Previous frame's entries (for diffing)
    std::unordered_set<TextEntry, TextEntryHash> m_previousFrame;
    bool m_firstFrame = true;

    // Our hooked function
    static const char* __fastcall HookedLookupText(
        void* manager, const char* tableName, int rowId, unsigned int language);
};
