#pragma once

#include "hooks.h"

#include <cstdint>
#include <string>

// Handles accessibility for cutscene subtitles (Vista cinematic system).
//
// NO hooks needed — purely polls the subtitle loader singleton each frame.
// The loader (at DAT_140f205d0) contains a cue vector with a cursor that
// the game's own update function advances based on playback time.
//
// Each frame:
//   1. Read loader state (+0x4C) — 2 = playing
//   2. Read cue cursor (+0x98) vs begin (+0x80) → cue index
//   3. If index changed, navigate cue → rowNode → columnData → SubtitleTextID
//   4. Call game's LookupText("subtitle_text", textId, language)
//   5. Speak the result
//
// Zero embedded game data, zero generated files. All text comes from the
// game's own text tables at runtime.
class SubtitleHandler : public IFrameHandler
{
public:
    static SubtitleHandler* Get();

    void Install();
    void Uninstall();

    bool IsInstalled() const { return m_installed; }

    // IFrameHandler — called every frame from SwapBuffers.
    void OnFrame() override;

private:
    SubtitleHandler() = default;

    bool m_installed = false;
    uintptr_t m_moduleBase = 0;

    // Tracking state
    bool m_wasPlaying = false;      // Was loader state == 2 last frame?
    int m_lastCueIndex = -1;        // Last spoken cue index
    std::string m_lastSpokenText;   // Dedup (same text across frames)

    // Helpers — all protected with SEH
    void* GetLoader() const;
    int GetLoaderState() const;
    int GetCueIndex() const;
    int GetCueTextId(int cueIndex) const;
    const char* LookupSubtitleText(int textId) const;
};
