#pragma once

#include "hooks.h"
#include <cstdint>
#include <string>

// Handles accessibility for the in-game main menu (CUiMainMenu).
//
// Strategy: Hook CUiMainMenu's tick function (vtable[3], RVA 0x4b6270) via
// MinHook to get called every frame while the menu is active. Track cursor
// position and announce changes via SpeechManager using the game's text
// lookup API (FUN_1401b9260) for dynamic, language-aware text.
//
// Text lookup: LookupText(textManager, "main_menu", cursor+1, language)
// Returns the localized menu item name (Organize, Items, Status, etc.)
//
// CUiMainMenu member layout (from Ghidra decompilation):
//   +0x27D8: int32 cursor index (0-7, wraps)
//   +0x2928: int16 state (0-5 state machine)
//   +0x2828: byte  done flag (menu closing)
//   +0x2A08: int32 item count (max 8)
//   +0x160:  item slots (8 entries, stride 0x50)
//   +0x3E0:  category entries (3 entries, stride 0x300)
//   +0x29F0: int32 selected category index
//   +0x29F8: int32 selected sub-item index
class MainMenuHandler : public IFrameHandler
{
public:
    static MainMenuHandler* Get();

    // Install MinHook on CUiMainMenu's tick function.
    // Call AFTER hooks_init() (MinHook must be initialized).
    void Install();
    void Uninstall();

    bool IsInstalled() const { return m_installed; }

    // IFrameHandler — called every frame from SwapBuffers
    void OnFrame() override;

private:
    MainMenuHandler() = default;

    // State tracking
    bool m_installed = false;
    void* m_lastThisPtr = nullptr;
    int32_t m_lastCursorIndex = -1;
    int16_t m_lastState = -1;
    bool m_menuActive = false;

    // Original tick function pointer (saved during hook)
    using TickFunc = void(__fastcall*)(void* thisPtr, void* param2);
    static inline TickFunc s_originalTick = nullptr;
    static inline void* s_hookTarget = nullptr;

    // Game function pointers (resolved once at install)
    using GetTextTableManagerFunc = void* (__fastcall*)();
    using LookupTextFunc = const char* (__fastcall*)(void* manager, const char* tableName,
                                                      int rowId, unsigned int language);

    static inline GetTextTableManagerFunc s_getTextTableManager = nullptr;
    static inline LookupTextFunc s_lookupText = nullptr;

    // Our hooked tick function
    static void __fastcall HookedTick(void* thisPtr, void* param2);

    // Read game state from the CUiMainMenu instance
    int32_t ReadCursor(void* thisPtr);
    int16_t ReadState(void* thisPtr);
    int32_t ReadItemCount(void* thisPtr);

    // Look up menu item text dynamically from game's text tables
    std::string LookupMenuItemText(int cursorIndex);

    // Announce helpers
    void CheckStateChanges(void* thisPtr);
    void AnnounceCurrentItem(void* thisPtr);
    void AnnounceMenuOpened(void* thisPtr);
};
