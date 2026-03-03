// File: src/handlers/title_handler.cpp
// Purpose: Accessibility handler for CUiTitle (title screen menu).
//          Hooks vtable[3] tick via MinHook ONLY to capture the this pointer.
//          All state reading and speech happen in OnFrame() (SwapBuffers context).
//
//          Title menu items are pre-baked textures (same in all languages),
//          so names are hardcoded rather than fetched via LookupText.
//          State 12 = interactive menu (19-state machine confirmed by Ghidra).
//          State at this+0xa8 (uint32), cursor at this+0x114 (int32),
//          item count at this+0x124 (int32).
//
// Speech rules:
//   Menu opened (state -> 12): Speak("Title Menu", true) then Speak(item, false)
//   Cursor moved in state 12:  Speak(item, true)

static constexpr uint32_t STATE_INTERACTIVE = 12  (line 24)

// Hardcoded title menu item labels (pre-baked textures, identical in all languages).
static const char* s_titleMenuItems[]  (line 30)
static constexpr int TITLE_MENU_ITEM_COUNT = 4  (line 36)

// Returns the hardcoded string for cursorIndex (0-3). Returns "Unknown" if out of range.
const char* TitleHandler::GetTitleMenuItem(int cursorIndex)  (line 38)

// Singleton accessor.
TitleHandler* TitleHandler::Get()  (line 50)

// Installs MinHook on CUiTitle tick (RVA from Offsets::FUNC_CUiTitle_Tick).
void TitleHandler::Install()  (line 60)

// Disables/removes tick hook, resets all state (including atomic s_thisPtr).
void TitleHandler::Uninstall()  (line 88)

// Tick detour — MINIMAL. Calls original, then atomically stores thisPtr. Nothing else.
void __fastcall TitleHandler::HookedTick(void* thisPtr, void* param2)  (line 113)

// File-scope SEH wrapper for OnFrameInner (separate function required because
// __try/__except cannot coexist with C++ object destructors in the same scope).
static void OnFrameSEH(TitleHandler* handler, void* thisPtr)  (line 132)

// IFrameHandler::OnFrame. Loads thisPtr from atomic, registers with MemoryInspector,
// then calls OnFrameSEH.
void TitleHandler::OnFrame()  (line 142)

// Reads state/cursor/itemCount, detects state->12 transition (calls AnnounceMenuOpened),
// state leaving 12, and cursor changes while active (calls AnnounceCurrentItem).
void TitleHandler::OnFrameInner(void* thisPtr)  (line 151)

// --- State reading helpers ---

// Reads uint32 at thisPtr + Offsets::Title::STATE (0xa8).
uint32_t TitleHandler::ReadState(void* thisPtr)  (line 187)

// Reads int32 at thisPtr + Offsets::Title::CURSOR_INDEX (0x114).
int32_t TitleHandler::ReadCursor(void* thisPtr)  (line 193)

// Reads int32 at thisPtr + Offsets::Title::ITEM_COUNT (0x124).
int32_t TitleHandler::ReadItemCount(void* thisPtr)  (line 199)

// --- Announcements ---

// Speaks "Title Menu" (interrupt), then queues "itemName, N of M" (not interrupt).
void TitleHandler::AnnounceMenuOpened(void* thisPtr)  (line 209)

// Speaks "itemName, N of M" (interrupt) for the newly selected cursor position.
void TitleHandler::AnnounceCurrentItem(void* thisPtr)  (line 227)
