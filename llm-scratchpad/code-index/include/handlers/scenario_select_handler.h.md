// File: include/handlers/scenario_select_handler.h
// Purpose: Accessibility handler for CUiScenarioSelect — campaign selection screen.
//
// Shown after "New Game" intro cutscene; player picks Cyber Sleuth (CS) or
// Hacker's Memory (HM).
//
// Hooks CUiScenarioSelect tick (vtable[3], RVA 0x4c89a0) via MinHook.
// Tick detour is minimal (atomic this-pointer store + tickFired flag only).
// All logic in OnFrame() (SwapBuffers context).
//
// IMPORTANT: The tick fires during the cutscene AND the interactive menu.
//            TextCapture's scenario_select:1 (prompt text) signals interactive phase start.
//
// Memory layout (from memory dumps):
//   +0xD0: int32 array of scenario_select row IDs (e.g. {2, 3})
//   +0xE0: int32 cursor (1-based, 1=first item)
//
// Campaign names are pre-baked brand textures (same in all languages — hardcoded):
//   Row ID 2 -> "Hacker's Memory"
//   Row ID 3 -> "Cyber Sleuth"

class ScenarioSelectHandler : public IFrameHandler  // (line 27)
{
public:
    static ScenarioSelectHandler* Get();  // (line 30) — singleton accessor

    void Install();    // (line 32)
    void Uninstall();  // (line 33)

    bool IsInstalled() const;  // (line 35)

    // IFrameHandler: reads cursor and TextCapture events, speaks changes.
    void OnFrame() override;  // (line 38)

    // Public so the SEH wrapper free function can call it without friendship.
    void OnFrameInner(void* thisPtr);  // (line 41)

private:
    ScenarioSelectHandler() = default;  // (line 44)

    bool m_installed = false;    // (line 46)
    bool m_tickActive = false;   // (line 47) — tick is firing (object exists in memory)
    bool m_interactive = false;  // (line 48) — prompt text received; interactive menu is visible
    int32_t m_lastCursor = -1;   // (line 49)
    int m_itemCount = 2;         // (line 50) — always 2 for normal gameplay (CS + HM)

    static inline std::atomic<void*> s_thisPtr{nullptr};  // (line 53) — written by tick detour
    static inline std::atomic<bool> s_tickFired{false};   // (line 54) — signals OnFrame that tick ran

    using TickFunc = void(__fastcall*)(void* thisPtr, void* param2);  // (line 56)
    static inline TickFunc s_originalTick = nullptr;  // (line 57)
    static inline void* s_hookTarget = nullptr;       // (line 58)

    // Minimal tick detour: calls original, stores this pointer, sets tickFired.
    static void __fastcall HookedTick(void* thisPtr, void* param2);  // (line 60)

    using GetTextTableManagerFunc = void* (*)();  // (line 63)
    using LookupTextFunc = const char* (*)(void* manager, const char* tableName,
                                           int rowId, unsigned int language);  // (line 64-65)
    static inline GetTextTableManagerFunc s_getTextTableManager = nullptr;  // (line 66)
    static inline LookupTextFunc s_lookupText = nullptr;                    // (line 67)

    // Read the 1-based cursor index from memory.
    static int32_t ReadCursor(void* thisPtr);  // (line 70)

    // Read the scenario_select row ID for a given 1-based cursor position.
    static int32_t ReadItemId(void* thisPtr, int32_t cursor);  // (line 71)

    // Map a scenario_select row ID to a hardcoded campaign name string.
    // (Names are pre-baked textures; cannot be read from text tables.)
    static const char* GetCampaignName(int rowId);  // (line 74)

    // Look up the description text for a row ID from the scenario_select MBE table.
    std::string LookupDescription(int rowId);  // (line 77)

    // Speak the item at `cursor`: "campaign name, description, N of M".
    // interrupt=true for cursor movement, false when queuing after the prompt.
    void AnnounceItem(int32_t cursor, void* thisPtr, bool interrupt);  // (line 80)
}
