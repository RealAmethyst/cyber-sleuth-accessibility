#pragma once

#include "handlers/tick_handler.h"
#include <cstdint>
#include <string>

// Handles accessibility for CUiTalkWindow — NPC dialog boxes.
//
// Dialog text does NOT go through LookupText (TextCapture won't see it).
// Instead, it comes from CSV message files loaded via Talk.Load() in Squirrel.
// The text is stored in a text widget at this+0xF0, with the char* at widget+0x28.
// Speaker names DO go through LookupText ("CharName" table).
//
// State machine has 13 states; state 7 = interactive (dialog visible).
// We announce speaker name + dialog body when state transitions to 7,
// and track page changes for multi-page dialogs.
class TalkHandler : public TickHandler<TalkHandler>
{
public:
    static TalkHandler* Get();

    // TickHandler interface
    const char* GetHandlerName() const { return "Talk"; }
    uintptr_t GetTickRVA() const;
    void OnFrameInner(void* thisPtr);
    void OnScreenClosed();

private:
    TalkHandler() = default;
    friend class TickHandler<TalkHandler>;

    int32_t m_lastState = -1;
    int32_t m_lastPage = -1;
    std::string m_lastBodyText;
    std::string m_lastSpeakerName;
    std::string m_announcedSpeaker;  // tracks last spoken speaker for dedup

    // Text reading from widget memory
    static std::string ReadWidgetText(void* thisPtr, uintptr_t widgetOffset);

    // Announcements
    void AnnounceDialog(const std::string& speaker, const std::string& body,
                        int32_t page, int32_t pageCount);
};
