// CUiYesNoWindow accessibility handler — Yes/No/Cancel dialog prompts.
//
// Hooks vtable[3] (tick/update) via MinHook ONLY to capture the this pointer.
// All state reading and speech happens in OnFrame() (SwapBuffers context).
//
// Message text is captured from TextCapture (yes_no_message table lookups).
// The game looks up the message text once when the dialog opens, so we cache
// it from TextCapture and announce when state transitions to interactive.
//
// Cursor tracking: byte at this+0x81 (0=Yes, 1=No), confirmed via memory dump.
// Button labels read dynamically from common_message table using IDs stored
// at this+0x78 (Yes label) and this+0x7c (No label).

#include "handlers/yesno_handler.h"
#include "text_capture.h"
#include "memory_inspector.h"
#include "speech_manager.h"
#include "offsets.h"
#include "logger.h"

#include <modloader/utils.h>
#include <MinHook.h>
#include <windows.h>

// State constants (from Ghidra decompilation of tick function)
static constexpr int32_t STATE_IDLE        = 0;
static constexpr int32_t STATE_READY       = 1;
static constexpr int32_t STATE_OPENING     = 2;
static constexpr int32_t STATE_ANIM        = 3;
static constexpr int32_t STATE_INTERACTIVE = 4;
static constexpr int32_t STATE_CLOSING     = 5;
static constexpr int32_t STATE_DONE        = 6;

// ============================================================
// Singleton
// ============================================================

YesNoHandler* YesNoHandler::Get()
{
    static YesNoHandler instance;
    return &instance;
}

// ============================================================
// Hook installation
// ============================================================

void YesNoHandler::Install()
{
    if (m_installed) return;

    uintptr_t base = reinterpret_cast<uintptr_t>(getBaseOffset());

    // Resolve text API function pointers
    s_getTextTableManager = reinterpret_cast<GetTextTableManagerFunc>(
        base + Offsets::Text::FUNC_GetTextTableManager);
    s_lookupText = reinterpret_cast<LookupTextFunc>(
        base + Offsets::Text::FUNC_LookupText);

    Logger_Log("YesNo", "Text API resolved: GetTextTableManager=%p, LookupText=%p",
               (void*)s_getTextTableManager, (void*)s_lookupText);

    s_hookTarget = reinterpret_cast<void*>(base + Offsets::FUNC_CUiYesNoWindow_Tick);

    MH_STATUS status = MH_CreateHook(s_hookTarget, (void*)&HookedTick,
                                      reinterpret_cast<void**>(&s_originalTick));
    if (status != MH_OK) {
        Logger_Log("YesNo", "MH_CreateHook failed for tick @ RVA 0x%llx: %d",
                   (unsigned long long)Offsets::FUNC_CUiYesNoWindow_Tick, status);
        return;
    }

    status = MH_EnableHook(s_hookTarget);
    if (status != MH_OK) {
        Logger_Log("YesNo", "MH_EnableHook failed: %d", status);
        MH_RemoveHook(s_hookTarget);
        return;
    }

    m_installed = true;
    Logger_Log("YesNo", "MinHook installed on CUiYesNoWindow tick (RVA 0x%llx)",
               (unsigned long long)Offsets::FUNC_CUiYesNoWindow_Tick);
}

void YesNoHandler::Uninstall()
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
    m_lastState = -1;
    m_lastCursor = -1;
    m_dialogActive = false;
    m_currentMessage.clear();
    m_installed = false;

    MemoryInspector::Get()->ClearPointer("CUiYesNoWindow");
    Logger_Log("YesNo", "MinHook uninstalled");
}

// ============================================================
// Tick detour — MINIMAL. Only captures this pointer.
// ============================================================

void __fastcall YesNoHandler::HookedTick(void* thisPtr, void* param2)
{
    if (s_originalTick) {
        s_originalTick(thisPtr, param2);
    }

    if (thisPtr) {
        s_thisPtr.store(thisPtr, std::memory_order_relaxed);
        s_tickFired.store(true, std::memory_order_relaxed);
    }
}

// ============================================================
// OnFrame — all state reading and speech happens here
// ============================================================

static void OnFrameSEH(YesNoHandler* handler, void* thisPtr)
{
    __try {
        handler->OnFrameInner(thisPtr);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        Logger_Log("YesNo", "EXCEPTION in OnFrame (this=%p, code=0x%08lx)",
                   thisPtr, GetExceptionCode());
    }
}

void YesNoHandler::OnFrame()
{
    // Check if tick fired this frame (dialog is active)
    bool tickFired = s_tickFired.exchange(false, std::memory_order_relaxed);

    if (!tickFired) {
        // Tick didn't fire — dialog may have closed
        if (m_dialogActive) {
            Logger_Log("YesNo", "Dialog closed (tick stopped firing)");
            m_dialogActive = false;
            m_lastState = -1;
            m_lastCursor = -1;
            m_currentMessage.clear();
        }
        return;
    }

    void* thisPtr = s_thisPtr.load(std::memory_order_relaxed);
    if (!thisPtr) return;

    MemoryInspector::Get()->SetActivePointer("CUiYesNoWindow", thisPtr);
    OnFrameSEH(this, thisPtr);
}

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

bool YesNoHandler::ReadCancelEnabled(void* thisPtr)
{
    auto* ptr = reinterpret_cast<uint8_t*>(thisPtr);
    return *reinterpret_cast<uint8_t*>(ptr + Offsets::YesNoWindow::CANCEL_ENABLED) != 0;
}

int32_t YesNoHandler::ReadCursor(void* thisPtr)
{
    auto* ptr = reinterpret_cast<uint8_t*>(thisPtr);
    uint8_t cursor = *(ptr + Offsets::YesNoWindow::CURSOR_INDEX);

    // Sanity check — cursor should be 0 (Yes) or 1 (No)
    if (cursor > 2) return -1;

    return static_cast<int32_t>(cursor);
}

// ============================================================
// Text lookup
// ============================================================

std::string YesNoHandler::LookupButtonLabel(int32_t commonMessageId)
{
    if (!s_getTextTableManager || !s_lookupText || commonMessageId <= 0) {
        return "";
    }

    uintptr_t base = reinterpret_cast<uintptr_t>(getBaseOffset());

    // Get language index
    unsigned int language = 1; // Default English
    uintptr_t langSettings = *reinterpret_cast<uintptr_t*>(
        base + Offsets::Text::DAT_LanguageSettings);
    if (langSettings != 0) {
        language = *reinterpret_cast<unsigned int*>(
            langSettings + Offsets::Text::LANGUAGE_INDEX_OFFSET);
    }

    void* manager = s_getTextTableManager();
    if (!manager) return "";

    const char* text = s_lookupText(manager, "common_message", commonMessageId, language);
    if (text && text[0] != '\0') {
        return std::string(text);
    }
    return "";
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
    int32_t cursor = ReadCursor(
        s_thisPtr.load(std::memory_order_relaxed));

    if (cursor >= 0) {
        AnnounceCurrentOption(cursor);
        m_lastCursor = cursor;
    }
}

void YesNoHandler::AnnounceCurrentOption(int32_t cursor)
{
    void* thisPtr = s_thisPtr.load(std::memory_order_relaxed);
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
