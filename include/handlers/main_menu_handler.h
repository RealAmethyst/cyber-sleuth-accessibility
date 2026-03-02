#pragma once

#include "hooks.h"
#include <cstdint>
#include <string>

// Handles accessibility for the in-game main menu (CUiMainMenu).
//
// Strategy: Hook CUiMainMenu's update virtual (vtable index 2) to get
// called every frame while the menu is active. Track cursor position
// and announce changes via SpeechManager.
//
// The vtable address is known from RTTI analysis (Offsets::VTABLE_CUiMainMenu).
// Member offsets for cursor index, item count, etc. need runtime discovery
// (attach debugger, break on vtable[2], inspect 'this' pointer).
//
// This is the proof-of-concept handler — once the pattern works here,
// we replicate it for all other CUi* classes.
class MainMenuHandler : public IFrameHandler
{
public:
    static MainMenuHandler* Get();

    // Install vtable hook on the CUiMainMenu class.
    // Called once the menu instance is found at runtime.
    void Install(void* menuInstance);
    void Uninstall();

    bool IsInstalled() const { return m_installed; }

    // IFrameHandler — called every frame from SwapBuffers
    void OnFrame() override;

private:
    MainMenuHandler() = default;

    // State tracking
    bool m_installed = false;
    void* m_menuInstance = nullptr;
    int32_t m_lastCursorIndex = -1;
    bool m_menuNameAnnounced = false;
    bool m_queueNextAnnouncement = false;

    // Original vtable function pointer (saved during hook)
    using UpdateFunc = void(__thiscall*)(void* thisPtr);
    static inline UpdateFunc s_originalUpdate = nullptr;

    // Our hooked update function
    static void __fastcall HookedUpdate(void* thisPtr);

    // Announce helpers
    void CheckStateChanges();
    void AnnounceCurrentItem();
};
