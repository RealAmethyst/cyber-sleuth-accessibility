// File: include/text_capture.h
// Purpose: Universal text capture via MinHook on LookupText (FUN_1401b9260).
//          Intercepts every MBE text table lookup the game makes, enabling
//          language-aware text detection without per-screen reverse engineering.
//
// Per-frame diffing detects NEW entries (appeared this frame, not last), which
// correspond to menu transitions, cursor moves, dialog advances, etc.
//
// Captured: all MBE table lookups — menu items, dialog, subtitles, yes/no prompts.
// NOT captured: title screen items and any other pre-baked texture text.
//
// Usage:
//   TextCapture::Get()->Install();        // after hooks_init()
//   RegisterFrameHandler(TextCapture::Get());
//   TextCapture::Get()->Uninstall();      // in onDisable()

class TextCapture : public IFrameHandler  // (line 26)
{
public:
    static TextCapture* Get();  // (line 29) — singleton accessor

    // Install MinHook on LookupText. Must be called after hooks_init().
    void Install();    // (line 33)
    void Uninstall();  // (line 34)

    bool IsInstalled() const;  // (line 36)

    // IFrameHandler: performs frame diffing — compares current frame's lookups
    // against the previous frame and logs any NEW entries.
    void OnFrame() override;  // (line 41)

    // A single captured text lookup (table name + row ID uniquely identify an entry).
    struct TextEntry {  // (line 44)
        std::string tableName;
        int rowId;
        std::string text;
        bool operator==(const TextEntry& other) const;  // (line 49) — equality by tableName+rowId only
    }

    // Hash functor for TextEntry (hashes tableName ^ rowId).
    struct TextEntryHash {  // (line 54)
        size_t operator()(const TextEntry& e) const;  // (line 55)
    }

    // Entry type used when returning ordered subtitle texts.
    struct SubtitleTextEntry {  // (line 64)
        int rowId;
        std::string text;
    }

    // Returns all subtitle_text entries captured this session, in load order.
    // Order matches the subtitle schedule (bulk-loaded in one call by the game).
    std::vector<SubtitleTextEntry> GetSubtitleTexts() const;  // (line 68)

    // Clear the cached subtitle texts (call when the subtitle player is destroyed).
    void ClearSubtitleTexts();  // (line 71)

    // Cached latest yes_no_message lookup.
    // YesNoHandler calls this to learn what message the dialog is showing.
    struct YesNoEvent {  // (line 75)
        int rowId = 0;
        std::string message;
    }
    // Returns and clears the cached yes_no_message (consume-once pattern).
    YesNoEvent ConsumeYesNoMessage();  // (line 79)

    // Accumulated scenario_select lookups, consumed by ScenarioSelectHandler.
    struct ScenarioSelectEvent {  // (line 83)
        int rowId = 0;
        std::string text;
    }
    // Returns and clears all accumulated scenario_select events for this frame.
    std::vector<ScenarioSelectEvent> ConsumeScenarioSelectEvents();  // (line 87)

private:
    TextCapture() = default;  // (line 90)

    mutable std::mutex m_subtitleMutex;              // (line 93)
    std::vector<SubtitleTextEntry> m_subtitleTexts;  // (line 94)

    mutable std::mutex m_yesNoMutex;  // (line 97)
    YesNoEvent m_latestYesNo;         // (line 98)

    mutable std::mutex m_scenarioMutex;               // (line 101)
    std::vector<ScenarioSelectEvent> m_scenarioEvents; // (line 102)

    bool m_installed = false;  // (line 104)

    // Typedef for the original LookupText function (used as trampoline by MinHook).
    using LookupTextFunc = const char* (__fastcall*)(
        void* manager, const char* tableName, int rowId, unsigned int language);  // (line 107-108)
    static inline LookupTextFunc s_originalLookupText = nullptr;  // (line 109)
    static inline void* s_hookTarget = nullptr;                   // (line 110)

    // Shared capture buffer: written by hook (any thread), read by OnFrame (SwapBuffers thread).
    static inline std::mutex s_captureMutex;                        // (line 113)
    static inline std::vector<TextEntry> s_currentFrameCaptures;    // (line 114)

    // Previous frame's entry set, used for NEW/GONE diffing.
    std::unordered_set<TextEntry, TextEntryHash> m_previousFrame;  // (line 117)
    bool m_firstFrame = true;                                       // (line 118)

    // The MinHook detour — replaces LookupText, records entry, then calls original.
    static const char* __fastcall HookedLookupText(
        void* manager, const char* tableName, int rowId, unsigned int language);  // (line 121-122)
}
