// CUiScenarioSelect accessibility handler — campaign selection screen.
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
#include "game_text.h"
#include "offsets.h"
#include "logger.h"

// ============================================================
// Singleton
// ============================================================

ScenarioSelectHandler* ScenarioSelectHandler::Get()
{
    static ScenarioSelectHandler instance;
    return &instance;
}

uintptr_t ScenarioSelectHandler::GetTickRVA() const
{
    return Offsets::FUNC_CUiScenarioSelect_Tick;
}

void ScenarioSelectHandler::OnScreenClosed()
{
    m_interactive = false;
    m_lastCursor = -1;
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
// OnFrame — custom override for pre-tick event consumption
// ============================================================

void ScenarioSelectHandler::OnFrame()
{
    if (!m_installed) return;

    // Always consume events to avoid stale buildup
    auto events = TextCapture::Get()->ConsumeScenarioSelectEvents();

    bool tickFired = ExchangeTickFired();

    if (!tickFired) {
        if (m_screenActive) {
            Logger_Log("ScenarioSelect", "Screen closed (tick stopped firing)");
            OnScreenClosed();
            m_screenActive = false;
        }
        return;
    }

    void* thisPtr = LoadThisPtr();
    if (!thisPtr) return;

    // Track tick activity (but don't announce yet — wait for interactive signal)
    if (!m_screenActive) {
        m_screenActive = true;
        Logger_Log("ScenarioSelect", "Tick started (cutscene/menu active)");
    }

    MemoryInspector::Get()->SetActivePointer("ScenarioSelect", thisPtr);

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
                CallWithSEH(thisPtr);
                return;
            }
        }
        // Not interactive yet — don't do anything
        return;
    }

    // Interactive phase — track cursor from memory
    CallWithSEH(thisPtr);
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
    if (rowId <= 0) return "";
    return GameText_Lookup("scenario_select", rowId);
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
