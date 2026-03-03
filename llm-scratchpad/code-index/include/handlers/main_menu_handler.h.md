// File: include/handlers/main_menu_handler.h
// Purpose: Accessibility handler for the in-game main menu (CUiMainMenu).
//
// Hooks CUiMainMenu's tick function (vtable[3], RVA 0x4b6270) via MinHook.
// Tick detour is minimal (atomic this-pointer store only).
// All state reading and speech happen in OnFrame() (SwapBuffers context).
// Uses the game's LookupText API for dynamic, language-aware menu item names.
//
// Text: LookupText(manager, "main_menu", cursor+1, language)
//   cursor 0 -> row 1 "Organize"
//   cursor 1 -> row 2 "Items"
//   cursor 2 -> row 3 "Status"
//   cursor 3 -> row 4 "Options"
//   cursor 4 -> row 5 "Save/Load"
//   cursor 5 -> row 6 "Sort Digimon"
//   cursor 6 -> row 7 "Farm"
//   cursor 7 -> row 8 "Exit"
//
// CUiMainMenu member layout (from Ghidra):
//   +0x27D8: int32 cursor index (0-7, wraps)
//   +0x2928: int16 state (0=closed, 3=interactive, 5=closing)
//   +0x2828: byte  done flag
//   +0x2A08: int32 item count (max 8)

class MainMenuHandler : public IFrameHandler  // (line 26)
{
public:
    static MainMenuHandler* Get();  // (line 29) — singleton accessor

    // Install MinHook on CUiMainMenu's tick. Call after hooks_init().
    void Install();    // (line 33)
    void Uninstall();  // (line 34)

    bool IsInstalled() const;  // (line 36)

    // IFrameHandler: reads state and speaks changes. Called every frame from SwapBuffers.
    void OnFrame() override;  // (line 39)

private:
    MainMenuHandler() = default;  // (line 42)

    bool m_installed = false;        // (line 45)
    void* m_lastThisPtr = nullptr;   // (line 46)
    int32_t m_lastCursorIndex = -1;  // (line 47)
    int16_t m_lastState = -1;        // (line 48)
    bool m_menuActive = false;       // (line 49)

    using TickFunc = void(__fastcall*)(void* thisPtr, void* param2);  // (line 52)
    static inline TickFunc s_originalTick = nullptr;  // (line 53)
    static inline void* s_hookTarget = nullptr;       // (line 54)

    using GetTextTableManagerFunc = void* (__fastcall*)();  // (line 57)
    using LookupTextFunc = const char* (__fastcall*)(void* manager, const char* tableName,
                                                      int rowId, unsigned int language);  // (line 58-59)
    static inline GetTextTableManagerFunc s_getTextTableManager = nullptr;  // (line 61)
    static inline LookupTextFunc s_lookupText = nullptr;                    // (line 62)

    // Minimal tick detour — calls original then atomically stores this pointer.
    static void __fastcall HookedTick(void* thisPtr, void* param2);  // (line 65)

    // SEH wrapper: calls HookedTickInner inside __try/__except (no C++ objects in scope).
    static void HookedTickSEH(void* thisPtr);    // (line 66)

    // Inner logic for the tick detour (C++ objects allowed here).
    static void HookedTickInner(void* thisPtr);  // (line 67)

    // State readers — all called from OnFrame only.
    int32_t ReadCursor(void* thisPtr);     // (line 70)
    int16_t ReadState(void* thisPtr);      // (line 71)
    int32_t ReadItemCount(void* thisPtr);  // (line 72)

    // Look up the localized name for a given 0-based cursor index via LookupText.
    std::string LookupMenuItemText(int cursorIndex);  // (line 75)

    // Detect state transitions and trigger the appropriate announce helpers.
    void CheckStateChanges(void* thisPtr);   // (line 78)

    // Speak the item at the current cursor position: "name, N of M"
    void AnnounceCurrentItem(void* thisPtr);  // (line 79)

    // Speak menu window name then queue first item (called when state -> 3).
    void AnnounceMenuOpened(void* thisPtr);   // (line 80)
}
