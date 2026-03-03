// CUiScenarioSelect accessibility handler — campaign selection screen.
//
// Hooks vtable[3] (tick/update) via MinHook ONLY to capture the this pointer.
// All state reading and speech happens in OnFrame() (SwapBuffers context).
//
// The tick fires during the cutscene AND the interactive menu. We wait for
// TextCapture to deliver scenario_select:1 (prompt text) before announcing
// anything — that's the signal the interactive phase has started.
//
// Cursor at +0xE0 (1-based): maps to item IDs at +0xD0 array.
// Campaign names are hardcoded (pre-baked textures, same in all languages).

#include "handlers/scenario_select_handler.h"
#include "text_capture.h"
#include "memory_inspector.h"
#include "speech_manager.h"
#include "offsets.h"
#include "logger.h"

#include <modloader/utils.h>
#include <MinHook.h>
#include <windows.h>

// ============================================================
// Singleton
// ============================================================

ScenarioSelectHandler* ScenarioSelectHandler::Get()
{
    static ScenarioSelectHandler instance;
    return &instance;
}

// ============================================================
// Campaign name mapping (pre-baked textures, same in all languages)
// ============================================================

const char* ScenarioSelectHandler::GetCampaignName(int rowId)
{
    switch (rowId) {
        case 2: return "Hacker's Memory";
        case 3: return "Cyber Sleuth";
        default: return nullptr;
    }
}

// ============================================================
// Hook installation
// ============================================================

void ScenarioSelectHandler::Install()
{
    if (m_installed) return;

    uintptr_t base = reinterpret_cast<uintptr_t>(getBaseOffset());

    // Resolve text API function pointers
    s_getTextTableManager = reinterpret_cast<GetTextTableManagerFunc>(
        base + Offsets::Text::FUNC_GetTextTableManager);
    s_lookupText = reinterpret_cast<LookupTextFunc>(
        base + Offsets::Text::FUNC_LookupText);

    s_hookTarget = reinterpret_cast<void*>(base + Offsets::FUNC_CUiScenarioSelect_Tick);

    MH_STATUS status = MH_CreateHook(s_hookTarget, (void*)&HookedTick,
                                      reinterpret_cast<void**>(&s_originalTick));
    if (status != MH_OK) {
        Logger_Log("ScenarioSelect", "MH_CreateHook failed for tick @ RVA 0x%llx: %d",
                   (unsigned long long)Offsets::FUNC_CUiScenarioSelect_Tick, status);
        return;
    }

    status = MH_EnableHook(s_hookTarget);
    if (status != MH_OK) {
        Logger_Log("ScenarioSelect", "MH_EnableHook failed: %d", status);
        MH_RemoveHook(s_hookTarget);
        return;
    }

    m_installed = true;
    Logger_Log("ScenarioSelect", "MinHook installed on CUiScenarioSelect tick (RVA 0x%llx)",
               (unsigned long long)Offsets::FUNC_CUiScenarioSelect_Tick);
}

void ScenarioSelectHandler::Uninstall()
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
    m_tickActive = false;
    m_interactive = false;
    m_lastCursor = -1;
    m_installed = false;

    MemoryInspector::Get()->ClearPointer("CUiScenarioSelect");
    Logger_Log("ScenarioSelect", "MinHook uninstalled");
}

// ============================================================
// Tick detour — MINIMAL. Only captures this pointer.
// ============================================================

void __fastcall ScenarioSelectHandler::HookedTick(void* thisPtr, void* param2)
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

static void OnFrameSEH(ScenarioSelectHandler* handler, void* thisPtr)
{
    __try {
        handler->OnFrameInner(thisPtr);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        Logger_Log("ScenarioSelect", "EXCEPTION in OnFrame (this=%p, code=0x%08lx)",
                   thisPtr, GetExceptionCode());
    }
}

void ScenarioSelectHandler::OnFrame()
{
    // Always consume events to avoid stale buildup
    auto events = TextCapture::Get()->ConsumeScenarioSelectEvents();

    bool tickFired = s_tickFired.exchange(false, std::memory_order_relaxed);

    if (!tickFired) {
        if (m_tickActive) {
            Logger_Log("ScenarioSelect", "Screen closed (tick stopped firing)");
            m_tickActive = false;
            m_interactive = false;
            m_lastCursor = -1;
        }
        return;
    }

    void* thisPtr = s_thisPtr.load(std::memory_order_relaxed);
    if (!thisPtr) return;

    // Track tick activity (but don't announce yet — wait for interactive signal)
    if (!m_tickActive) {
        m_tickActive = true;
        Logger_Log("ScenarioSelect", "Tick started (cutscene/menu active)");
    }

    MemoryInspector::Get()->SetActivePointer("CUiScenarioSelect", thisPtr);

    // Check TextCapture events for the prompt text (signals interactive phase)
    if (!m_interactive) {
        for (auto& event : events) {
            if (event.rowId == 1 || event.rowId == 101 || event.rowId == 201) {
                // Prompt text appeared — interactive phase started
                m_interactive = true;
                m_lastCursor = -1;

                Logger_Log("ScenarioSelect", "Interactive phase started (prompt ID %d)", event.rowId);
                SpeechManager::Get()->Speak(event.text, true);

                // Read and announce current cursor position
                OnFrameSEH(this, thisPtr);
                return;
            }
        }
        // Not interactive yet — don't do anything
        return;
    }

    // Interactive phase — track cursor from memory
    OnFrameSEH(this, thisPtr);
}

void ScenarioSelectHandler::OnFrameInner(void* thisPtr)
{
    int32_t cursor = ReadCursor(thisPtr);
    if (cursor < 1) return;  // invalid

    if (cursor != m_lastCursor) {
        // First announcement after open: queue after prompt
        bool interrupt = (m_lastCursor >= 0);
        AnnounceItem(cursor, thisPtr, interrupt);
        m_lastCursor = cursor;
    }
}

// ============================================================
// State reading helpers
// ============================================================

int32_t ScenarioSelectHandler::ReadCursor(void* thisPtr)
{
    auto* ptr = reinterpret_cast<uint8_t*>(thisPtr);
    int32_t cursor = *reinterpret_cast<int32_t*>(ptr + Offsets::ScenarioSelect::CURSOR_INDEX);

    // Sanity: cursor should be 1-based, small positive
    if (cursor < 1 || cursor > 10) return -1;
    return cursor;
}

int32_t ScenarioSelectHandler::ReadItemId(void* thisPtr, int32_t cursor)
{
    auto* ptr = reinterpret_cast<uint8_t*>(thisPtr);
    uintptr_t offset = Offsets::ScenarioSelect::ITEM_ID_BASE +
                       static_cast<uintptr_t>(cursor - 1) * Offsets::ScenarioSelect::ITEM_ID_STRIDE;
    return *reinterpret_cast<int32_t*>(ptr + offset);
}

// ============================================================
// Text lookup
// ============================================================

std::string ScenarioSelectHandler::LookupDescription(int rowId)
{
    if (!s_getTextTableManager || !s_lookupText || rowId <= 0) {
        return "";
    }

    uintptr_t base = reinterpret_cast<uintptr_t>(getBaseOffset());

    unsigned int language = 1; // Default English
    uintptr_t langSettings = *reinterpret_cast<uintptr_t*>(
        base + Offsets::Text::DAT_LanguageSettings);
    if (langSettings != 0) {
        language = *reinterpret_cast<unsigned int*>(
            langSettings + Offsets::Text::LANGUAGE_INDEX_OFFSET);
    }

    void* manager = s_getTextTableManager();
    if (!manager) return "";

    const char* text = s_lookupText(manager, "scenario_select", rowId, language);
    if (text && text[0] != '\0') {
        return std::string(text);
    }
    return "";
}

// ============================================================
// Announcements
// ============================================================

void ScenarioSelectHandler::AnnounceItem(int32_t cursor, void* thisPtr, bool interrupt)
{
    int32_t itemId = ReadItemId(thisPtr, cursor);
    const char* name = GetCampaignName(itemId);

    std::string announcement;
    if (name) {
        announcement = std::string(name);
    } else {
        announcement = "Item " + std::to_string(cursor);
    }

    // Look up description text from game's text tables
    std::string desc = LookupDescription(itemId);
    if (!desc.empty()) {
        announcement += ", " + desc;
    }

    // Add position info
    announcement += ", " + std::to_string(cursor) + " of " + std::to_string(m_itemCount);

    Logger_Log("ScenarioSelect", "Cursor %d (itemId=%d, name='%s'), announcing: %s",
               cursor, itemId, name ? name : "?", announcement.c_str());

    SpeechManager::Get()->Speak(announcement, interrupt);
}
