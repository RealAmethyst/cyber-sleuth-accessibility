// File: src/handlers/scenario_select_handler.cpp
// Purpose: Accessibility handler for CUiScenarioSelect (campaign selection screen).
//          Hooks vtable[3] tick via MinHook ONLY to capture the this pointer.
//          All state reading and speech happen in OnFrame() (SwapBuffers context).
//
//          The tick fires during both the opening cutscene and the interactive menu.
//          We wait for TextCapture to deliver scenario_select row ID 1, 101, or 201
//          (the prompt text) before announcing — this signals the interactive phase.
//
//          Campaign names are hardcoded (pre-baked textures, same in all languages):
//            ID 2 = "Hacker's Memory", ID 3 = "Cyber Sleuth"
//          Cursor at this+0xE0 (int32, 1-based).
//          Item ID array at this+0xD0 (int32 array, stride 4).

// Singleton accessor.
ScenarioSelectHandler* ScenarioSelectHandler::Get()  (line 28)

// Maps scenario_select row IDs to hardcoded campaign name strings.
// Returns nullptr for unknown IDs.
const char* ScenarioSelectHandler::GetCampaignName(int rowId)  (line 38)

// Resolves text API pointers and installs MinHook on CUiScenarioSelect tick
// (RVA from Offsets::FUNC_CUiScenarioSelect_Tick).
void ScenarioSelectHandler::Install()  (line 51)

// Disables/removes tick hook, resets all state atomics and member variables.
void ScenarioSelectHandler::Uninstall()  (line 85)

// Tick detour — MINIMAL. Calls original, then atomically stores thisPtr and
// sets s_tickFired flag. No other work done here.
void __fastcall ScenarioSelectHandler::HookedTick(void* thisPtr, void* param2)  (line 111)

// SEH wrapper for OnFrameInner, file-scope (not a member).
static void OnFrameSEH(ScenarioSelectHandler* handler, void* thisPtr)  (line 127)

// IFrameHandler::OnFrame. Consumes pending scenario_select events from TextCapture.
// Uses s_tickFired (atomic exchange) to detect whether the screen is open.
// On tick stopping: resets interactive/cursor state.
// While tick is running but not yet interactive: waits for prompt text (row 1/101/201)
// from TextCapture events before announcing anything.
// Once interactive: calls OnFrameSEH -> OnFrameInner each frame to track cursor.
void ScenarioSelectHandler::OnFrame()  (line 137)

// Reads cursor and calls AnnounceItem() if it has changed. interrupt=false on
// first announcement (queues after prompt text), true thereafter.
void ScenarioSelectHandler::OnFrameInner(void* thisPtr)  (line 189)

// --- State reading helpers ---

// Reads int32 at thisPtr + Offsets::ScenarioSelect::CURSOR_INDEX (0xE0).
// Returns -1 if value is outside 1-10 (sanity check).
int32_t ScenarioSelectHandler::ReadCursor(void* thisPtr)  (line 206)

// Reads int32 from the item ID array at thisPtr + ITEM_ID_BASE + (cursor-1)*ITEM_ID_STRIDE.
int32_t ScenarioSelectHandler::ReadItemId(void* thisPtr, int32_t cursor)  (line 216)

// Calls LookupText("scenario_select", rowId, language) for description text.
// Returns empty string if API unavailable or text is empty.
std::string ScenarioSelectHandler::LookupDescription(int rowId)  (line 228)

// Builds announcement: "CampaignName, description, N of M" and speaks it.
// interrupt controls whether speech interrupts or queues after the prompt.
void ScenarioSelectHandler::AnnounceItem(int32_t cursor, void* thisPtr, bool interrupt)  (line 258)
