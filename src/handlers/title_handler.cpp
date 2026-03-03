// CUiTitle accessibility handler — title screen menu.
//
// Title menu items are pre-baked textures (same in all languages):
//   0: "New Game", 1: "Continue", 2: "New Game +", 3: "Exit Game"
//
// Speech rules:
//   Menu opened (state -> 12): Speak("Title Menu", true) then Speak(item, false)
//   Cursor moved in state 12:  Speak(item, true)

#include "handlers/title_handler.h"
#include "speech_manager.h"
#include "offsets.h"
#include "logger.h"

// State 12 = interactive menu (from Ghidra state machine analysis)
static constexpr uint32_t STATE_INTERACTIVE = 12;

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
    auto* ptr = reinterpret_cast<uint8_t*>(thisPtr);
    return *reinterpret_cast<uint32_t*>(ptr + Offsets::Title::STATE);
}

int32_t TitleHandler::ReadCursor(void* thisPtr)
{
    auto* ptr = reinterpret_cast<uint8_t*>(thisPtr);
    return *reinterpret_cast<int32_t*>(ptr + Offsets::Title::CURSOR_INDEX);
}

int32_t TitleHandler::ReadItemCount(void* thisPtr)
{
    auto* ptr = reinterpret_cast<uint8_t*>(thisPtr);
    return *reinterpret_cast<int32_t*>(ptr + Offsets::Title::ITEM_COUNT);
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
    std::string announcement = std::string(itemText) + ", " +
        std::to_string(cursor + 1) + " of " + std::to_string(itemCount);
    SpeechManager::Get()->Speak(announcement, false);
}

void TitleHandler::AnnounceCurrentItem(void* thisPtr)
{
    int32_t cursor = ReadCursor(thisPtr);
    int32_t itemCount = ReadItemCount(thisPtr);

    const char* itemText = GetTitleMenuItem(cursor);

    std::string announcement = std::string(itemText) + ", " +
        std::to_string(cursor + 1) + " of " + std::to_string(itemCount);

    Logger_Log("Title", "Cursor moved, announcing: %s", announcement.c_str());
    SpeechManager::Get()->Speak(announcement, true);
}
