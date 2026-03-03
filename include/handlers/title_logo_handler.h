#pragma once

#include "handlers/tick_handler.h"
#include <cstdint>

// Handles accessibility for the pre-title logo sequence (CUiTitleLogo).
//
// The game shows 8 screens before the title menu:
//   State  5: Bandai Namco Entertainment
//   State  8: Media.Vision
//   State 11: ha.n.d.
//   State 14: Sound Prestige (Prestige Sound)
//   State 17: CRI Ware
//   State 20: Disclaimer (warning + fiction notice)
//   State 23: Digimon 20th Anniversary
//   State 26: Press Any Button
//
// All content is pre-baked textures — text varies by language for
// disclaimer and "Press Any Button", company names are universal.
// State field at +0xF8 (int32), 29 total states with function table dispatch.
class TitleLogoHandler : public TickHandler<TitleLogoHandler>
{
public:
    static TitleLogoHandler* Get();

    // TickHandler interface
    const char* GetHandlerName() const { return "TitleLogo"; }
    uintptr_t GetTickRVA() const;
    void OnFrameInner(void* thisPtr);
    void OnScreenClosed();

private:
    TitleLogoHandler() = default;
    friend class TickHandler<TitleLogoHandler>;

    uint32_t m_lastState = 0xFFFFFFFF;

    static uint32_t ReadState(void* thisPtr);
    static const char* GetAnnouncementForState(uint32_t state);
};
