#pragma once

#include "hooks.h"
#include <atomic>
#include <cstdint>
#include <string>

// Handles accessibility for CUiScenarioSelect — campaign selection screen.
//
// Appears after selecting "New Game" and watching the intro cutscene.
// Player chooses between Cyber Sleuth (CS) and Hacker's Memory (HM) campaigns.
//
// Strategy: Hook CUiScenarioSelect tick (vtable[3], RVA 0x4c89a0) via MinHook
// to capture the this pointer. Text content from TextCapture.
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
class ScenarioSelectHandler : public IFrameHandler
{
public:
    static ScenarioSelectHandler* Get();

    void Install();
    void Uninstall();

    bool IsInstalled() const { return m_installed; }

    // IFrameHandler
    void OnFrame() override;

    // Public so the SEH wrapper can call it
    void OnFrameInner(void* thisPtr);

private:
    ScenarioSelectHandler() = default;

    bool m_installed = false;
    bool m_tickActive = false;       // tick is firing (object exists)
    bool m_interactive = false;      // prompt text received (menu shown)
    int32_t m_lastCursor = -1;
    int m_itemCount = 2;             // always 2 for normal gameplay (CS + HM)

    // Tick detour stores this pointer atomically
    static inline std::atomic<void*> s_thisPtr{nullptr};
    static inline std::atomic<bool> s_tickFired{false};

    using TickFunc = void(__fastcall*)(void* thisPtr, void* param2);
    static inline TickFunc s_originalTick = nullptr;
    static inline void* s_hookTarget = nullptr;

    static void __fastcall HookedTick(void* thisPtr, void* param2);

    // State reading
    static int32_t ReadCursor(void* thisPtr);
    static int32_t ReadItemId(void* thisPtr, int32_t cursor);

    // Map scenario_select row ID to campaign name
    static const char* GetCampaignName(int rowId);

    // Look up description text from scenario_select table
    std::string LookupDescription(int rowId);

    // Announcements
    void AnnounceItem(int32_t cursor, void* thisPtr, bool interrupt);
};
