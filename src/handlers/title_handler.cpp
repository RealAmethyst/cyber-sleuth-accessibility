// CUiTitle accessibility handler — title screen menu.
//
// Hooks vtable[3] (tick/update) via MinHook ONLY to capture the this pointer.
// All state reading and speech happens in OnFrame() (SwapBuffers context).
//
// Title menu items are pre-baked textures (same in all languages):
//   0: "New Game", 1: "Continue", 2: "New Game +", 3: "Exit Game"
//
// Speech rules:
//   Menu opened (state -> 12): Speak("Title Menu", true) then Speak(item, false)
//   Cursor moved in state 12:  Speak(item, true)

#include "handlers/title_handler.h"
#include "memory_inspector.h"
#include "speech_manager.h"
#include "offsets.h"
#include "logger.h"

#include <modloader/utils.h>
#include <MinHook.h>
#include <windows.h>

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

// ============================================================
// Hook installation
// ============================================================

void TitleHandler::Install()
{
    if (m_installed) return;

    uintptr_t base = reinterpret_cast<uintptr_t>(getBaseOffset());

    s_hookTarget = reinterpret_cast<void*>(base + Offsets::FUNC_CUiTitle_Tick);

    MH_STATUS status = MH_CreateHook(s_hookTarget, (void*)&HookedTick,
                                      reinterpret_cast<void**>(&s_originalTick));
    if (status != MH_OK) {
        Logger_Log("Title", "MH_CreateHook failed for tick @ RVA 0x%llx: %d",
                   (unsigned long long)Offsets::FUNC_CUiTitle_Tick, status);
        return;
    }

    status = MH_EnableHook(s_hookTarget);
    if (status != MH_OK) {
        Logger_Log("Title", "MH_EnableHook failed: %d", status);
        MH_RemoveHook(s_hookTarget);
        return;
    }

    m_installed = true;
    Logger_Log("Title", "MinHook installed on CUiTitle tick (RVA 0x%llx)",
               (unsigned long long)Offsets::FUNC_CUiTitle_Tick);
}

void TitleHandler::Uninstall()
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
    m_lastState = 0xFFFFFFFF;
    m_menuActive = false;
    m_installed = false;

    MemoryInspector::Get()->ClearPointer("CUiTitle");
    Logger_Log("Title", "MinHook uninstalled");
}

// ============================================================
// Tick detour — MINIMAL. Only captures this pointer.
// ============================================================

void __fastcall TitleHandler::HookedTick(void* thisPtr, void* param2)
{
    // Call original tick first (always)
    if (s_originalTick) {
        s_originalTick(thisPtr, param2);
    }

    // Store this pointer for OnFrame to use. That's ALL we do here.
    if (thisPtr) {
        s_thisPtr.store(thisPtr, std::memory_order_relaxed);
        s_tickFired.store(true, std::memory_order_relaxed);
    }
}

// ============================================================
// OnFrame — all state reading and speech happens here
// (called from SwapBuffers hook, safe context)
// ============================================================

// SEH wrapper — must be in a separate function with no C++ objects
static void OnFrameSEH(TitleHandler* handler, void* thisPtr)
{
    __try {
        handler->OnFrameInner(thisPtr);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        Logger_Log("Title", "EXCEPTION in OnFrame (this=%p, code=0x%08lx)",
                   thisPtr, GetExceptionCode());
    }
}

void TitleHandler::OnFrame()
{
    bool tickFired = s_tickFired.exchange(false, std::memory_order_relaxed);

    if (!tickFired) {
        if (m_menuActive) {
            Logger_Log("Title", "Screen closed (tick stopped firing)");
            m_menuActive = false;
            m_lastState = 0xFFFFFFFF;
            m_lastCursorIndex = -1;
        }
        return;
    }

    void* thisPtr = s_thisPtr.load(std::memory_order_relaxed);
    if (!thisPtr) return;

    MemoryInspector::Get()->SetActivePointer("CUiTitle", thisPtr);
    OnFrameSEH(this, thisPtr);
}

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

    // Speech rule: cursor moved — speak new item (interrupt): "name, N of M"
    std::string announcement = std::string(itemText) + ", " +
        std::to_string(cursor + 1) + " of " + std::to_string(itemCount);

    Logger_Log("Title", "Cursor moved, announcing: %s", announcement.c_str());
    SpeechManager::Get()->Speak(announcement, true);
}
