// File: src/text_capture.cpp
// Purpose: Hooks FUN_1401b9260 (LookupText, RVA 0x1b9260) via MinHook to
//          intercept every text string the game fetches from MBE tables.
//          Per-frame diffing logs NEW/GONE entries. Side-channel caches
//          serve specialised data to YesNoHandler, ScenarioSelectHandler,
//          and SubtitleHandler.
//
// Hook target signature:
//   const char* LookupText(void* manager, const char* tableName,
//                           int rowId, unsigned int language)
//
// All text the game displays from MBE tables (menu items, dialog, UI labels,
// subtitles, yes/no prompts) flows through this single function.

// Singleton accessor.
TextCapture* TextCapture::Get()  (line 24)

// Creates and enables a MinHook on LookupText (module_base + Offsets::Text::FUNC_LookupText).
void TextCapture::Install()  (line 34)

// Disables and removes the hook, clears all capture buffers.
void TextCapture::Uninstall()  (line 65)

// MinHook detour for LookupText. Calls original first, then records the
// {tableName, rowId, text} triple. Additionally populates three side-channel caches:
//   - yes_no_message entries  -> m_latestYesNo  (for YesNoHandler)
//   - scenario_select entries -> m_scenarioEvents (for ScenarioSelectHandler)
//   - subtitle_text entries   -> m_subtitleTexts  (for SubtitleHandler, dedup'd by rowId)
// All cache writes are mutex-protected. The main capture list goes to s_currentFrameCaptures.
static const char* __fastcall TextCapture::HookedLookupText(
    void* manager, const char* tableName, int rowId, unsigned int language)  (line 91)

// Returns a snapshot of all subtitle_text entries captured so far (mutex-protected copy).
std::vector<TextCapture::SubtitleTextEntry> TextCapture::GetSubtitleTexts() const  (line 146)

// Clears the subtitle_text cache (call before a new cutscene to avoid stale entries).
void TextCapture::ClearSubtitleTexts()  (line 152)

// Returns and clears the latest yes_no_message event (consume-once semantics).
TextCapture::YesNoEvent TextCapture::ConsumeYesNoMessage()  (line 158)

// Returns and clears all pending scenario_select events (consume-once semantics).
std::vector<TextCapture::ScenarioSelectEvent> TextCapture::ConsumeScenarioSelectEvents()  (line 166)

// IFrameHandler::OnFrame implementation. Swaps out s_currentFrameCaptures,
// builds a set for the current frame, diffs against m_previousFrame, and
// logs NEW/GONE entries. On the very first frame with data, logs everything
// as a baseline without diffing.
void TextCapture::OnFrame()  (line 178)
