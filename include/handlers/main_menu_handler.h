#pragma once

#include "hooks.h"
#include <atomic>
#include <cstdint>
#include <string>

// Handles accessibility for the in-game main menu (CUiMainMenu).
//
// Strategy: Hook CUiMainMenu's tick function (vtable[3], RVA 0x4b6270) via
// MinHook ONLY to capture the this pointer. All state reading and speech
// happens in OnFrame() (SwapBuffers context).
//
// Text lookup: LookupText(textManager, "main_menu", cursor+1, language)
// Returns the localized menu item name (Organize, Items, Status, etc.)
//
// Speech rules:
//   Menu opened (state -> 3): Speak("Main Menu", true) then Speak(item, false)
//   Cursor moved in state 3:  Speak(item, true)
class MainMenuHandler : public IFrameHandler
{
public:
    static MainMenuHandler* Get();

    void Install();
    void Uninstall();

    bool IsInstalled() const { return m_installed; }

    // IFrameHandler — called every frame from SwapBuffers
    void OnFrame() override;

    // Called from SEH wrapper free function
    void OnFrameInner(void* thisPtr);

private:
    MainMenuHandler() = default;

    // State tracking
    bool m_installed = false;
    int32_t m_lastCursorIndex = -1;
    int16_t m_lastState = -1;
    bool m_menuActive = false;

    // Atomic this-pointer captured by tick detour
    static inline std::atomic<void*> s_thisPtr{nullptr};
    static inline std::atomic<bool> s_tickFired{false};

    // Original tick function pointer (saved during hook)
    using TickFunc = void(__fastcall*)(void* thisPtr, void* param2);
    static inline TickFunc s_originalTick = nullptr;
    static inline void* s_hookTarget = nullptr;

    // Our hooked tick function — MINIMAL, only captures this pointer
    static void __fastcall HookedTick(void* thisPtr, void* param2);

    // Read game state from the CUiMainMenu instance
    int32_t ReadCursor(void* thisPtr);
    int16_t ReadState(void* thisPtr);
    int32_t ReadItemCount(void* thisPtr);

    // Look up menu item text dynamically from game's text tables
    std::string LookupMenuItemText(int cursorIndex);

    // Announce current item: "itemName, N of M"
    void AnnounceItem(void* thisPtr, bool interrupt);
};
