#pragma once

#include "hooks.h"
#include <atomic>
#include <cstdint>
#include <string>

// Handles accessibility for the title screen (CUiTitle).
//
// Strategy: Hook CUiTitle's tick function (vtable[3], RVA 0x4CC2F0) via
// MinHook ONLY to capture the this pointer. All state reading and speech
// happens in OnFrame() (SwapBuffers context) — never inside the tick detour.
//
// Title menu items are pre-baked textures (identical in all languages):
//   Cursor 0: "New Game"
//   Cursor 1: "Continue"
//   Cursor 2: "New Game +"  (locked when +0xc4 == 0)
//   Cursor 3: "Exit Game"
//
// CUiTitle member layout (from Ghidra decompilation):
//   +0xa8:  uint32 state index (state 12 = interactive menu)
//   +0xb8:  byte   done flag
//   +0xc4:  byte   locked-item flag (0 = "New Game +" is locked)
//   +0x114: int32  cursor index (0-3)
//   +0x124: int32  item count
class TitleHandler : public IFrameHandler
{
public:
    static TitleHandler* Get();

    // Install MinHook on CUiTitle's tick function.
    // Call AFTER hooks_init().
    void Install();
    void Uninstall();

    bool IsInstalled() const { return m_installed; }

    // IFrameHandler — all state reading and speech happens here.
    void OnFrame() override;

    // Inner logic called from SEH wrapper (public so the free function can call it)
    void OnFrameInner(void* thisPtr);

private:
    TitleHandler() = default;

    bool m_installed = false;
    int32_t m_lastCursorIndex = -1;
    uint32_t m_lastState = 0xFFFFFFFF;
    bool m_menuActive = false;

    // Tick detour stores this pointer atomically — OnFrame reads it.
    static inline std::atomic<void*> s_thisPtr{nullptr};

    // Original tick function pointer
    using TickFunc = void(__fastcall*)(void* thisPtr, void* param2);
    static inline TickFunc s_originalTick = nullptr;
    static inline void* s_hookTarget = nullptr;

    // Minimal tick detour — captures this pointer only.
    static void __fastcall HookedTick(void* thisPtr, void* param2);

    // Read game state (called from OnFrame only)
    static uint32_t ReadState(void* thisPtr);
    static int32_t ReadCursor(void* thisPtr);
    static int32_t ReadItemCount(void* thisPtr);

    // Get menu item text for a cursor position
    static const char* GetTitleMenuItem(int cursorIndex);

    // Announce helpers (called from OnFrame only)
    void AnnounceMenuOpened(void* thisPtr);
    void AnnounceCurrentItem(void* thisPtr);
};
