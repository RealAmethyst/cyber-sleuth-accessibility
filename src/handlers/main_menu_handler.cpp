// CUiMainMenu accessibility handler.
//
// Text lookup: FUN_1401b9260(textManager, "main_menu", id, language)
// main_menu.mbe IDs: 1=Organize, 2=Items, 3=Status, 4=Options,
//   5=Save/Load, 6=Sort Digimon, 7=Farm, 8=Exit
//
// Speech rules:
//   Menu opened (state -> 3): Speak("Main Menu", true) then Speak(item, false)
//   Cursor moved in state 3:  Speak(item, true)

#include "handlers/main_menu_handler.h"
#include "speech_manager.h"
#include "game_text.h"
#include "offsets.h"
#include "logger.h"

// State 3 = interactive menu
static constexpr int16_t STATE_INTERACTIVE = 3;

MainMenuHandler* MainMenuHandler::Get()
{
    static MainMenuHandler instance;
    return &instance;
}

uintptr_t MainMenuHandler::GetTickRVA() const
{
    return Offsets::FUNC_CUiMainMenu_Tick;
}

void MainMenuHandler::OnScreenClosed()
{
    m_menuActive = false;
    m_lastState = -1;
    m_lastCursorIndex = -1;
}

// === OnFrameInner — handler-specific per-frame logic ===

void MainMenuHandler::OnFrameInner(void* thisPtr)
{
    int16_t state = ReadState(thisPtr);
    int32_t cursor = ReadCursor(thisPtr);
    int32_t itemCount = ReadItemCount(thisPtr);

    // Clamp cursor to valid range
    if (itemCount > 0 && itemCount <= 8) {
        cursor = cursor % itemCount;
    }

    // Detect menu becoming active (state transitions to 3 = interactive)
    if (state == STATE_INTERACTIVE && m_lastState != STATE_INTERACTIVE) {
        m_menuActive = true;
        m_lastCursorIndex = cursor;

        SpeechManager::Get()->Speak("Main Menu", true);
        AnnounceItem(thisPtr, false);

        Logger_Log("MainMenu", "Menu opened, cursor=%d, itemCount=%d", cursor, itemCount);
    }

    // Detect menu closing
    if (state != STATE_INTERACTIVE && m_lastState == STATE_INTERACTIVE) {
        m_menuActive = false;
    }

    // Detect cursor changes while menu is active
    if (m_menuActive && cursor != m_lastCursorIndex && m_lastCursorIndex >= 0) {
        AnnounceItem(thisPtr, true);
    }

    m_lastState = state;
    m_lastCursorIndex = cursor;
}

// === State reading helpers ===

int32_t MainMenuHandler::ReadCursor(void* thisPtr)
{
    auto* ptr = reinterpret_cast<uint8_t*>(thisPtr);
    return *reinterpret_cast<int32_t*>(ptr + Offsets::MainMenu::CURSOR_INDEX);
}

int16_t MainMenuHandler::ReadState(void* thisPtr)
{
    auto* ptr = reinterpret_cast<uint8_t*>(thisPtr);
    return *reinterpret_cast<int16_t*>(ptr + Offsets::MainMenu::STATE);
}

int32_t MainMenuHandler::ReadItemCount(void* thisPtr)
{
    auto* ptr = reinterpret_cast<uint8_t*>(thisPtr);
    return *reinterpret_cast<int32_t*>(ptr + Offsets::MainMenu::ITEM_COUNT);
}

// === Text lookup ===

std::string MainMenuHandler::LookupMenuItemText(int cursorIndex)
{
    std::string text = GameText_Lookup("main_menu", cursorIndex + 1);
    if (!text.empty()) return text;
    return "item " + std::to_string(cursorIndex + 1);
}

// === Announcement ===

void MainMenuHandler::AnnounceItem(void* thisPtr, bool interrupt)
{
    int32_t cursor = ReadCursor(thisPtr);
    int32_t itemCount = ReadItemCount(thisPtr);

    std::string itemText = LookupMenuItemText(cursor);
    std::string announcement = itemText + ", " +
        std::to_string(cursor + 1) + " of " + std::to_string(itemCount);

    Logger_Log("MainMenu", "Announcing: %s (interrupt=%d)", announcement.c_str(), interrupt);
    SpeechManager::Get()->Speak(announcement, interrupt);
}
