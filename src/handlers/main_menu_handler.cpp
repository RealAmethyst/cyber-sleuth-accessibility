// CUiMainMenu accessibility handler.
//
// Hooks vtable[3] (tick/update) via MinHook to get per-frame callbacks.
// Reads cursor position at this+0x27D8 and uses the game's text lookup
// API to get localized menu item names dynamically.
//
// Text lookup: FUN_1401b9260(textManager, "main_menu", id, language)
// main_menu.mbe IDs: 1=Organize, 2=Items, 3=Status, 4=Options,
//   5=Save/Load, 6=Sort Digimon, 7=Farm, 8=Exit

#include "handlers/main_menu_handler.h"
#include "memory_inspector.h"
#include "speech_manager.h"
#include "offsets.h"
#include "logger.h"

#include <modloader/utils.h>
#include <MinHook.h>
#include <windows.h>

MainMenuHandler* MainMenuHandler::Get()
{
    static MainMenuHandler instance;
    return &instance;
}

void MainMenuHandler::Install()
{
    if (m_installed) return;

    uintptr_t base = reinterpret_cast<uintptr_t>(getBaseOffset());

    // Resolve text API function pointers
    s_getTextTableManager = reinterpret_cast<GetTextTableManagerFunc>(
        base + Offsets::Text::FUNC_GetTextTableManager);
    s_lookupText = reinterpret_cast<LookupTextFunc>(
        base + Offsets::Text::FUNC_LookupText);

    Logger_Log("MainMenu", "Text API resolved: GetTextTableManager=%p, LookupText=%p",
               (void*)s_getTextTableManager, (void*)s_lookupText);

    // Hook CUiMainMenu's tick function (vtable[3]) via MinHook
    s_hookTarget = reinterpret_cast<void*>(base + Offsets::FUNC_CUiMainMenu_Tick);

    MH_STATUS status = MH_CreateHook(s_hookTarget, (void*)&HookedTick,
                                      reinterpret_cast<void**>(&s_originalTick));
    if (status != MH_OK) {
        Logger_Log("MainMenu", "MH_CreateHook failed for tick @ 0x%llx: %d",
                   (unsigned long long)(base + Offsets::FUNC_CUiMainMenu_Tick), status);
        return;
    }

    status = MH_EnableHook(s_hookTarget);
    if (status != MH_OK) {
        Logger_Log("MainMenu", "MH_EnableHook failed: %d", status);
        MH_RemoveHook(s_hookTarget);
        return;
    }

    m_installed = true;
    Logger_Log("MainMenu", "MinHook installed on CUiMainMenu tick (RVA 0x%llx)",
               (unsigned long long)Offsets::FUNC_CUiMainMenu_Tick);
}

void MainMenuHandler::Uninstall()
{
    if (!m_installed) return;

    if (s_hookTarget) {
        MH_DisableHook(s_hookTarget);
        MH_RemoveHook(s_hookTarget);
    }

    s_originalTick = nullptr;
    s_hookTarget = nullptr;
    m_lastThisPtr = nullptr;
    m_lastCursorIndex = -1;
    m_lastState = -1;
    m_menuActive = false;
    m_installed = false;

    MemoryInspector::Get()->ClearPointer("CUiMainMenu");
    Logger_Log("MainMenu", "MinHook uninstalled");
}

void __fastcall MainMenuHandler::HookedTick(void* thisPtr, void* param2)
{
    // Call original tick first
    if (s_originalTick) {
        s_originalTick(thisPtr, param2);
    }

    if (!thisPtr) return;

    auto* handler = Get();

    // Track instance
    if (handler->m_lastThisPtr != thisPtr) {
        handler->m_lastThisPtr = thisPtr;
        Logger_Log("MainMenu", "CUiMainMenu tick firing (this=%p)", thisPtr);
    }

    MemoryInspector::Get()->SetActivePointer("CUiMainMenu", thisPtr);

    // Check for state changes and announce
    handler->CheckStateChanges(thisPtr);
}

void MainMenuHandler::OnFrame()
{
    // All work happens in HookedTick via MinHook.
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
    if (!s_getTextTableManager || !s_lookupText) {
        return "item " + std::to_string(cursorIndex + 1);
    }

    uintptr_t base = reinterpret_cast<uintptr_t>(getBaseOffset());

    // Get language index from the language settings singleton
    unsigned int language = 1; // Default to English
    uintptr_t langSettings = *reinterpret_cast<uintptr_t*>(
        base + Offsets::Text::DAT_LanguageSettings);
    if (langSettings != 0) {
        language = *reinterpret_cast<unsigned int*>(
            langSettings + Offsets::Text::LANGUAGE_INDEX_OFFSET);
    }

    // Get text table manager
    void* manager = s_getTextTableManager();
    if (!manager) {
        Logger_Log("MainMenu", "GetTextTableManager returned null");
        return "item " + std::to_string(cursorIndex + 1);
    }

    // Lookup: main_menu.mbe, ID = cursor + 1 (IDs are 1-based)
    const char* text = s_lookupText(manager, "main_menu", cursorIndex + 1, language);
    if (text && text[0] != '\0') {
        return std::string(text);
    }

    Logger_Log("MainMenu", "LookupText returned null/empty for main_menu ID %d", cursorIndex + 1);
    return "item " + std::to_string(cursorIndex + 1);
}

// === State change detection and announcements ===

void MainMenuHandler::CheckStateChanges(void* thisPtr)
{
    int16_t state = ReadState(thisPtr);
    int32_t cursor = ReadCursor(thisPtr);
    int32_t itemCount = ReadItemCount(thisPtr);

    // Clamp cursor to valid range
    if (itemCount > 0 && itemCount <= 8) {
        cursor = cursor % itemCount;
    }

    // Detect menu becoming active (state transitions to 3 = interactive)
    // States: 0=closed?, 1=opening?, 2=animating?, 3=interactive, 4=?, 5=closing?
    if (state == 3 && m_lastState != 3) {
        m_menuActive = true;
        AnnounceMenuOpened(thisPtr);
        m_lastCursorIndex = cursor;
    }

    // Detect menu closing
    if (state != 3 && m_lastState == 3) {
        m_menuActive = false;
    }

    // Detect cursor changes while menu is active
    if (m_menuActive && cursor != m_lastCursorIndex && m_lastCursorIndex >= 0) {
        AnnounceCurrentItem(thisPtr);
    }

    m_lastState = state;
    m_lastCursorIndex = cursor;
}

void MainMenuHandler::AnnounceMenuOpened(void* thisPtr)
{
    int32_t cursor = ReadCursor(thisPtr);
    int32_t itemCount = ReadItemCount(thisPtr);

    // Look up the current item text
    std::string itemText = LookupMenuItemText(cursor);

    // Format: "itemName, N of M"
    std::string announcement = itemText + ", " +
        std::to_string(cursor + 1) + " of " + std::to_string(itemCount);

    Logger_Log("MainMenu", "Menu opened, announcing: %s", announcement.c_str());
    SpeechManager::Get()->Speak(announcement, true);
}

void MainMenuHandler::AnnounceCurrentItem(void* thisPtr)
{
    int32_t cursor = ReadCursor(thisPtr);
    int32_t itemCount = ReadItemCount(thisPtr);

    // Look up the current item text
    std::string itemText = LookupMenuItemText(cursor);

    // Format: "itemName, N of M"
    std::string announcement = itemText + ", " +
        std::to_string(cursor + 1) + " of " + std::to_string(itemCount);

    Logger_Log("MainMenu", "Cursor moved, announcing: %s", announcement.c_str());
    SpeechManager::Get()->Speak(announcement, true);
}
