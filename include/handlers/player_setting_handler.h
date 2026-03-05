#pragma once

#include "handlers/tick_handler.h"
#include <cstdint>
#include <string>

// Handles accessibility for CUiFirstSequence — the new game naming/gender screen.
//
// In Cyber Sleuth, the naming flow is managed by CUiFirstSequence (NOT CUiPlayerSetting,
// which is only used for Hacker's Memory's avatar customization).
//
// CUiFirstSequence has a 31-state machine at this+0x108 (function table at this+0x10).
// Key interactive states:
//   State 10: Gender selection (Male/Female picker)
//   State 6:  Login form (Gender field, Name field, Log In button)
class PlayerSettingHandler : public TickHandler<PlayerSettingHandler>
{
public:
    static PlayerSettingHandler* Get();

    // TickHandler interface
    const char* GetHandlerName() const { return "FirstSeq"; }
    uintptr_t GetTickRVA() const;
    void OnFrameInner(void* thisPtr);
    void OnScreenClosed();

private:
    PlayerSettingHandler() = default;
    friend class TickHandler<PlayerSettingHandler>;

    int32_t m_lastState = -1;
    int32_t m_lastGender = -1;      // gender selection (1=male, 2=female)
    int32_t m_lastCursor = -1;      // cursor on the login form (0=gender, 1=name, 2=login)
    bool m_screenOpened = false;
};
