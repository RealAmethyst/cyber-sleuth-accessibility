// CUiYesNoWindow accessibility handler — Yes/No/Cancel dialog prompts.
//
// Message text is captured from TextCapture (yes_no_message table lookups).
// The game looks up the message text once when the dialog opens, so we cache
// it from TextCapture and announce when state transitions to interactive.
//
// Cursor tracking: byte at this+0x81 (0=No/bottom, 1=Yes/top).
// Button labels read dynamically from common_message table using IDs stored
// at this+0x78 (Yes label) and this+0x7c (No label).

#include "handlers/yesno_handler.h"
#include "text_capture.h"
#include "speech_manager.h"
#include "game_text.h"
#include "offsets.h"
#include "logger.h"

// State constants (from Ghidra decompilation of tick function)
static constexpr int32_t STATE_INTERACTIVE = 4;
static constexpr int32_t STATE_CLOSING     = 5;
static constexpr int32_t STATE_DONE        = 6;

YesNoHandler* YesNoHandler::Get()
{
    static YesNoHandler instance;
    return &instance;
}

uintptr_t YesNoHandler::GetTickRVA() const
{
    return Offsets::FUNC_CUiYesNoWindow_Tick;
}

void YesNoHandler::OnScreenClosed()
{
    m_lastState = -1;
    m_lastCursor = -1;
    m_dialogActive = false;
    m_currentMessage.clear();
}

// ============================================================
// OnFrameInner — handler-specific per-frame logic
// ============================================================

void YesNoHandler::OnFrameInner(void* thisPtr)
{
    int32_t state = ReadState(thisPtr);

    // Check TextCapture for a new yes_no_message (appears when dialog is being set up)
    auto yesNoEvent = TextCapture::Get()->ConsumeYesNoMessage();
    if (!yesNoEvent.message.empty()) {
        m_currentMessage = yesNoEvent.message;
        Logger_Log("YesNo", "Captured message [%d]: %s",
                   yesNoEvent.rowId, m_currentMessage.c_str());
    }

    // Detect transition to interactive state
    if (state == STATE_INTERACTIVE && m_lastState != STATE_INTERACTIVE) {
        m_dialogActive = true;
        m_lastCursor = -1;  // Reset BEFORE announcing so first item queues
        AnnounceDialogOpened();
    }

    // Track cursor during interactive state
    if (state == STATE_INTERACTIVE) {
        int32_t cursor = ReadCursor(thisPtr);
        if (cursor >= 0 && cursor != m_lastCursor && m_lastCursor >= 0) {
            AnnounceCurrentOption(cursor);
        }
        if (cursor >= 0) {
            m_lastCursor = cursor;
        }
    }

    // Detect dialog closing
    if ((state == STATE_CLOSING || state == STATE_DONE) &&
        m_lastState == STATE_INTERACTIVE) {
        int32_t result = ReadResult(thisPtr);
        Logger_Log("YesNo", "Dialog closing with result: %d", result);
    }

    m_lastState = state;
}

// ============================================================
// State reading helpers
// ============================================================

int32_t YesNoHandler::ReadState(void* thisPtr)
{
    auto* ptr = reinterpret_cast<uint8_t*>(thisPtr);
    return *reinterpret_cast<int32_t*>(ptr + Offsets::YesNoWindow::STATE);
}

int32_t YesNoHandler::ReadResult(void* thisPtr)
{
    auto* ptr = reinterpret_cast<uint8_t*>(thisPtr);
    return *reinterpret_cast<int32_t*>(ptr + Offsets::YesNoWindow::RESULT);
}

int32_t YesNoHandler::ReadCursor(void* thisPtr)
{
    auto* ptr = reinterpret_cast<uint8_t*>(thisPtr);
    uint8_t cursor = *(ptr + Offsets::YesNoWindow::CURSOR_INDEX);

    // Sanity check — cursor should be 0 (No/bottom) or 1 (Yes/top)
    if (cursor > 1) return -1;

    return static_cast<int32_t>(cursor);
}

// ============================================================
// Text lookup
// ============================================================

std::string YesNoHandler::LookupButtonLabel(int32_t commonMessageId)
{
    if (commonMessageId <= 0) return "";
    return GameText_Lookup("common_message", commonMessageId);
}

// ============================================================
// Announcements
// ============================================================

void YesNoHandler::AnnounceDialogOpened()
{
    Logger_Log("YesNo", "Dialog opened (state -> interactive)");

    // 1. Announce the message (interrupt)
    if (!m_currentMessage.empty()) {
        SpeechManager::Get()->Speak(m_currentMessage, true);
    } else {
        SpeechManager::Get()->Speak("Yes or No", true);
        Logger_Log("YesNo", "WARNING: No message text captured for this dialog");
    }

    // 2. Announce the default option (queue)
    void* thisPtr = LoadThisPtr();
    int32_t cursor = -1;
    if (thisPtr) {
        cursor = ReadCursor(thisPtr);
    }

    if (cursor >= 0) {
        AnnounceCurrentOption(cursor);
        m_lastCursor = cursor;
    }
}

void YesNoHandler::AnnounceCurrentOption(int32_t cursor)
{
    void* thisPtr = LoadThisPtr();
    if (!thisPtr) return;

    auto* ptr = reinterpret_cast<uint8_t*>(thisPtr);

    // Cursor 0 = bottom option (No), cursor 1 = top option (Yes)
    // Confirmed via gameplay: selecting cursor 0 on "Quit?" does NOT quit.
    std::string label;

    switch (cursor) {
        case 0: {
            int32_t noId = *reinterpret_cast<int32_t*>(ptr + Offsets::YesNoWindow::NO_TEXT_ID);
            label = LookupButtonLabel(noId);
            if (label.empty()) label = "No";
            break;
        }
        case 1: {
            int32_t yesId = *reinterpret_cast<int32_t*>(ptr + Offsets::YesNoWindow::YES_TEXT_ID);
            label = LookupButtonLabel(yesId);
            if (label.empty()) label = "Yes";
            break;
        }
        default:
            Logger_Log("YesNo", "Unexpected cursor value: %d", cursor);
            return;
    }

    int totalOptions = 2;

    std::string announcement = label + ", " +
        std::to_string(cursor + 1) + " of " + std::to_string(totalOptions);

    Logger_Log("YesNo", "Cursor: %d (label='%s'), announcing: %s",
               cursor, label.c_str(), announcement.c_str());

    // If this is the first announcement (dialog just opened), queue after message
    if (m_lastCursor < 0) {
        SpeechManager::Get()->Speak(announcement, false);
    } else {
        SpeechManager::Get()->Speak(announcement, true);
    }
}
