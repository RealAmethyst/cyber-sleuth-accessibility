// CUiTalkWindow accessibility handler — NPC dialog boxes.
//
// Dialog body text comes from CSV message files (not LookupText), stored in
// a text widget at this+0xF0. Speaker names come from "CharName" via LookupText
// and are stored in a text widget at this+0x1E8.
//
// We read the char* at widget+0x28 for both speaker name and body text.
// Announcement triggers:
//   - State transitions to 7 (interactive): speak "speaker: body"
//   - Page changes while in state 7: speak new page text
//   - Body text changes while in state 7: speak updated text
//   - Body text changes while active flag is set (auto-advancing dialog)

#include "handlers/talk_handler.h"
#include "handlers/handler_utils.h"
#include "speech_manager.h"
#include "offsets.h"
#include "logger.h"

static constexpr int32_t STATE_INTERACTIVE = 7;

TalkHandler* TalkHandler::Get()
{
    static TalkHandler instance;
    return &instance;
}

uintptr_t TalkHandler::GetTickRVA() const
{
    return Offsets::FUNC_CUiTalkWindow_Tick;
}

void TalkHandler::OnScreenClosed()
{
    m_lastState = -1;
    m_lastPage = -1;
    m_lastBodyText.clear();
    m_lastSpeakerName.clear();
    m_announcedSpeaker.clear();
}

// SEH-safe probe: returns the text pointer if readable, nullptr otherwise.
// Must be in a separate function with no C++ objects for __try compatibility.
static const char* ProbeTextPointer(void* thisPtr, uintptr_t widgetOffset, uintptr_t textPtrOffset)
{
    __try {
        auto* base = reinterpret_cast<uint8_t*>(thisPtr);
        void* widget = *reinterpret_cast<void**>(base + widgetOffset);
        if (!widget) return nullptr;

        auto* widgetBytes = reinterpret_cast<uint8_t*>(widget);
        const char* text = *reinterpret_cast<const char**>(widgetBytes + textPtrOffset);
        if (!text || text[0] == '\0') return nullptr;

        // Force a read to validate the pointer
        volatile char c = text[0];
        (void)c;
        return text;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        return nullptr;
    }
}

// Read text from a widget pointer field in CUiTalkWindow.
// widgetOffset = offset to the widget ptr (e.g. 0xF0 for body, 0x1E8 for speaker).
std::string TalkHandler::ReadWidgetText(void* thisPtr, uintptr_t widgetOffset)
{
    const char* text = ProbeTextPointer(thisPtr, widgetOffset, Offsets::TalkWindow::WIDGET_TEXT_PTR);
    if (!text) return "";
    return std::string(text);
}

void TalkHandler::OnFrameInner(void* thisPtr)
{
    int32_t state = HandlerUtils::ReadMemory<int32_t>(thisPtr, Offsets::TalkWindow::SUB_STATE);
    int32_t page = HandlerUtils::ReadMemory<int32_t>(thisPtr, Offsets::TalkWindow::PAGE_INDEX);
    int32_t pageCount = HandlerUtils::ReadMemory<int32_t>(thisPtr, Offsets::TalkWindow::PAGE_COUNT);
    uint8_t active = HandlerUtils::ReadMemory<uint8_t>(thisPtr, Offsets::TalkWindow::ACTIVE_FLAG);

    // State transition to interactive — new dialog appeared
    if (state == STATE_INTERACTIVE && m_lastState != STATE_INTERACTIVE) {
        std::string body = ReadWidgetText(thisPtr, Offsets::TalkWindow::BODY_TEXT_WIDGET);
        std::string speaker = ReadWidgetText(thisPtr, Offsets::TalkWindow::SPEAKER_WIDGET);

        if (!body.empty()) {
            m_lastBodyText = body;
            m_lastSpeakerName = speaker;
            m_lastPage = page;
            AnnounceDialog(speaker, body, page, pageCount);
        }
    }

    // Already in interactive state — check for page or text changes
    if (state == STATE_INTERACTIVE && m_lastState == STATE_INTERACTIVE) {
        std::string body = ReadWidgetText(thisPtr, Offsets::TalkWindow::BODY_TEXT_WIDGET);

        if (!body.empty() && body != m_lastBodyText) {
            std::string speaker = ReadWidgetText(thisPtr, Offsets::TalkWindow::SPEAKER_WIDGET);
            m_lastBodyText = body;
            m_lastSpeakerName = speaker;
            m_lastPage = page;
            AnnounceDialog(speaker, body, page, pageCount);
        }
        else if (page != m_lastPage && m_lastPage >= 0) {
            // Same text widget but page changed (multi-page dialog)
            // Re-read since the widget content may update on next frame
            m_lastPage = page;
        }
    }

    // Auto-advancing dialog: active flag is set but state is NOT 7 (no player input needed).
    // Catches Message_Auto / MessageTalk_Auto / Message_C_Auto calls from scripts.
    if (active && state != STATE_INTERACTIVE) {
        std::string body = ReadWidgetText(thisPtr, Offsets::TalkWindow::BODY_TEXT_WIDGET);

        if (!body.empty() && body != m_lastBodyText) {
            std::string speaker = ReadWidgetText(thisPtr, Offsets::TalkWindow::SPEAKER_WIDGET);
            m_lastBodyText = body;
            m_lastSpeakerName = speaker;
            m_lastPage = page;

            Logger_Log("Talk", "Auto-advance dialog (state %d): [%s] %s",
                       state, speaker.c_str(), body.c_str());
            AnnounceDialog(speaker, body, page, pageCount);
        }
    }

    // Reset tracked text when dialog becomes inactive so next dialog starts fresh
    if (!active && m_lastBodyText.length() > 0) {
        m_lastBodyText.clear();
        m_lastSpeakerName.clear();
    }

    m_lastState = state;
}

void TalkHandler::AnnounceDialog(const std::string& speaker, const std::string& body,
                                  int32_t page, int32_t pageCount)
{
    std::string announcement;

    // Only include speaker name when it changes — mirrors the game's own
    // dedup at this+0xE8 (FUN_140410d50 skips name update for same speaker).
    bool speakerChanged = (speaker != m_announcedSpeaker);
    if (!speaker.empty() && speakerChanged) {
        announcement = speaker + ": " + body;
        m_announcedSpeaker = speaker;
    } else {
        announcement = body;
    }

    if (pageCount > 1) {
        announcement += " (page " + std::to_string(page + 1) +
                        " of " + std::to_string(pageCount) + ")";
    }

    Logger_Log("Talk", "Dialog: [%s] %s (page %d/%d, speaker %s)",
               speaker.c_str(), body.c_str(), page + 1, pageCount,
               speakerChanged ? "changed" : "same");

    SpeechManager::Get()->Speak(announcement, true);
}
