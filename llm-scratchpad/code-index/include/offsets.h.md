// File: include/offsets.h
// Purpose: Single source of truth for all RVA offsets into Digimon Story CS.exe.
//          VTable addresses were extracted from RTTI analysis (128 CUi* classes).
//          Usage: (char*)GetModuleHandle(nullptr) + RVA
//
// Vtable layout (13 entries, indices 0-12):
//   [0]  destructor
//   [1]  init / setup (OnOpen)
//   [2]  CLEANUP / TEARDOWN — fires only on screen transitions, NOT per-frame
//   [3]  TICK / UPDATE — per-frame state machine (THE real update)
//   [4]  state check / state setter
//   [5]  0x13bf00 (empty base virtual)
//   [6]  message handler (not per-frame)
//   [7-8] varies (base virtual)
//   [9]  0x13b970 (empty base virtual)
//   [10] post-tick / render children callback
//   [11] 0x13b970 (empty base virtual)
//   [12] cleanup / finalize

namespace Offsets {

// Index of the per-frame tick slot in all standard CUi vtables.
constexpr int VTABLE_TICK_INDEX = 3;  // (line 29)

// --- Key menus (vtable RVAs) ---
constexpr uintptr_t VTABLE_CUiMainMenu       = 0xab0588;  // (line 32)
constexpr uintptr_t VTABLE_CUiMenuTop         = 0xab0120;  // (line 33)
constexpr uintptr_t VTABLE_CUiTitle           = 0xab09a8;  // (line 34)
constexpr uintptr_t VTABLE_CUiTitleLogo       = 0xab0a18;  // (line 35)
constexpr uintptr_t VTABLE_CUiScenarioSelect  = 0xab07d8;  // (line 36)
constexpr uintptr_t VTABLE_CUiFirstSequence   = 0xaaf750;  // (line 37)

// --- System menus ---
constexpr uintptr_t VTABLE_CUiOption          = 0xab0300;  // (line 40)
constexpr uintptr_t VTABLE_CUiSaveload        = 0xab0370;  // (line 41)
constexpr uintptr_t VTABLE_CUiSettingMenu     = 0xab0468;  // (line 42)
constexpr uintptr_t VTABLE_CUiStatusMenu      = 0xab04d8;  // (line 43)
constexpr uintptr_t VTABLE_CUiItemMenu        = 0xab0040;  // (line 44)
constexpr uintptr_t VTABLE_CUiKeywordMenu     = 0xab00b0;  // (line 45)
constexpr uintptr_t VTABLE_CUiCustomSound     = 0xab0190;  // (line 46)

// --- Dialog ---
constexpr uintptr_t VTABLE_CUiTalkWindow        = 0xaaef00;  // (line 49)
constexpr uintptr_t VTABLE_CUiTalkWindowParts   = 0xaaef70;  // (line 50)
constexpr uintptr_t VTABLE_CUiTalkCommunication = 0xaaedb0;  // (line 51)
constexpr uintptr_t VTABLE_CUiTalkCutinWindow   = 0xaaee20;  // (line 52)

// --- Battle ---
constexpr uintptr_t VTABLE_CUi_Btl_bace        = 0xaae178;  // (line 55)
constexpr uintptr_t VTABLE_CUi_Btl_CommandBace  = 0xaae268;  // (line 56)
constexpr uintptr_t VTABLE_CUi_Btl_Skill        = 0xaae8e0;  // (line 57)
constexpr uintptr_t VTABLE_CUi_Btl_Item         = 0xaae608;  // (line 58)
constexpr uintptr_t VTABLE_CUi_Btl_Chang        = 0xaae1f8;  // (line 59)
constexpr uintptr_t VTABLE_CUi_Btl_Order        = 0xaae708;  // (line 60)
constexpr uintptr_t VTABLE_CUi_Btl_Auto         = 0xaae108;  // (line 61)
constexpr uintptr_t VTABLE_CUi_Btl_eacape       = 0xaae3b8;  // (line 62) — "eacape" is the game's own typo
constexpr uintptr_t VTABLE_CUi_Btl_Info         = 0xaae578;  // (line 63)
constexpr uintptr_t VTABLE_CUi_Btl_PlayBace     = 0xaae790;  // (line 64)
constexpr uintptr_t VTABLE_CUi_Btl_EnemyBace    = 0xaae498;  // (line 65)
constexpr uintptr_t VTABLE_CUi_Btl_ScanBace     = 0xaae800;  // (line 66)
constexpr uintptr_t VTABLE_CUi_Btl_ScanMsg      = 0xaae870;  // (line 67)
constexpr uintptr_t VTABLE_CUi_Btl_GameOver     = 0xaae508;  // (line 68)
constexpr uintptr_t VTABLE_CUiBattleResult      = 0xab0618;  // (line 69)
constexpr uintptr_t VTABLE_CUiBattleResultScan  = 0xab0688;  // (line 70)
constexpr uintptr_t VTABLE_CUiLevelUp           = 0xab0768;  // (line 71)
constexpr uintptr_t VTABLE_CUiChooseHoldSkill   = 0xab06f8;  // (line 72)

// --- DigiBank ---
constexpr uintptr_t VTABLE_CUiDigibankTop            = 0xaad6d8;  // (line 75)
constexpr uintptr_t VTABLE_CUiDigibankTopConvert      = 0xaad838;  // (line 76)
constexpr uintptr_t VTABLE_CUiDigibankTopEvolution    = 0xaad948;  // (line 77)
constexpr uintptr_t VTABLE_CUiDigibankTopLoad         = 0xaada38;  // (line 78)
constexpr uintptr_t VTABLE_CUiDigibankTopReplace      = 0xaadb28;  // (line 79)
constexpr uintptr_t VTABLE_CUiDigibankTopReturnData   = 0xaadc18;  // (line 80)
constexpr uintptr_t VTABLE_CUiSelectEvolution         = 0xaaddf8;  // (line 81)

// --- Farm ---
constexpr uintptr_t VTABLE_CUiDigifarm       = 0xaade68;  // (line 84)
constexpr uintptr_t VTABLE_CUiDigifarmGoods  = 0xaaded8;  // (line 85)
constexpr uintptr_t VTABLE_CUiDigifarmLeader = 0xaadf48;  // (line 86)
constexpr uintptr_t VTABLE_CUiDigifarmTask   = 0xaadfb8;  // (line 87)
constexpr uintptr_t VTABLE_CUiInFarm         = 0xaaf3d0;  // (line 88)

// --- Shop ---
constexpr uintptr_t VTABLE_CUiShopMenu      = 0xaaecd0;  // (line 91)
constexpr uintptr_t VTABLE_CUiShopBuyList   = 0xaaec60;  // (line 92)
constexpr uintptr_t VTABLE_CUiShopSellList  = 0xaaed40;  // (line 93)

// --- Quest ---
constexpr uintptr_t VTABLE_CUiQuestBoard   = 0xaaeb10;  // (line 96)
constexpr uintptr_t VTABLE_CUiQuestDetail  = 0xaaeb80;  // (line 97)
constexpr uintptr_t VTABLE_CUiQuestResult  = 0xaaebf0;  // (line 98)

// --- Field ---
constexpr uintptr_t VTABLE_CUiField_001      = 0xaaf0c0;  // (line 101)
constexpr uintptr_t VTABLE_CUiField_002      = 0xaaf130;  // (line 102)
constexpr uintptr_t VTABLE_CUiFieldChapter   = 0xaaefe0;  // (line 103)
constexpr uintptr_t VTABLE_CUiFieldDigiLine  = 0xaae950;  // (line 104)
constexpr uintptr_t VTABLE_CUiFieldHacking   = 0xaaf1a0;  // (line 105)
constexpr uintptr_t VTABLE_CUiFieldTownMap   = 0xaaf210;  // (line 106)

// --- Misc UI ---
constexpr uintptr_t VTABLE_CUiOrganize          = 0xaae9c0;  // (line 109)
constexpr uintptr_t VTABLE_CUiSelectMap         = 0xaaf520;  // (line 110)
constexpr uintptr_t VTABLE_CUiMapSelect         = 0xaaea30;  // (line 111)
constexpr uintptr_t VTABLE_CUiDigimonBookIndex  = 0xaafd58;  // (line 112)
constexpr uintptr_t VTABLE_CUiDigimonBookDetail = 0xaafb80;  // (line 113)
constexpr uintptr_t VTABLE_CUiYesNoWindow       = 0xaaf6e0;  // (line 114)
constexpr uintptr_t VTABLE_CUiMultiSelectWindow = 0xaaf4b0;  // (line 115)
constexpr uintptr_t VTABLE_CUiInfoWindow        = 0xaaf2f0;  // (line 116)
constexpr uintptr_t VTABLE_CUiMoneyWindow       = 0xaaf440;  // (line 117)
constexpr uintptr_t VTABLE_CUiTutorialWindow    = 0xaaf600;  // (line 118)
constexpr uintptr_t VTABLE_CUiWindowBackground  = 0xaaf670;  // (line 119)
constexpr uintptr_t VTABLE_CUiLoading           = 0xaafa30;  // (line 120)
constexpr uintptr_t VTABLE_CUiPlayerSetting     = 0xaad1f0;  // (line 121)
constexpr uintptr_t VTABLE_CUiHacker            = 0xaafdc8;  // (line 122)
constexpr uintptr_t VTABLE_CUiColosseum         = 0xaacfa0;  // (line 123)
constexpr uintptr_t VTABLE_CUiRanking           = 0xaad290;  // (line 124)
constexpr uintptr_t VTABLE_CUiDigiline          = 0xaafb10;  // (line 125)
constexpr uintptr_t VTABLE_CUiEden              = 0xaaf050;  // (line 126)
constexpr uintptr_t VTABLE_CUiInternetTop       = 0xaaf360;  // (line 127)
constexpr uintptr_t VTABLE_CUiCostumeSelect     = 0xaad668;  // (line 128)
constexpr uintptr_t VTABLE_CUiBaseMenu          = 0xaaf9c0;  // (line 129)
constexpr uintptr_t VTABLE_CUiAuthorization     = 0xaac810;  // (line 130)
constexpr uintptr_t VTABLE_CUiMedalList         = 0xaaeaa0;  // (line 131)

// === Tick function RVAs (actual code addresses for vtable[3] contents) ===
// Used with MH_CreateHook; signature: void __fastcall tick(void* thisPtr, void* param2)
// Note: MinHook on function prologues works even when the game uses table-driven dispatch
//       that bypasses normal virtual calls.
constexpr uintptr_t FUNC_CUiTitle_Tick          = 0x4CC2F0;  // (line 138)
constexpr uintptr_t FUNC_CUiMainMenu_Tick        = 0x4B6270;  // (line 139)
constexpr uintptr_t FUNC_CUiYesNoWindow_Tick     = 0x426C90;  // (line 140)
constexpr uintptr_t FUNC_CUiScenarioSelect_Tick  = 0x4C89A0;  // (line 141)

// === CUiScenarioSelect member offsets ===
// Tick fires during cutscene AND interactive menu.
// Interactive phase is signaled by TextCapture seeing scenario_select:1.
// Cursor is 1-based: 1 = first item (HM row ID 2), 2 = second item (CS row ID 3).
namespace ScenarioSelect {  // (line 147)
    constexpr uintptr_t ITEM_ID_BASE   = 0xD0;  // int32 array of scenario_select row IDs  (line 148)
    constexpr uintptr_t ITEM_ID_STRIDE = 0x04;  // (line 149)
    constexpr uintptr_t CURSOR_INDEX   = 0xE0;  // int32, 1-based cursor                   (line 150)
}

// Old cleanup/teardown function RVAs — NOT per-frame, fire only on transitions.
constexpr uintptr_t FUNC_CUiTitle_Cleanup    = 0x4CC160;  // (line 154)
constexpr uintptr_t FUNC_CUiMainMenu_Cleanup = 0x4B49D0;  // (line 155)

// Empty base-class virtual stubs used for identification.
constexpr uintptr_t FUNC_EMPTY_VIRTUAL_1 = 0x13b970;  // (line 158) — at vtable[9] and [11]
constexpr uintptr_t FUNC_EMPTY_VIRTUAL_2 = 0x13bf00;  // (line 159) — at vtable[5]

// === CUiMainMenu member offsets ===
namespace MainMenu {  // (line 162)
    constexpr uintptr_t CURSOR_INDEX    = 0x27D8;  // int32, 0-7 wraps         (line 163)
    constexpr uintptr_t STATE           = 0x2928;  // int16, states 0-5        (line 164)
    constexpr uintptr_t DONE_FLAG       = 0x2828;  // byte, menu closing        (line 165)
    constexpr uintptr_t ITEM_COUNT      = 0x2A08;  // int32, max 8              (line 166)
    constexpr uintptr_t ITEMS_BASE      = 0x160;   // 8 item slots              (line 167)
    constexpr uintptr_t ITEM_STRIDE     = 0x50;    // (line 168)
    constexpr uintptr_t CATEGORY_BASE   = 0x3E0;   // 3 category entries        (line 169)
    constexpr uintptr_t CATEGORY_STRIDE = 0x300;   // (line 170)
    constexpr uintptr_t CATEGORY_INDEX  = 0x29F0;  // int32, selected category  (line 171)
    constexpr uintptr_t SUBITEM_INDEX   = 0x29F8;  // int32, selected sub-item  (line 172)
}

// === CUiTitle member offsets ===
namespace Title {  // (line 176)
    constexpr uintptr_t STATE        = 0xa8;   // uint32, state machine (12 = interactive)  (line 177)
    constexpr uintptr_t DONE_FLAG    = 0xb8;   // byte, exit flag                            (line 178)
    constexpr uintptr_t MODE_FLAG    = 0xbc;   // int32, 0 = normal title                   (line 179)
    constexpr uintptr_t CAMPAIGN     = 0xc0;   // int32, campaign select (0-3)              (line 180)
    constexpr uintptr_t LOCKED_FLAG  = 0xc4;   // byte, 0 = "New Game +" locked             (line 181)
    constexpr uintptr_t CURSOR_INDEX = 0x114;  // int32, 0-3                                (line 182)
    constexpr uintptr_t PREV_CURSOR  = 0x118;  // int32, previous cursor                    (line 183)
    constexpr uintptr_t ITEM_COUNT   = 0x124;  // int32, max selectable items               (line 184)
}

// === CUiYesNoWindow member offsets ===
// State machine: 0=idle, 1=ready, 2=opening, 3=opening-anim, 4=interactive, 5=closing, 6=done
// Result values: 1=Yes, 2=No, 3=Cancel
namespace YesNoWindow {  // (line 191)
    constexpr uintptr_t STATE          = 0x0C;  // int32, state machine              (line 192)
    constexpr uintptr_t ACTIVE_WINDOW  = 0x40;  // ptr, active window widget          (line 193)
    constexpr uintptr_t ACTIVE_YES_CUR = 0x48;  // ptr, Yes cursor widget             (line 194)
    constexpr uintptr_t ACTIVE_NO_CUR  = 0x50;  // ptr, No cursor widget              (line 195)
    constexpr uintptr_t STYLE_TYPE     = 0x58;  // int32, 1=info, 2=system            (line 196)
    constexpr uintptr_t YES_TEXT_ID    = 0x78;  // int32, common_message row ID (2100)(line 197)
    constexpr uintptr_t NO_TEXT_ID     = 0x7C;  // int32, common_message row ID (2101)(line 198)
    constexpr uintptr_t CANCEL_ENABLED = 0x80;  // byte, B button dismiss available   (line 199)
    // IMPORTANT: cursor 0 = Yes (top), cursor 1 = No (bottom) — counter-intuitive
    constexpr uintptr_t CURSOR_INDEX   = 0x81;  // byte, 0=Yes, 1=No                  (line 200)
    constexpr uintptr_t RESULT         = 0x84;  // int32, 0=none/1=Yes/2=No/3=Cancel  (line 201)
    constexpr uintptr_t ACTIVE_FLAG    = 0x88;  // byte, visible/active               (line 202)
}

// === Text system RVAs ===
namespace Text {  // (line 206)
    constexpr uintptr_t FUNC_GetTextTableManager = 0x1B8BC0;  // returns void*        (line 208)
    constexpr uintptr_t DAT_TextTableManager     = 0xF205D8;  // cached singleton ptr  (line 209)

    // Core lookup: char* LookupText(manager, tableName, rowId, language)
    constexpr uintptr_t FUNC_LookupText          = 0x1B9260;  // (line 212)

    constexpr uintptr_t DAT_LanguageSettings     = 0xF206C8;  // ptr to 0x150 settings object (line 215)
    constexpr uintptr_t LANGUAGE_INDEX_OFFSET    = 0xB4;      // int32 at settings+0xB4        (line 216)
    // Language values: 0=Japanese, 1=English, 2=Chinese, 3=EnglishCensored, 4=Korean, 5=German

    // Convenience wrappers (call LookupText internally)
    constexpr uintptr_t FUNC_GetItemName  = 0x4B7C70;  // char*(int id) — "item_name" table  (line 219)
    constexpr uintptr_t FUNC_GetSkillName = 0x4B7E60;  // char*(int id) — "skill_name" table (line 220)

    constexpr uintptr_t FUNC_GetDataTableManager = 0x1B5300;  // (line 223)
    constexpr uintptr_t FUNC_DataTableLookup     = 0x1B5920;  // (line 224)
}

// === Vista / Subtitle system RVAs ===
namespace Vista {  // (line 228)
    constexpr uintptr_t DAT_VistaSingleton  = 0xF205E8;  // ptr to Vista cinematic manager  (line 230)

    // Subtitle loader (0xa8-byte object created on first LoadSubtitle call).
    // Contains loaded schedule data, cue vector, and playback state.
    // Per-frame update (FUN_1401b66c0) advances cues based on time.
    constexpr uintptr_t DAT_SubtitleLoader  = 0xF205D0;  // ptr to loader object  (line 235)

    namespace Loader {  // (line 238)
        constexpr uintptr_t STATE        = 0x4C;  // int32: 1=loaded, 2=playing, 5=ending  (line 239)
        constexpr uintptr_t TIME         = 0x58;  // int32: current playback time (ms)      (line 240)
        constexpr uintptr_t PREV_TIME    = 0x5C;  // int32: previous frame time (ms)        (line 241)
        constexpr uintptr_t NAME         = 0x68;  // ptr: subtitle schedule name (char*)    (line 242)
        constexpr uintptr_t VECTOR_BEGIN = 0x80;  // ptr: cue vector begin                  (line 243)
        constexpr uintptr_t VECTOR_END   = 0x88;  // ptr: cue vector end (write position)   (line 244)
        constexpr uintptr_t CURSOR       = 0x98;  // ptr: current cue cursor (into vector)  (line 245)
        constexpr uintptr_t DONE         = 0xA0;  // byte: done flag                        (line 246)
    }

    // Cue data layout (3 pointer dereferences: vector -> cueStruct -> rowData):
    //   vector entry (8 bytes) -> cueStruct ptr (16-byte struct)
    //   cueStruct[0] = row data ptr -> column data
    //   Row layout: +0x00 id, +0x04 StartTime(ms), +0x08 EndTime(ms), +0x0C SubtitleTextID
    constexpr uintptr_t CUE_COL_STARTTIME = 0x04;  // (line 258)
    constexpr uintptr_t CUE_COL_ENDTIME   = 0x08;  // (line 259)
    constexpr uintptr_t CUE_COL_TEXTID    = 0x0C;  // (line 260)
}

// === Game context RVAs ===
namespace Context {  // (line 264)
    constexpr uintptr_t DAT_GameContext       = 0xF205C0;  // ptr to GameContext singleton   (line 265)
    constexpr uintptr_t FUNC_GetGameContext   = 0x1A70D0;  // returns GameContext*           (line 266)
    constexpr uintptr_t STORY_MODE_OFFSET     = 0x90;      // ptr at GameContext+0x90        (line 267)
    constexpr uintptr_t STORY_MODE_CHECK      = 0x08;      // int at storyMode+8 (0=CS, else=HM) (line 268)
    constexpr uintptr_t DIGIMON_CS_OFFSET     = 0x10;      // data ptr for Cyber Sleuth      (line 269)
    constexpr uintptr_t DIGIMON_HM_OFFSET     = 0x60;      // data ptr for Hacker's Memory   (line 270)
    constexpr uintptr_t MENU_ITEMS_VECTOR     = 0x20;      // std::vector<char*> at data+0x20(line 271)
}

} // namespace Offsets
