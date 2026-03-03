#pragma once

#include "handlers/tick_handler.h"
#include <cstdint>
#include <string>

// Handles accessibility for CUiYesNoWindow — Yes/No/Cancel dialog prompts.
//
// Message text comes from TextCapture (yes_no_message table lookups).
// Button labels are "Yes"/"No" from common_message:2100/2101 (or custom per
// yes_no_window_para Unk4/Unk5 columns).
//
// CUiYesNoWindow member layout (from Ghidra):
//   +0x0c: int32  state (0=idle,1=ready,2=opening,3=anim,4=interactive,5=closing,6=done)
//   +0x40: ptr    active window widget
//   +0x48: ptr    active Yes cursor widget
//   +0x50: ptr    active No cursor widget
//   +0x58: int32  style type (1=info, 2=system)
//   +0x80: byte   cancel enabled
//   +0x84: int32  result (0=none, 1=Yes, 2=No, 3=Cancel)
//   +0x88: byte   active/visible
//
// Speech rules:
//   Dialog opened (state -> 4): Speak(message, true) then Speak(option, false)
//   Cursor moved in state 4:   Speak(option, true)
//   Dialog closed (state 5/6): silence (old speech is stale)
class YesNoHandler : public TickHandler<YesNoHandler>
{
public:
    static YesNoHandler* Get();

    // TickHandler interface
    const char* GetHandlerName() const { return "YesNo"; }
    uintptr_t GetTickRVA() const;
    void OnFrameInner(void* thisPtr);
    void OnScreenClosed();

private:
    YesNoHandler() = default;
    friend class TickHandler<YesNoHandler>;

    int32_t m_lastState = -1;
    int32_t m_lastCursor = -1;
    bool m_dialogActive = false;
    std::string m_currentMessage;  // cached from TextCapture

    // State reading
    static int32_t ReadState(void* thisPtr);
    static int32_t ReadResult(void* thisPtr);
    static int32_t ReadCursor(void* thisPtr);

    // Text lookup
    std::string LookupButtonLabel(int32_t commonMessageId);

    // Announcements
    void AnnounceDialogOpened();
    void AnnounceCurrentOption(int32_t cursor);
};
