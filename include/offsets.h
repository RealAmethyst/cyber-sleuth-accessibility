#pragma once
#include <cstdint>

// All RVA offsets for Digimon Story CS.exe
// VTable addresses extracted from RTTI analysis (128 CUi* classes found)
// Use: (char*)GetModuleHandle(nullptr) + RVA

namespace Offsets {

// === CUi* VTable RVAs (from Ghidra RTTI extraction) ===
// Each vtable has 13 entries (indices 0-12) for standard CUi classes.
// Confirmed vtable layout (Ghidra decompilation of unpacked exe):
//   [0]  destructor
//   [1]  init / setup (OnOpen)
//   [2]  CLEANUP / TEARDOWN — destroys child objects, zeros pointers
//        NOT per-frame! Only fires on screen transitions.
//   [3]  TICK / UPDATE — per-frame state machine dispatch (**THE REAL UPDATE**)
//        Signature: void tick(this, param_2)
//   [4]  state check / state setter
//   [5]  0x13bf00 (empty base virtual)
//   [6]  message handler (not per-frame)
//   [7]  varies (base virtual)
//   [8]  varies (base virtual)
//   [9]  0x13b970 (empty base virtual)
//   [10] post-tick / render children callback
//   [11] 0x13b970 (empty base virtual)
//   [12] cleanup / finalize

constexpr int VTABLE_TICK_INDEX = 3;   // Per-frame update slot

// --- Key menus ---
constexpr uintptr_t VTABLE_CUiMainMenu       = 0xab0588;
constexpr uintptr_t VTABLE_CUiMenuTop         = 0xab0120;
constexpr uintptr_t VTABLE_CUiTitle           = 0xab09a8;
constexpr uintptr_t VTABLE_CUiTitleLogo       = 0xab0a18;
constexpr uintptr_t VTABLE_CUiScenarioSelect  = 0xab07d8;
constexpr uintptr_t VTABLE_CUiFirstSequence   = 0xaaf750;

// --- System menus ---
constexpr uintptr_t VTABLE_CUiOption          = 0xab0300;
constexpr uintptr_t VTABLE_CUiSaveload        = 0xab0370;
constexpr uintptr_t VTABLE_CUiSettingMenu     = 0xab0468;
constexpr uintptr_t VTABLE_CUiStatusMenu      = 0xab04d8;
constexpr uintptr_t VTABLE_CUiItemMenu        = 0xab0040;
constexpr uintptr_t VTABLE_CUiKeywordMenu     = 0xab00b0;
constexpr uintptr_t VTABLE_CUiCustomSound     = 0xab0190;

// --- Dialog ---
constexpr uintptr_t VTABLE_CUiTalkWindow      = 0xaaef00;
constexpr uintptr_t VTABLE_CUiTalkWindowParts = 0xaaef70;
constexpr uintptr_t VTABLE_CUiTalkCommunication = 0xaaedb0;
constexpr uintptr_t VTABLE_CUiTalkCutinWindow = 0xaaee20;

// --- Battle ---
constexpr uintptr_t VTABLE_CUi_Btl_bace       = 0xaae178;
constexpr uintptr_t VTABLE_CUi_Btl_CommandBace = 0xaae268;
constexpr uintptr_t VTABLE_CUi_Btl_Skill      = 0xaae8e0;
constexpr uintptr_t VTABLE_CUi_Btl_Item       = 0xaae608;
constexpr uintptr_t VTABLE_CUi_Btl_Chang      = 0xaae1f8;
constexpr uintptr_t VTABLE_CUi_Btl_Order      = 0xaae708;
constexpr uintptr_t VTABLE_CUi_Btl_Auto       = 0xaae108;
constexpr uintptr_t VTABLE_CUi_Btl_eacape     = 0xaae3b8;
constexpr uintptr_t VTABLE_CUi_Btl_Info       = 0xaae578;
constexpr uintptr_t VTABLE_CUi_Btl_PlayBace   = 0xaae790;
constexpr uintptr_t VTABLE_CUi_Btl_EnemyBace  = 0xaae498;
constexpr uintptr_t VTABLE_CUi_Btl_ScanBace   = 0xaae800;
constexpr uintptr_t VTABLE_CUi_Btl_ScanMsg    = 0xaae870;
constexpr uintptr_t VTABLE_CUi_Btl_GameOver   = 0xaae508;
constexpr uintptr_t VTABLE_CUiBattleResult     = 0xab0618;
constexpr uintptr_t VTABLE_CUiBattleResultScan = 0xab0688;
constexpr uintptr_t VTABLE_CUiLevelUp         = 0xab0768;
constexpr uintptr_t VTABLE_CUiChooseHoldSkill = 0xab06f8;

// --- DigiBank ---
constexpr uintptr_t VTABLE_CUiDigibankTop     = 0xaad6d8;
constexpr uintptr_t VTABLE_CUiDigibankTopConvert   = 0xaad838;
constexpr uintptr_t VTABLE_CUiDigibankTopEvolution = 0xaad948;
constexpr uintptr_t VTABLE_CUiDigibankTopLoad      = 0xaada38;
constexpr uintptr_t VTABLE_CUiDigibankTopReplace    = 0xaadb28;
constexpr uintptr_t VTABLE_CUiDigibankTopReturnData = 0xaadc18;
constexpr uintptr_t VTABLE_CUiSelectEvolution       = 0xaaddf8;

// --- Farm ---
constexpr uintptr_t VTABLE_CUiDigifarm        = 0xaade68;
constexpr uintptr_t VTABLE_CUiDigifarmGoods   = 0xaaded8;
constexpr uintptr_t VTABLE_CUiDigifarmLeader  = 0xaadf48;
constexpr uintptr_t VTABLE_CUiDigifarmTask    = 0xaadfb8;
constexpr uintptr_t VTABLE_CUiInFarm          = 0xaaf3d0;

// --- Shop ---
constexpr uintptr_t VTABLE_CUiShopMenu        = 0xaaecd0;
constexpr uintptr_t VTABLE_CUiShopBuyList     = 0xaaec60;
constexpr uintptr_t VTABLE_CUiShopSellList    = 0xaaed40;

// --- Quest ---
constexpr uintptr_t VTABLE_CUiQuestBoard      = 0xaaeb10;
constexpr uintptr_t VTABLE_CUiQuestDetail     = 0xaaeb80;
constexpr uintptr_t VTABLE_CUiQuestResult     = 0xaaebf0;

// --- Field ---
constexpr uintptr_t VTABLE_CUiField_001       = 0xaaf0c0;
constexpr uintptr_t VTABLE_CUiField_002       = 0xaaf130;
constexpr uintptr_t VTABLE_CUiFieldChapter    = 0xaaefe0;
constexpr uintptr_t VTABLE_CUiFieldDigiLine   = 0xaae950;
constexpr uintptr_t VTABLE_CUiFieldHacking    = 0xaaf1a0;
constexpr uintptr_t VTABLE_CUiFieldTownMap    = 0xaaf210;

// --- Misc UI ---
constexpr uintptr_t VTABLE_CUiOrganize        = 0xaae9c0;
constexpr uintptr_t VTABLE_CUiSelectMap        = 0xaaf520;
constexpr uintptr_t VTABLE_CUiMapSelect        = 0xaaea30;
constexpr uintptr_t VTABLE_CUiDigimonBookIndex  = 0xaafd58;
constexpr uintptr_t VTABLE_CUiDigimonBookDetail = 0xaafb80;
constexpr uintptr_t VTABLE_CUiYesNoWindow     = 0xaaf6e0;
constexpr uintptr_t VTABLE_CUiMultiSelectWindow = 0xaaf4b0;
constexpr uintptr_t VTABLE_CUiInfoWindow      = 0xaaf2f0;
constexpr uintptr_t VTABLE_CUiMoneyWindow     = 0xaaf440;
constexpr uintptr_t VTABLE_CUiTutorialWindow  = 0xaaf600;
constexpr uintptr_t VTABLE_CUiWindowBackground = 0xaaf670;
constexpr uintptr_t VTABLE_CUiLoading         = 0xaafa30;
constexpr uintptr_t VTABLE_CUiPlayerSetting   = 0xaad1f0;
constexpr uintptr_t VTABLE_CUiHacker          = 0xaafdc8;
constexpr uintptr_t VTABLE_CUiColosseum       = 0xaacfa0;
constexpr uintptr_t VTABLE_CUiRanking         = 0xaad290;
constexpr uintptr_t VTABLE_CUiDigiline        = 0xaafb10;
constexpr uintptr_t VTABLE_CUiEden            = 0xaaf050;
constexpr uintptr_t VTABLE_CUiInternetTop     = 0xaaf360;
constexpr uintptr_t VTABLE_CUiCostumeSelect   = 0xaad668;
constexpr uintptr_t VTABLE_CUiBaseMenu        = 0xaaf9c0;
constexpr uintptr_t VTABLE_CUiAuthorization   = 0xaac810;
constexpr uintptr_t VTABLE_CUiMedalList       = 0xaaeaa0;

// === Tick function RVAs (vtable[3] contents) ===
// These are the actual code addresses for the per-frame tick/update.
// Use with MinHook (MH_CreateHook) for reliable interception regardless
// of how the game dispatches calls (vtable, direct, table-driven).
// Signature: void __fastcall tick(void* thisPtr, void* param2)
constexpr uintptr_t FUNC_CUiTitle_Tick        = 0x4CC2F0;
constexpr uintptr_t FUNC_CUiTitleLogo_Tick    = 0x4CF430;
constexpr uintptr_t FUNC_CUiFirstSequence_Tick = 0x4295C0;
constexpr uintptr_t FUNC_CUiMainMenu_Tick     = 0x4B6270;
constexpr uintptr_t FUNC_CUiYesNoWindow_Tick  = 0x426C90;
constexpr uintptr_t FUNC_CUiScenarioSelect_Tick = 0x4C89A0;
constexpr uintptr_t FUNC_CUiOption_Tick        = 0x48DD20;
constexpr uintptr_t FUNC_CUiPlayerSetting_Tick = 0x337CF0;
constexpr uintptr_t FUNC_CUiTalkWindow_Tick   = 0x40F480;

// === CUiTalkWindow member offsets (from Ghidra decompilation) ===
// State machine: 13 states (0-12), dispatched via function table at this+0x08.
// State 7 = interactive (dialog visible, waiting for player input).
// Tick fires continuously while the dialog system is active.
namespace TalkWindow {
    constexpr uintptr_t STATE             = 0x70;   // uint32, main state
    constexpr uintptr_t SUB_STATE         = 0x74;   // uint32, next/sub state
    constexpr uintptr_t PARTS_SIDE0       = 0x80;   // ptr, TalkWindowParts (left/side 0)
    constexpr uintptr_t PARTS_SIDE1       = 0x88;   // ptr, TalkWindowParts (right/side 1)
    constexpr uintptr_t MSG_DATA          = 0xB0;   // 28 bytes, current message data
    constexpr uintptr_t ACTIVE_SIDE       = 0x2E0;  // int32, 0 or 1
    constexpr uintptr_t HAS_SPEAKER       = 0x2E4;  // byte, speaker name present
    constexpr uintptr_t ACTIVE_FLAG       = 0x2F0;  // byte, dialog is active
    constexpr uintptr_t MSG_PENDING       = 0x2F1;  // byte, message pending
    constexpr uintptr_t MULTI_PAGE        = 0x2F2;  // byte, multi-page flag
    constexpr uintptr_t CURSOR_VISIBLE    = 0x2F4;  // byte, cursor visible
    constexpr uintptr_t SPEAKER_VISIBLE   = 0x2F5;  // byte, speaker name visible
    constexpr uintptr_t CLOSE_REQUESTED   = 0x2F9;  // byte, close requested
    constexpr uintptr_t PAGE_INDEX        = 0x304;  // int32, current page (0-based)
    constexpr uintptr_t PAGE_COUNT        = 0x308;  // int32, total pages

    // Text widget pointers — dereference widget, then read char* at widget+0x28
    constexpr uintptr_t BODY_TEXT_WIDGET  = 0xF0;   // ptr to text widget for dialog body
    constexpr uintptr_t SPEAKER_WIDGET    = 0x1E8;  // ptr to text widget for speaker name
    constexpr uintptr_t WIDGET_TEXT_PTR   = 0x28;   // char* inside the text widget object

    // Speaker ID tracking
    constexpr uintptr_t LAST_SPEAKER_ID   = 0xE8;   // int32, last speaker character ID
}

// === CUiTalkCommunication member offsets (from Ghidra decompilation) ===
// Chat room / EDEN communication scenes.
// State machine: 0=idle, 1=opening, 2=setup, 3=animating, 4=closing
// TalkWindowParts at this+0x08 holds the chat bubble widget.
constexpr uintptr_t FUNC_CUiTalkCommunication_Tick = 0x40C450;

namespace TalkComm {
    constexpr uintptr_t PARTS_WIDGET      = 0x08;   // ptr, TalkWindowParts (chat bubble)
    constexpr uintptr_t MSG_HANDLER_ID    = 0x10;   // int32, message handler ID
    constexpr uintptr_t DONE_FLAG         = 0x14;   // byte, done/return value
    constexpr uintptr_t STATE             = 0x18;   // int32, state machine
    constexpr uintptr_t POSITION_ID       = 0x20;   // int32, position
    constexpr uintptr_t SPEAKER_ID        = 0x24;   // int32, speaker/offset ID
    constexpr uintptr_t SPEAKER_NAME      = 0x28;   // SSO string (32 bytes), speaker name
    constexpr uintptr_t TEXT_STRING        = 0x48;   // SSO string (32 bytes), message text (?)
    constexpr uintptr_t RESOURCE_NAME     = 0x68;   // SSO string (32 bytes), resource name
    constexpr uintptr_t SCENE_OBJECT      = 0x88;   // ptr, 3D scene display (0x2b0)
    constexpr uintptr_t CAMERA_OBJECT     = 0x90;   // ptr, camera (0x50)

    // SSO string layout (MSVC): 16-byte inline buffer, then size_t size at +0x10,
    // size_t capacity at +0x18. If capacity >= 16, first 8 bytes = heap pointer.
    constexpr uintptr_t SSO_CAPACITY_OFFSET = 0x18;  // relative to SSO string start
}

// === CUiPlayerSetting member offsets (from Ghidra decompilation) ===
// Naming/avatar selection screen. 9-state machine dispatched via function table at this+0x10.
// State 4 = interactive (avatar selection with tabs). Tabs 0-3 cycled with L/R.
// Each tab has a list object with selectable items.
namespace PlayerSetting {
    constexpr uintptr_t STATE             = 0x58;   // uint32, main state (0-8)
    constexpr uintptr_t NEXT_STATE        = 0x5C;   // uint32, next/target state
    constexpr uintptr_t DONE_FLAG         = 0x68;   // byte, screen closing
    constexpr uintptr_t INTERACTIVE_FLAG  = 0x6A;   // byte, interactive mode active
    constexpr uintptr_t TAB_INDEX         = 0x1C0;  // int32, current tab (0-3)
    constexpr uintptr_t TAB_LIST_BASE     = 0x1A0;  // array of 4 tab list object pointers
    constexpr uintptr_t TAB_LIST_STRIDE   = 0x08;   // 8 bytes per pointer

    // Tab list object member offsets
    constexpr uintptr_t LIST_BEGIN        = 0x08;   // ptr, list data begin
    constexpr uintptr_t LIST_END          = 0x10;   // ptr, list data end
    constexpr uintptr_t LIST_SELECTED     = 0x20;   // int32, selected index (absolute)
    constexpr uintptr_t LIST_SCROLL       = 0x28;   // int32, scroll offset
    constexpr uintptr_t LIST_ENTRY_SIZE   = 0x10;   // 16 bytes per entry
}

// === CUiScenarioSelect member offsets (from memory dumps) ===
// Tick fires during the cutscene AND the interactive menu.
// The interactive phase begins when scenario_select:1 (prompt text) is looked up.
// Cursor is 1-based: 1 = first item (HM, row ID 2), 2 = second item (CS, row ID 3).
namespace ScenarioSelect {
    constexpr uintptr_t ITEM_ID_BASE     = 0xD0;   // int32 array, item row IDs (scenario_select table)
    constexpr uintptr_t ITEM_ID_STRIDE   = 0x04;
    constexpr uintptr_t CURSOR_INDEX     = 0xE0;   // int32, 1-based cursor (1=HM, 2=CS for normal new game)
}

// === CUiTitleLogo member offsets ===
namespace TitleLogo {
    constexpr uintptr_t STATE = 0xF8;  // int32, state machine index (29 states, function table dispatch at +0x10)
}

// Old vtable[2] cleanup/teardown RVAs (NOT per-frame — only fire on transitions)
constexpr uintptr_t FUNC_CUiTitle_Cleanup     = 0x4CC160;
constexpr uintptr_t FUNC_CUiMainMenu_Cleanup  = 0x4B49D0;

// --- Common base class empty virtuals (for identification) ---
constexpr uintptr_t FUNC_EMPTY_VIRTUAL_1      = 0x13b970;  // appears at [9], [11] in most vtables
constexpr uintptr_t FUNC_EMPTY_VIRTUAL_2      = 0x13bf00;  // appears at [5] in most vtables

// === CUiMainMenu member offsets (from Ghidra decompilation) ===
namespace MainMenu {
    constexpr uintptr_t CURSOR_INDEX    = 0x27D8;  // int32, 0-7 (wraps)
    constexpr uintptr_t STATE           = 0x2928;   // int16, states 0-5
    constexpr uintptr_t DONE_FLAG       = 0x2828;   // byte, signals menu closing
    constexpr uintptr_t ITEM_COUNT      = 0x2A08;   // int32, max 8
    constexpr uintptr_t ITEMS_BASE      = 0x160;    // 8 item slots, stride 0x50
    constexpr uintptr_t ITEM_STRIDE     = 0x50;
    constexpr uintptr_t CATEGORY_BASE   = 0x3E0;    // 3 category entries, stride 0x300
    constexpr uintptr_t CATEGORY_STRIDE = 0x300;
    constexpr uintptr_t CATEGORY_INDEX  = 0x29F0;   // int32, selected category
    constexpr uintptr_t SUBITEM_INDEX   = 0x29F8;   // int32, selected sub-item
}

// === CUiTitle member offsets (from Ghidra decompilation) ===
namespace Title {
    constexpr uintptr_t STATE           = 0xa8;   // uint32, state machine index (12 = interactive)
    constexpr uintptr_t DONE_FLAG       = 0xb8;   // byte, exit flag
    constexpr uintptr_t MODE_FLAG       = 0xbc;   // int32, 0 = normal title
    constexpr uintptr_t CAMPAIGN        = 0xc0;   // int32, campaign select (0-3)
    constexpr uintptr_t LOCKED_FLAG     = 0xc4;   // byte, 0 = "New Game +" locked
    constexpr uintptr_t CURSOR_INDEX    = 0x114;  // int32, 0-3
    constexpr uintptr_t PREV_CURSOR     = 0x118;  // int32, previous cursor
    constexpr uintptr_t ITEM_COUNT      = 0x124;  // int32, max selectable items
}

// === CUiYesNoWindow member offsets (from Ghidra decompilation of tick + message handler) ===
// Tick: RVA 0x426c90 (365 bytes), message handler: RVA 0x426e50
// State machine: 0=idle, 1=ready, 2=opening, 3=opening-anim, 4=interactive, 5=closing, 6=done
// Result: 1=Yes, 2=No, 3=Cancel (read by Squirrel GetResultYesNoInfo)
namespace YesNoWindow {
    constexpr uintptr_t STATE           = 0x0C;   // int32, state machine
    constexpr uintptr_t ACTIVE_WINDOW   = 0x40;   // ptr, active window widget
    constexpr uintptr_t ACTIVE_YES_CUR  = 0x48;   // ptr, Yes cursor widget
    constexpr uintptr_t ACTIVE_NO_CUR   = 0x50;   // ptr, No cursor widget
    constexpr uintptr_t STYLE_TYPE      = 0x58;   // int32, 1=info, 2=system
    constexpr uintptr_t YES_TEXT_ID     = 0x78;   // int32, common_message ID for Yes label (usually 2100)
    constexpr uintptr_t NO_TEXT_ID      = 0x7C;   // int32, common_message ID for No label (usually 2101)
    constexpr uintptr_t CANCEL_ENABLED  = 0x80;   // byte, cancel button available
    constexpr uintptr_t CURSOR_INDEX    = 0x81;   // byte, 0=No (bottom), 1=Yes (top) — Japanese UI convention
    constexpr uintptr_t RESULT          = 0x84;   // int32, 0=none, 1=Yes, 2=No, 3=Cancel
    constexpr uintptr_t ACTIVE_FLAG     = 0x88;   // byte, visible/active
}

// === Text system RVAs ===
namespace Text {
    // Text table manager singleton
    constexpr uintptr_t FUNC_GetTextTableManager = 0x1B8BC0;  // returns void*
    constexpr uintptr_t DAT_TextTableManager     = 0xF205D8;  // cached singleton

    // Core text lookup: char* LookupText(manager, tableName, rowId, language)
    constexpr uintptr_t FUNC_LookupText          = 0x1B9260;

    // Language settings singleton
    constexpr uintptr_t DAT_LanguageSettings      = 0xF206C8;  // ptr to 0x150 object
    constexpr uintptr_t LANGUAGE_INDEX_OFFSET      = 0xB4;      // int32 at settings+0xB4

    // Convenience wrappers (call LookupText internally)
    constexpr uintptr_t FUNC_GetItemName          = 0x4B7C70;  // char*(int id) — "item_name" table
    constexpr uintptr_t FUNC_GetSkillName         = 0x4B7E60;  // char*(int id) — "skill_name" table

    // Data table manager (for non-text game data)
    constexpr uintptr_t FUNC_GetDataTableManager  = 0x1B5300;
    constexpr uintptr_t FUNC_DataTableLookup      = 0x1B5920;
}

// === Vista / Subtitle system RVAs ===
namespace Vista {
    // Vista singleton — the cinematic/cutscene manager
    constexpr uintptr_t DAT_VistaSingleton       = 0xF205E8;  // ptr to Vista object

    // Subtitle loader singleton (0xa8-byte object, created on first LoadSubtitle call)
    // Contains the loaded schedule data, cue vector, and playback state.
    // The game's per-frame update (FUN_1401b66c0) advances cues based on time.
    constexpr uintptr_t DAT_SubtitleLoader       = 0xF205D0;  // ptr to loader object

    // Subtitle loader member offsets (from Ghidra decompilation of FUN_1401b66c0)
    namespace Loader {
        constexpr uintptr_t STATE        = 0x4C;  // int32: 1=loaded, 2=playing, 5=ending
        constexpr uintptr_t TIME         = 0x58;  // int32: current playback time (ms)
        constexpr uintptr_t PREV_TIME    = 0x5C;  // int32: previous frame time (ms)
        constexpr uintptr_t NAME         = 0x68;  // ptr: subtitle schedule name (char*)
        constexpr uintptr_t VECTOR_BEGIN = 0x80;  // ptr: cue vector begin
        constexpr uintptr_t VECTOR_END   = 0x88;  // ptr: cue vector end (write pos)
        constexpr uintptr_t CURSOR       = 0x98;  // ptr: current cue cursor (into vector)
        constexpr uintptr_t DONE         = 0xA0;  // byte: done flag
    }

    // Cue data layout (from Ghidra — per cue in the vector):
    //   vector entry (8 bytes) → cueStruct ptr (16-byte struct)
    //   cueStruct[0] = row data ptr (points directly to column data)
    //   cueStruct[1] = widget ptr (0xf8-byte subtitle widget)
    //   Row data layout (3 dereferences total: vector → cueStruct → rowData):
    //     +0x00: int32 id
    //     +0x04: int32 StartTime (ms)
    //     +0x08: int32 EndTime (ms)
    //     +0x0C: int32 SubtitleTextID
    constexpr uintptr_t CUE_COL_STARTTIME = 0x04;
    constexpr uintptr_t CUE_COL_ENDTIME   = 0x08;
    constexpr uintptr_t CUE_COL_TEXTID    = 0x0C;
}

// === CUiOption member offsets (from memory dumps) ===
// Sub-screen state: 5=main settings tab, 17=graphic options submenu
// Cursor is a global index across all tabs.
// Item count at +0x0344 = 13 (max index) on main tab.
namespace Option {
    // State handler function RVAs — dispatched from the tick's function table
    // at this+0x10 indexed by the state at this+0x138.
    // These only fire when the menu is in that interactive state (zero overhead when idle).
    constexpr uintptr_t FUNC_State5_Main    = 0x492DA0;  // Main options tab (1996 bytes)
    constexpr uintptr_t FUNC_State13_Button = 0x493A20;  // Button settings (152 bytes)
    constexpr uintptr_t FUNC_State17_Graphic = 0x493C50; // Graphic options (152 bytes)

    constexpr uintptr_t SUB_SCREEN       = 0x0138;  // int32, sub-screen state (5=main, 17=graphic options)
    constexpr uintptr_t CURSOR_INDEX     = 0x0338;  // int32, current cursor position
    constexpr uintptr_t PREV_CURSOR      = 0x033C;  // int32, previous cursor position
    constexpr uintptr_t ITEM_COUNT_MAX   = 0x0344;  // int32, max cursor index (13 on main tab)

    // Global singleton that manages Graphic/Button sub-menus.
    // State 17 (graphic) calls FUN_1404dbf40(DAT_140f20848, 0x90, 0, 0).
    // The cursor for graphics options lives inside a sub-object (not in CUiOption).
    constexpr uintptr_t DAT_GraphicGlobal = 0xF20848;  // ptr to global manager object

    // GraphicGlobal internal layout:
    //   +0x10: vector<T*> begin (pointers to wrapper objects)
    //   +0x18: vector<T*> end
    // Each wrapper's first 8 bytes point to an inner "screen" object.
    // Inner object layout:
    //   +0x08: int32 command ID (0x90 = graphic, 0x8f = button)
    //   +0x1A0: int32 cursor index (0-based)
    //   +0x1A4: int32 previous cursor
    //   +0x1AC: int32 item count
    constexpr int GRAPHIC_CMD_ID      = 0x90;
    constexpr int BUTTON_CMD_ID       = 0x8F;
    constexpr uintptr_t INNER_CMD_ID  = 0x08;   // int32 command ID
    constexpr uintptr_t INNER_CURSOR  = 0x1A0;  // int32 cursor index
    constexpr uintptr_t INNER_PREV    = 0x1A4;  // int32 previous cursor
    constexpr uintptr_t INNER_COUNT   = 0x1AC;  // int32 item count
}

// === Game context RVAs ===
namespace Context {
    constexpr uintptr_t DAT_GameContext           = 0xF205C0;  // ptr to GameContext singleton
    constexpr uintptr_t FUNC_GetGameContext       = 0x1A70D0;  // returns GameContext*
    constexpr uintptr_t STORY_MODE_OFFSET         = 0x90;      // ptr at GameContext+0x90
    constexpr uintptr_t STORY_MODE_CHECK          = 0x08;      // int at storyMode+8 (0=CS, else=HM)
    constexpr uintptr_t DIGIMON_CS_OFFSET         = 0x10;      // data ptr for Cyber Sleuth
    constexpr uintptr_t DIGIMON_HM_OFFSET         = 0x60;      // data ptr for Hacker's Memory
    constexpr uintptr_t MENU_ITEMS_VECTOR         = 0x20;      // std::vector<char*> begin at data+0x20
}

}
