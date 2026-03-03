// File: src/handlers/main_menu_handler.cpp
// Purpose: Accessibility handler for CUiMainMenu (in-game pause menu).
//          Hooks vtable[3] tick via MinHook. Unlike newer handlers, state checking
//          and speech happen inside HookedTick (via HookedTickInner) rather than
//          OnFrame — OnFrame() is a no-op. Uses the game's LookupText API directly
//          to get localised menu item names from main_menu.mbe.
//
// Text lookup: FUN_1401b9260(textManager, "main_menu", id, language)
// main_menu.mbe IDs: 1=Organize, 2=Items, 3=Status, 4=Options,
//   5=Save/Load, 6=Sort Digimon, 7=Farm, 8=Exit
// Cursor at this+0x27D8 (int32), State at this+0x2928 (int16, 3=interactive),
// Item count at this+0x2A08 (int32).

// Singleton accessor.
MainMenuHandler* MainMenuHandler::Get()  (line 21)

// Resolves text API function pointers (GetTextTableManager, LookupText) and
// installs a MinHook on CUiMainMenu's tick function (RVA from Offsets::FUNC_CUiMainMenu_Tick).
void MainMenuHandler::Install()  (line 27)

// Disables/removes the tick hook and resets all state.
void MainMenuHandler::Uninstall()  (line 65)

// MinHook detour for CUiMainMenu tick. Calls original, then delegates to
// HookedTickSEH -> HookedTickInner.
void __fastcall MainMenuHandler::HookedTick(void* thisPtr, void* param2)  (line 86)

// SEH wrapper — no C++ objects, safe for __try/__except.
void MainMenuHandler::HookedTickSEH(void* thisPtr)  (line 100)

// Updates m_lastThisPtr, registers with MemoryInspector, calls CheckStateChanges().
void MainMenuHandler::HookedTickInner(void* thisPtr)  (line 110)

// No-op — all work happens in HookedTick (this handler predates the OnFrame pattern).
void MainMenuHandler::OnFrame()  (line 126)

// --- State reading helpers ---

// Reads int32 at thisPtr + Offsets::MainMenu::CURSOR_INDEX (0x27D8).
int32_t MainMenuHandler::ReadCursor(void* thisPtr)  (line 133)

// Reads int16 at thisPtr + Offsets::MainMenu::STATE (0x2928).
int16_t MainMenuHandler::ReadState(void* thisPtr)  (line 139)

// Reads int32 at thisPtr + Offsets::MainMenu::ITEM_COUNT (0x2A08).
int32_t MainMenuHandler::ReadItemCount(void* thisPtr)  (line 145)

// Calls GetTextTableManager(), reads current language, then calls
// LookupText("main_menu", cursorIndex+1, language). Falls back to
// "item N" if text API is unavailable or returns empty.
std::string MainMenuHandler::LookupMenuItemText(int cursorIndex)  (line 153)

// Compares state/cursor against previous frame values. On state->3 transition
// (menu becomes interactive) calls AnnounceMenuOpened(). On cursor change
// while active calls AnnounceCurrentItem().
void MainMenuHandler::CheckStateChanges(void* thisPtr)  (line 189)

// Speaks the first item on menu open (interrupt). Format: "name, N of M".
void MainMenuHandler::AnnounceMenuOpened(void* thisPtr)  (line 222)

// Speaks the newly selected item (interrupt). Format: "name, N of M".
void MainMenuHandler::AnnounceCurrentItem(void* thisPtr)  (line 238)
