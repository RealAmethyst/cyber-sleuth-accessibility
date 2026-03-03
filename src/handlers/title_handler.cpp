// CUiTitle accessibility handler — title screen menu.
//
// Title menu items are pre-baked textures (same in all languages):
//   0: "New Game", 1: "Continue", 2: "New Game +", 3: "Exit Game"
//
// Speech rules:
//   Menu opened (state -> 12): Speak("Title Menu", true) then Speak(item, false)
//   Cursor moved in state 12:  Speak(item, true)

#include "handlers/title_handler.h"
#include "handlers/handler_utils.h"
#include "speech_manager.h"
#include "game_text.h"
#include "offsets.h"
#include "logger.h"

// State 7 = "Press Any Button" idle screen (title logo visible, waiting for input)
// State 12 = interactive menu (from Ghidra state machine analysis)
static constexpr uint32_t STATE_PRESS_ANY_BUTTON = 7;
static constexpr uint32_t STATE_INTERACTIVE = 12;

// ============================================================
// "Press Any Button" text per language (from ui_title_logo_bg.img)
// ============================================================

static const char* s_pressAnyButtonText[] = {
    "Press Any button",                              // 0: JP
    "Press Any button",                              // 1: EN
    "Press Any button",                              // 2: CN
    "Press Any button",                              // 3: EN_Censor
    "Press Any button",                              // 4: KR
    "Beliebige Taste dr\u00fccken",                  // 5: DE
};
static constexpr int LANGUAGE_COUNT = 6;

// ============================================================
// Title menu item labels (pre-baked textures, same in all languages)
// ============================================================

static const char* s_titleMenuItems[] = {
    "New Game",
    "Continue",
    "New Game +",
    "Exit Game",
};
static constexpr int TITLE_MENU_ITEM_COUNT = 4;

const char* TitleHandler::GetTitleMenuItem(int cursorIndex)
{
    if (cursorIndex >= 0 && cursorIndex < TITLE_MENU_ITEM_COUNT) {
        return s_titleMenuItems[cursorIndex];
    }
    return "Unknown";
}

// ============================================================
// Singleton
// ============================================================

TitleHandler* TitleHandler::Get()
{
    static TitleHandler instance;
    return &instance;
}

uintptr_t TitleHandler::GetTickRVA() const
{
    return Offsets::FUNC_CUiTitle_Tick;
}

void TitleHandler::OnScreenClosed()
{
    m_menuActive = false;
    m_lastState = 0xFFFFFFFF;
    m_lastCursorIndex = -1;
}

// ============================================================
// OnFrameInner — handler-specific per-frame logic
// ============================================================

void TitleHandler::OnFrameInner(void* thisPtr)
{
    uint32_t state = ReadState(thisPtr);
    int32_t cursor = ReadCursor(thisPtr);
    int32_t itemCount = ReadItemCount(thisPtr);

    // Clamp cursor
    if (itemCount > 0 && itemCount <= TITLE_MENU_ITEM_COUNT) {
        cursor = cursor % itemCount;
    }

    // Detect "Press Any Button" screen (state transitions to 7)
    if (state == STATE_PRESS_ANY_BUTTON && m_lastState != STATE_PRESS_ANY_BUTTON) {
        int lang = GameText_GetLanguage();
        if (lang < 0 || lang >= LANGUAGE_COUNT) lang = 1;

        Logger_Log("Title", "Press Any Button screen (state %u)", state);
        SpeechManager::Get()->Speak("Digimon Story Cyber Sleuth Complete Edition", true);
        SpeechManager::Get()->Speak(s_pressAnyButtonText[lang], false);
    }

    // Detect menu becoming interactive (state transitions to 12)
    if (state == STATE_INTERACTIVE && m_lastState != STATE_INTERACTIVE) {
        m_menuActive = true;
        AnnounceMenuOpened(thisPtr);
        m_lastCursorIndex = cursor;
    }

    // Detect menu leaving interactive state
    if (state != STATE_INTERACTIVE && m_lastState == STATE_INTERACTIVE) {
        m_menuActive = false;
    }

    // Detect cursor changes while menu is active
    if (m_menuActive && cursor != m_lastCursorIndex && m_lastCursorIndex >= 0) {
        AnnounceCurrentItem(thisPtr);
    }

    m_lastState = state;
    m_lastCursorIndex = cursor;
}

// ============================================================
// State reading helpers
// ============================================================

uint32_t TitleHandler::ReadState(void* thisPtr)
{
    return HandlerUtils::ReadMemory<uint32_t>(thisPtr, Offsets::Title::STATE);
}

int32_t TitleHandler::ReadCursor(void* thisPtr)
{
    return HandlerUtils::ReadMemory<int32_t>(thisPtr, Offsets::Title::CURSOR_INDEX);
}

int32_t TitleHandler::ReadItemCount(void* thisPtr)
{
    return HandlerUtils::ReadMemory<int32_t>(thisPtr, Offsets::Title::ITEM_COUNT);
}

// ============================================================
// Announcements
// ============================================================

void TitleHandler::AnnounceMenuOpened(void* thisPtr)
{
    int32_t cursor = ReadCursor(thisPtr);
    int32_t itemCount = ReadItemCount(thisPtr);

    const char* itemText = GetTitleMenuItem(cursor);

    Logger_Log("Title", "Menu opened at cursor %d, itemCount %d", cursor, itemCount);

    // 1. Menu name (interrupt)
    SpeechManager::Get()->Speak("Title Menu", true);

    // 2. First item (queue): "name, N of M"
    auto announcement = HandlerUtils::FormatAnnouncement(itemText, cursor, itemCount);
    SpeechManager::Get()->Speak(announcement, false);
}

void TitleHandler::AnnounceCurrentItem(void* thisPtr)
{
    int32_t cursor = ReadCursor(thisPtr);
    int32_t itemCount = ReadItemCount(thisPtr);

    const char* itemText = GetTitleMenuItem(cursor);

    auto announcement = HandlerUtils::FormatAnnouncement(itemText, cursor, itemCount);

    Logger_Log("Title", "Cursor moved, announcing: %s", announcement.c_str());
    SpeechManager::Get()->Speak(announcement, true);
}
