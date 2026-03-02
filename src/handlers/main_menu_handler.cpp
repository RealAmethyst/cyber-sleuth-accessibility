// CUiMainMenu accessibility handler.
//
// Current status: SKELETON — vtable hook infrastructure is ready,
// but member offsets (cursor index, item list) need runtime discovery.
//
// Next steps:
// 1. Attach debugger, set breakpoint on CUiMainMenu vtable[2] (RVA 0x4b49d0)
// 2. When it hits, inspect 'this' pointer (rcx) members
// 3. Find cursor index offset and item count
// 4. Fill in CheckStateChanges() to read those members
// 5. Look up menu item text from game's text tables (MBE CSVs)
//
// The vtable layout for CUiMainMenu (13 entries):
//   [0] 0x4b3d50  — destructor
//   [1] 0x4b4200  — init
//   [2] 0x4b49d0  — update/tick (THIS IS WHAT WE HOOK)
//   [3] 0x4b6270  — draw
//   [4] 0x4b64f0  — state check
//   [5] 0x13bf00  — empty base virtual
//   [6] 0x4b6680  — event handler
//   [7] 0x2a35a0  — base virtual
//   [8] 0x2a35c0  — base virtual
//   [9] 0x13b970  — empty base virtual
//   [10] 0x4b6af0 — callback
//   [11] 0x13b970 — empty base virtual
//   [12] 0x4b7320 — cleanup

#include "handlers/main_menu_handler.h"
#include "speech_manager.h"
#include "offsets.h"
#include "logger.h"

#include <modloader/utils.h>
#include <windows.h>

// Vtable index for the update/tick function
static constexpr int VTABLE_UPDATE_INDEX = 2;

MainMenuHandler* MainMenuHandler::Get()
{
    static MainMenuHandler instance;
    return &instance;
}

void MainMenuHandler::Install(void* menuInstance)
{
    if (m_installed) return;
    if (!menuInstance) return;

    m_menuInstance = menuInstance;

    // Hook vtable[2] (update/tick)
    s_originalUpdate = reinterpret_cast<UpdateFunc>(
        HookVTableEntry(menuInstance, VTABLE_UPDATE_INDEX, (void*)&HookedUpdate));

    if (s_originalUpdate) {
        m_installed = true;
        m_lastCursorIndex = -1;
        m_menuNameAnnounced = false;
        Logger_Log("MainMenu", "VTable hook installed on CUiMainMenu instance %p", menuInstance);
    } else {
        Logger_Log("MainMenu", "FAILED to hook CUiMainMenu vtable");
    }
}

void MainMenuHandler::Uninstall()
{
    if (!m_installed || !m_menuInstance) return;

    // Restore original vtable entry
    if (s_originalUpdate) {
        HookVTableEntry(m_menuInstance, VTABLE_UPDATE_INDEX, (void*)s_originalUpdate);
    }

    s_originalUpdate = nullptr;
    m_menuInstance = nullptr;
    m_installed = false;
    m_lastCursorIndex = -1;
    m_menuNameAnnounced = false;
    Logger_Log("MainMenu", "VTable hook uninstalled");
}

void __fastcall MainMenuHandler::HookedUpdate(void* thisPtr)
{
    // Call original update first
    if (s_originalUpdate) {
        s_originalUpdate(thisPtr);
    }

    // Then check for accessibility-relevant state changes
    auto* handler = Get();
    if (handler->m_installed) {
        handler->CheckStateChanges();
    }
}

void MainMenuHandler::OnFrame()
{
    // For now, OnFrame does nothing extra — all work happens in HookedUpdate.
    // This will be used later for scanning/discovering the CUiMainMenu instance
    // when we detect the menu has opened (e.g., by scanning for objects whose
    // vtable matches VTABLE_CUiMainMenu).
    //
    // TODO: Implement menu instance discovery:
    // 1. On each frame, check if main menu is open (game state table at RVA 0xa59800)
    // 2. If open and not yet hooked, scan for the CUiMainMenu instance
    // 3. Call Install() on the found instance
}

void MainMenuHandler::CheckStateChanges()
{
    // TODO: Read cursor index from this->m_menuInstance + CURSOR_OFFSET
    // TODO: Read item count from this->m_menuInstance + COUNT_OFFSET
    // TODO: Compare with m_lastCursorIndex
    // TODO: If changed, call AnnounceCurrentItem()
    //
    // Member offsets need runtime discovery:
    // - Set breakpoint on RVA 0x4b49d0 (vtable[2])
    // - Inspect rcx (this pointer) when navigating menu
    // - Watch which members change when cursor moves
}

void MainMenuHandler::AnnounceCurrentItem()
{
    // TODO: Look up item name from game text tables
    // Format: "name, description, N of M"
    //
    // Main menu items from csv_text/main_menu.csv:
    //   ID 1 = "Organize"
    //   ID 2 = "Items"
    //   ID 3 = "Status"
    //   ID 4 = "Options"
    //   ID 5 = "Save/Load"
    //
    // For now, just log that we would announce
    // (until we know the cursor offset)
}
