// File: include/handlers/subtitle_handler.h
// Purpose: Accessibility handler for cutscene subtitles (Vista cinematic system).
//
// NO hooks are installed — purely polls the subtitle loader singleton each frame.
// The loader at DAT_140f205d0 holds a cue vector with a cursor the game's own
// update function advances based on playback time.
//
// Per-frame algorithm:
//   1. Read loader state (+0x4C) — 2 = playing
//   2. Compute cue index: (cursor - begin) / sizeof(entry)
//   3. If index changed, walk: cursor -> cueStruct[0] -> rowData -> +0x0C = SubtitleTextID
//   4. Call LookupText("subtitle_text", textId, language)
//   5. Speak the result (interrupt=true)
//
// All text from game tables at runtime — zero embedded data, zero generated files.

class SubtitleHandler : public IFrameHandler  // (line 23)
{
public:
    static SubtitleHandler* Get();  // (line 25) — singleton accessor

    // Install: resolves LookupText function pointer and stores module base.
    // Does NOT install any MinHook — pure polling only.
    void Install();    // (line 28)
    void Uninstall();  // (line 29)

    bool IsInstalled() const;  // (line 31)

    // IFrameHandler: polls loader state and cue index every frame; speaks on change.
    void OnFrame() override;  // (line 34)

private:
    SubtitleHandler() = default;  // (line 37)

    bool m_installed = false;        // (line 39)
    uintptr_t m_moduleBase = 0;      // (line 40)

    bool m_wasPlaying = false;       // (line 43) — was loader state == 2 last frame?
    int m_lastCueIndex = -1;         // (line 44) — last spoken cue index (change detection)
    std::string m_lastSpokenText;    // (line 45) — deduplication: skip identical consecutive cues

    // Cached LookupText function pointer (resolved once in Install).
    using LookupTextFn = const char* (__fastcall*)(
        void* manager, const char* tableName, int rowId, unsigned int language);  // (line 48-49)
    LookupTextFn m_lookupText = nullptr;  // (line 50)

    // All helpers are protected with SEH against bad pointer reads.
    void* GetLoader() const;                    // (line 53) — read DAT_SubtitleLoader ptr
    int GetLoaderState() const;                 // (line 54) — read loader+0x4C
    int GetCueIndex() const;                    // (line 55) — (cursor - begin) / entry_size
    int GetCueTextId(int cueIndex) const;       // (line 56) — walk cue chain -> SubtitleTextID
    const char* LookupSubtitleText(int textId) const;  // (line 57) — LookupText("subtitle_text", ...)
}
