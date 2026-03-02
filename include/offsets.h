#pragma once
#include <cstdint>

// All RVA offsets for Digimon Story CS.exe
// VTable addresses extracted from RTTI analysis (128 CUi* classes found)
// Use: (char*)GetModuleHandle(nullptr) + RVA

namespace Offsets {

// === CUi* VTable RVAs (from Ghidra RTTI extraction) ===
// Each vtable has 13 entries (indices 0-12) for standard CUi classes.
// Common vtable layout (based on CUiBase hierarchy):
//   [0] destructor / first virtual
//   [1] init / setup
//   [2] update / tick  (likely the per-frame update)
//   [3] draw / render
//   [4] some state check
//   [5] 0x13bf00 (empty base virtual)
//   [6] handler / event
//   [7] varies
//   [8] varies
//   [9] 0x13b970 (empty base virtual)
//   [10] some callback
//   [11] 0x13b970 (empty base virtual)
//   [12] cleanup / finalize

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

// --- Common base class empty virtuals (for identification) ---
constexpr uintptr_t FUNC_EMPTY_VIRTUAL_1      = 0x13b970;  // appears at [9], [11] in most vtables
constexpr uintptr_t FUNC_EMPTY_VIRTUAL_2      = 0x13bf00;  // appears at [5] in most vtables

}
