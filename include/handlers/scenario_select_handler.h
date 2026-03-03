#pragma once

#include "handlers/tick_handler.h"
#include <cstdint>
#include <string>

// Handles accessibility for CUiScenarioSelect — campaign selection screen.
//
// Appears after selecting "New Game" and watching the intro cutscene.
// Player chooses between Cyber Sleuth (CS) and Hacker's Memory (HM) campaigns.
//
// IMPORTANT: The tick fires during the entire cutscene, not just the interactive
// menu. We use TextCapture's scenario_select:1 (prompt text) as the signal that
// the interactive phase has started.
//
// Memory layout (from dumps):
//   +0xD0: int32 item ID array (scenario_select row IDs, e.g. {2, 3})
//   +0xE0: int32 cursor (1-based index into item array)
//
// Campaign names are pre-baked textures (brand names, same in all languages):
//   Row ID 2 = "Hacker's Memory"
//   Row ID 3 = "Cyber Sleuth"
class ScenarioSelectHandler : public TickHandler<ScenarioSelectHandler>
{
public:
    static ScenarioSelectHandler* Get();

    // TickHandler interface
    const char* GetHandlerName() const { return "ScenarioSelect"; }
    uintptr_t GetTickRVA() const;
    void OnFrameInner(void* thisPtr);
    void OnScreenClosed();

    // Override OnFrame for custom pre-tick event consumption
    void OnFrame() override;

private:
    ScenarioSelectHandler() = default;
    friend class TickHandler<ScenarioSelectHandler>;

    bool m_interactive = false;      // prompt text received (menu shown)
    int32_t m_lastCursor = -1;
    int m_itemCount = 2;             // always 2 for normal gameplay (CS + HM)

    // State reading
    static int32_t ReadCursor(void* thisPtr);
    static int32_t ReadItemId(void* thisPtr, int32_t cursor);

    static const char* GetCampaignName(int rowId);
    std::string LookupDescription(int rowId);
    void AnnounceItem(int32_t cursor, void* thisPtr, bool interrupt);
};
