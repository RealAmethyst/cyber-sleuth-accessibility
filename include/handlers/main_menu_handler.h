#pragma once

#include "handlers/tick_handler.h"
#include <cstdint>
#include <string>

// Handles accessibility for the in-game main menu (CUiMainMenu).
//
// Text lookup: LookupText(textManager, "main_menu", cursor+1, language)
// Returns the localized menu item name (Organize, Items, Status, etc.)
//
// Speech rules:
//   Menu opened (state -> 3): Speak("Main Menu", true) then Speak(item, false)
//   Cursor moved in state 3:  Speak(item, true)
class MainMenuHandler : public TickHandler<MainMenuHandler>
{
public:
    static MainMenuHandler* Get();

    // TickHandler interface
    const char* GetHandlerName() const { return "MainMenu"; }
    uintptr_t GetTickRVA() const;
    void OnFrameInner(void* thisPtr);
    void OnScreenClosed();

private:
    MainMenuHandler() = default;
    friend class TickHandler<MainMenuHandler>;

    int32_t m_lastCursorIndex = -1;
    int16_t m_lastState = -1;
    bool m_menuActive = false;

    // Read game state
    int32_t ReadCursor(void* thisPtr);
    int16_t ReadState(void* thisPtr);
    int32_t ReadItemCount(void* thisPtr);

    std::string LookupMenuItemText(int cursorIndex);
    void AnnounceItem(void* thisPtr, bool interrupt);
};
