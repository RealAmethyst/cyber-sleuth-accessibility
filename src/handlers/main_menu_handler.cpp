// CUiMainMenu accessibility handler.
//
// Hooks vtable[3] (tick/update) via MinHook ONLY to capture the this pointer.
// All state reading and speech happens in OnFrame() (SwapBuffers context).
//
// Text lookup: FUN_1401b9260(textManager, "main_menu", id, language)
// main_menu.mbe IDs: 1=Organize, 2=Items, 3=Status, 4=Options,
//   5=Save/Load, 6=Sort Digimon, 7=Farm, 8=Exit
//
// Speech rules:
//   Menu opened (state -> 3): Speak("Main Menu", true) then Speak(item, false)
//   Cursor moved in state 3:  Speak(item, true)

#include "handlers/main_menu_handler.h"
#include "memory_inspector.h"
#include "speech_manager.h"
#include "offsets.h"
#include "logger.h"

#include <modloader/utils.h>
#include <MinHook.h>
#include <windows.h>

// State 3 = interactive menu
static constexpr int16_t STATE_INTERACTIVE = 3;

// SEH wrapper — must be a free function (no C++ objects with destructors)
static void OnFrameSEH(MainMenuHandler* handler, void* thisPtr)
{
    __try {
        handler->OnFrameInner(thisPtr);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        Logger_Log("MainMenu", "EXCEPTION in OnFrame (this=%p, code=0x%08lx)",
                   thisPtr, GetExceptionCode());
    }
}

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

    // Hook CUiMainMenu's tick function via MinHook
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
    s_thisPtr.store(nullptr, std::memory_order_relaxed);
    s_tickFired.store(false, std::memory_order_relaxed);
    m_lastCursorIndex = -1;
    m_lastState = -1;
    m_menuActive = false;
    m_installed = false;

    MemoryInspector::Get()->ClearPointer("CUiMainMenu");
    Logger_Log("MainMenu", "MinHook uninstalled");
}

// === Tick detour — MINIMAL, only captures this pointer ===

void __fastcall MainMenuHandler::HookedTick(void* thisPtr, void* param2)
{
    if (s_originalTick) {
        s_originalTick(thisPtr, param2);
    }

    if (thisPtr) {
        s_thisPtr.store(thisPtr, std::memory_order_relaxed);
        s_tickFired.store(true, std::memory_order_relaxed);
    }
}

// === OnFrame — all work happens here (SwapBuffers context) ===

void MainMenuHandler::OnFrame()
{
    if (!m_installed) return;

    bool fired = s_tickFired.exchange(false, std::memory_order_relaxed);

    if (!fired) {
        // Tick stopped firing — menu was closed
        if (m_menuActive) {
            Logger_Log("MainMenu", "Tick stopped — menu closed");
            m_menuActive = false;
            m_lastState = -1;
            m_lastCursorIndex = -1;
        }
        return;
    }

    void* thisPtr = s_thisPtr.load(std::memory_order_relaxed);
    if (!thisPtr) return;

    MemoryInspector::Get()->SetActivePointer("CUiMainMenu", thisPtr);
    OnFrameSEH(this, thisPtr);
}

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

        // Speech rule: menu name first (interrupt), then current item (queued)
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

    void* manager = s_getTextTableManager();
    if (!manager) {
        return "item " + std::to_string(cursorIndex + 1);
    }

    // main_menu.mbe IDs are 1-based (cursor + 1)
    const char* text = s_lookupText(manager, "main_menu", cursorIndex + 1, language);
    if (text && text[0] != '\0') {
        return std::string(text);
    }

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
