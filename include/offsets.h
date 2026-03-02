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
constexpr uintptr_t FUNC_CUiMainMenu_Tick     = 0x4B6270;

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
