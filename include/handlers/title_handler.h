#pragma once

#include "handlers/tick_handler.h"
#include <cstdint>
#include <string>

// Handles accessibility for the title screen (CUiTitle).
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
class TitleHandler : public TickHandler<TitleHandler>
{
public:
    static TitleHandler* Get();

    // TickHandler interface
    const char* GetHandlerName() const { return "Title"; }
    uintptr_t GetTickRVA() const;
    void OnFrameInner(void* thisPtr);
    void OnScreenClosed();

private:
    TitleHandler() = default;
    friend class TickHandler<TitleHandler>;

    int32_t m_lastCursorIndex = -1;
    uint32_t m_lastState = 0xFFFFFFFF;
    bool m_menuActive = false;

    // Read game state
    static uint32_t ReadState(void* thisPtr);
    static int32_t ReadCursor(void* thisPtr);
    static int32_t ReadItemCount(void* thisPtr);

    static const char* GetTitleMenuItem(int cursorIndex);

    void AnnounceMenuOpened(void* thisPtr);
    void AnnounceCurrentItem(void* thisPtr);
};
