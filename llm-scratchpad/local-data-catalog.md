# Local Data Catalog — Digimon Story Cyber Sleuth Research Repo

**Repo location:** `C:\Users\Amethyst\projects\digimon story cyber slooth\`
**Plugin repo:** `C:\Users\Amethyst\projects\cyber-sleuth-accessibility\`

This catalog documents all locally available reference material so future sessions know what exists without re-searching.

---

## 1. Text Tables (`extracted/csv_text/`) — 61 files

All text tables are MBE archives. Each is a directory (e.g. `main_menu.mbe/`) containing one or more CSV files. The primary data file is usually `000_Sheet1.csv` or `000_para.csv`. All have the column schema: `ID, Japanese, English, Chinese, EnglishCensored, Korean, German`.

These are the tables captured by the TextCapture hook (LookupText, RVA 0x1B9260). When implementing handlers, use these files to understand what IDs to expect and what text they contain.

### 1a. Core UI / Menu Text

| File | Contents |
|---|---|
| `main_menu.mbe` | IDs 1–8: Organize, Items, Status, Options, Save/Load, Sort Digimon, Farm, Exit |
| `menu_text.mbe` | General UI labels: Wallet, Organize, Settings, Party Memory, SS stat prefix |
| `common_message.mbe` | Large shared UI text — YES/NO buttons (IDs 3/4), Party/Farm/DigiBank labels, evolution requirement strings, stat abbreviations, slot labels. ~2100+ IDs |
| `yes_no_message.mbe` | Yes/No dialog prompts — the question text shown above the Yes/No buttons. ID 1="Access the Digital World?", ID 2="Logout?", ID 100="Play as male?", ID 1000="Are you sure you want to overwrite?" etc. |
| `info_message.mbe` | Info overlay messages — displayed without requiring player input. ID 1="You got a Free Pass", ID 2="Field Tutorial 1", ID 3–5 = tutorial text. |
| `multi_select_text.mbe` | Text for CUiMultiSelectWindow (3+ option dialogs). ID 10="Select destination floor", IDs 11–13=floor options (K-Cafe, Restaurants, CD Shop) |
| `scenario_select.mbe` | Campaign selection prompt and descriptions. ID 1=prompt text, ID 2=HM description, ID 3=CS description |
| `shop_text.mbe` | Shop menu labels: ID 1=Buy, ID 2=Sell, ID 3=Exit |
| `tutorial_title.mbe` | Tutorial window titles — e.g. "Battle: Stats and Timeline", "Battle: Battle Commands" |
| `help_message.mbe` | Help text (Kyoko/Mirei helper). Contains debug test string at ID 1. Actual help starts around ID 1000 |
| `story_progress.mbe` | Chapter names — Prologue: EDEN, Chapter 1: The Kuremi Detective Agency, etc. |

### 1b. Battle Text

| File | Contents |
|---|---|
| `skill_name.mbe` | Battle command names: ID 1=Attack, ID 3=Guard, ID 4=Escape, ID 5=Change, ID 6=Strategy, ID 7=Auto; also status effect messages ("Can't escape!", "Paralyzed!", "Asleep!") |
| `skill_content_name.mbe` | Skill effect category labels (mostly empty in rows 1–3, content unknown) |
| `skill_target_name.mbe` | Skill targeting labels: None, Self, Single Ally, etc. |
| `support_skill_name.mbe` | Passive/support skill names — Dragon's Wrath, Ironclad Defense, Adroit Wisdom, etc. |
| `support_skill_content_name.mbe` | Support skill effect descriptions — "Increases ATK by 15%", "Increases DEF by 15%", etc. |
| `battle_info_message.mbe` | In-battle info messages: ID 1="Critical Hit", ID 2="Cannot learn this skill", ID 3="There are no sub-members" |

### 1c. Items and Equipment

| File | Contents |
|---|---|
| `item_name.mbe` | All item names. IDs 1–19: HP Capsule C/B/A, HP Spray C/B/A, SP Capsule C/B/A, SP Spray B/A, Medical Spray, Medical Spray DX, Poison/Panic/Paralysis/Sleep/Stun/Sprite Recovery |
| `item_explanation.mbe` | Item descriptions. ID 1="Restores 250 HP for one ally", etc. |
| `equip_name.mbe` | Equipment names — HP Attach E/D/C, etc. (IDs start at 301) |
| `equip_explanation.mbe` | Equipment descriptions — "Max HP +50 / Equipment", etc. |

### 1d. Digimon / Game World Text

| File | Contents |
|---|---|
| `charname.mbe` | Digimon names by ID — ID 1009=Hagurumon, 1010=Meramon, 1012=Garurumon, etc. These are NPC/story Digimon IDs, not the full roster |
| `digimon_type.mbe` | Type labels: ID 0=Free, ID 1=Virus, ID 2=Vaccine, ID 3=Data |
| `digimon_book_explanation.mbe` | Digimon encyclopedia entries (flavor text). IDs correspond to digimon roster |
| `generation.mbe` | Evolution stage labels: ID 1=Training I (Baby), ID 2=Training II, ID 3=Rookie, ID 4 onward=Champion/Ultimate/Mega |
| `personality.mbe` | Personality type labels: ID 1=Durable, ID 2=Lively, etc. |
| `element.mbe` | Attribute labels (Fire, Water, etc.) — IDs not confirmed but standard attribute ordering |
| `evolution.mbe` | Evolution milestone text (post-digivolution messages, e.g. "Case solved: Facing Lust") |

### 1e. World / Location Text

| File | Contents |
|---|---|
| `fieldname.mbe` | Field/area names — IDs 1000+: EDEN Entrance, EDEN Community Area, EDEN Free Area, etc. |
| `map_select_town_name.mbe` | Town/district names for map select — ID 1=Nakano, ID 2=Central Hospital, ID 3=Shinjuku, etc. |
| `eden_text.mbe` | EDEN area text (000_para.csv) — ID 0=Logout, ID 1=EDEN Entrance, ID 2=EDEN Community Area |
| `eden_group_text.mbe` | EDEN faction/group names (000_para.csv) — ID 0=Logout, ID 1=EDEN, ID 2=Kamishiro |

### 1f. Hacker's Memory Specific Text

| File | Contents |
|---|---|
| `hacker_rank.mbe` | Hacker rank names — ID 0=blank, ID 1=Baby Cyber Sleuth, ID 2=Starter Cyber Sleuth |
| `hackers_battle_mission.mbe` | HM battle mission objectives |
| `hacking_skill_name.mbe` | Hacking skill names — Wall Crack (Lv. 1/2/3), etc. |
| `hacking_skill_explanation.mbe` | Hacking skill descriptions — "Releases Lv. 1 Firewall (Yellow)" |
| `hacking_skill_condition_exp.mbe` | Hacking skill unlock conditions — "There is at least one Digimon", etc. |

### 1g. Digiline / Social System

| File | Contents |
|---|---|
| `digiline_text.mbe` | DigiLine message text (in-game social media) — message content not dialog |
| `digiline_text_add.mbe` | Additional DigiLine message text (HM content) |
| `bbs_post_text.mbe` | BBS bulletin board post text — dates like "05/01/20XX", "05/04/20XX" |
| `keyword_name.mbe` | Keyword item names — ID 100="Galacta Park, Kowloon", ID 120=Zaxon, ID 121="Account Raiding" |
| `keyword_explanation.mbe` | Keyword item descriptions |
| `keyword_get_explanation.mbe` | Text shown when obtaining a keyword |

### 1h. Farm / DigiBank / DigiLab

| File | Contents |
|---|---|
| `dig_farm.mbe` | Farm UI labels: ID 1=Farm, ID 2=Reserve, ID 3=Party |
| `digifarm_text.mbe` | DigiLab/DigiBank area text: ID 1=Farm Island, ID 2=Add Islands, ID 10=Command |
| `digifarm_food_text.mbe` | Digimon food/stat boost labels: ID 1=CAM UP, ID 2=MAJOR CAM UP, ID 3=BIG CAM UP |

### 1i. Colosseum

| File | Contents |
|---|---|
| `colosseum_text.mbe` | Colosseum mode labels: ID 1=Ranking Battle, ID 2=Local Battle, ID 3=Opponent Search |
| `colosseum_event_battle_mission.mbe` | Colosseum event mission objectives: ID 0="[n0] points", ID 1="[n0] points in [n1] turns", ID 2="Wipe out enemy team" |
| `colosseum_event_battle_rule.mbe` | Colosseum event rule text |
| `colosseum_free_rule_comment.mbe` | Colosseum free battle rule comments |
| `colosseum_item_name.mbe` | Colosseum costume/item names — ID 1="C. Sleuth Male", ID 2="C. Sleuth Female", etc. |
| `colosseum_item_explanation.mbe` | Colosseum item descriptions |
| `tournament_name.mbe` | Tournament names (000_event.csv) — ID 1="Omni New Year Contest", ID 2="Champion Rivals Contest" |

### 1j. Quest Text

| File | Contents |
|---|---|
| `quest_text.mbe` | Quest names and descriptions — ID 20="Broadway on Fire!" |
| `quest_text_add.mbe` | Additional quest text (HM content) |

### 1k. Audio / Customization

| File | Contents |
|---|---|
| `bgm.mbe` | BGM track names — ID 1=Normal Battle (Cyber Sleuth), ID 2=Normal Battle (Hacker's Memory), ID 3=Domination Battle |
| `custom_sound_bgm.mbe` | Custom sound selection BGM names — ID 1=Default, ID 2=Butterfly, ID 3=Brave Heart |
| `custom_sound_scene.mbe` | Custom sound scene context names — ID 1=DigiLab, ID 2=DigiFarm, ID 3=Digivolution |

### 1l. Subtitle Text

| File | Contents |
|---|---|
| `subtitle_text.mbe` | ALL cutscene subtitle text. Columns: ID, Japanese, English, Chinese, EnglishCensored, Korean, German. ID 301–701+ are early cutscene lines. Example: ID 401="Aaah! What, what are you!?" (opening cutscene). This is what `LookupText("subtitle_text", id, language)` returns. The schedule (timing) is in the subtitle_*.mbe data tables, not here. |

### 1m. Mirror Dungeon / Other

| File | Contents |
|---|---|
| `mirror_dungeon_text.mbe` | Mirror Dungeon floor/area names (000_para.csv) — ID 1=Avalon Server, ID 2=Makoto Yamashina's Memory, ID 3=Akihabara: Digital Shift |
| `medal_name.mbe` | Medal item names — IDs 1001=Armageddemon, 1002=Icemon, 1003=Aquilamon (Digimon medal names) |

---

## 2. Message Tables (`extracted/csv_message/`) — 1,451 files

These are interactive dialog/event message tables. Each file is an MBE directory containing `000_Sheet1.csv`. The column schema adds a `Speaker` field: `ID, Speaker, Japanese, English, Chinese, EnglishCensored, Korean, German`.

Speaker values are NPC/character IDs (e.g., 3012 = a specific NPC, 2000 = narrator/silent). Text IDs within each file are locally scoped (no global uniqueness).

### Naming Patterns and Categories

**Main story dialog — `m##_xxx_####.mbe`**
Format: `m[chapter]_[area]_[event_id].mbe`
- `m00_*` through `m21_*` = Cyber Sleuth main story (chapters 0–21)
- `m51_*` through `m68_*` = Hacker's Memory main story
- Sub-prefixes:
  - `_t##_` = town area dialog (e.g., `m00_t01_1501.mbe` = chapter 0, town 01, event 1501)
  - `_d##_` = dungeon/digital world area dialog
  - `_e00_` = opening/ending cutscene dialog
- Example: `m00_t01_1501.mbe` contains the opening street scene dialog; Speaker 4250/4251/4252 are bystanders, Speaker 2000 is narrator

**Side story dialog — `s###_xxx_####.mbe`**
- `s103_*`, `s110_*`, `s113_*` = DLC story content
- `s501_*` through `s564_*` = Hacker's Memory side cases
- `s601_*` through `s630_*` = Cyber Sleuth relationship events (30 files, `_e00` suffix)
- `s701_*` through `s715_*` = Hacker's Memory relationship events (15 files, `_e00` suffix)

**Town/field NPC dialog — `t####[_add].mbe`**
- `t0101.mbe` through `t5006.mbe` = overworld NPC and shop dialog, town-by-town
- `_add` suffix = Hacker's Memory additions to the same area

**Dungeon/field NPC dialog — `d####[_add].mbe`**
- `d0101.mbe` through `d2003.mbe` = digital world / dungeon NPC dialog
- `d9001_*` through `d9402_*` = post-game/extra dungeon dialog

**Battle-related messages**
- `battle_1020.mbe`, `battle_1021.mbe` = battle event dialog (story battles)
- `battle_2500.mbe`, `battle_4028.mbe` = more battle event dialogs
- `battle_colosseum.mbe` = colosseum battle dialog

**Quest messages**
- `quest_000.mbe` through `quest_*` = individual quest dialog (numbered by quest ID)
- `quest_complete.mbe` = quest completion messages
- `quest_hucker.mbe` = HM-specific quest dialog
- `quest_hucker_event.mbe` = HM quest event dialog
- `quest_iteme.mbe` = quest item dialog

**System/UI messages**
- `colosseum_message.mbe` = colosseum system messages
- `field_text.mbe` / `field_text_add.mbe` = field sign/notice text (text that appears in the world)
- `s_common_message.mbe` = shared system messages (HM campaign)
- `after_evt.mbe` = post-event dialog
- `broken_nabit.mbe` = "Nabit" NPC dialog
- `data_plate_message.mbe` / `data_plate_message_add.mbe` = data plate item dialog
- `emblem_quest.mbe` = emblem quest dialog
- `v_uchikda_message.mbe` = Uchida character messages
- `kyoko_help.mbe` / `mirei_help.mbe` / `mirei_help_add.mbe` = helper character tutorials

**Keyword NPC dialog — `keyword_*_npc.mbe`**
- ~25 files, one per area with keyword-triggered NPC conversations
- Pattern: `keyword_[area_code]_npc_####.mbe`

**HM-specific quest/mission messages**
- `hm_quest_hucker_event.mbe`, `hm_quest_iteme.mbe`, `hm_quest_message.mbe`

---

## 3. Decompiled Scripts (`extracted/decompiled_scripts/`) — 2,623 files

All files are `.txt` containing decompiled Squirrel VM bytecode. Every script begins with the same large block of utility function definitions (from `function_common.txt` / `include.txt`) because the decompiler inlines include chains. The actual unique event logic starts after those boilerplate functions.

### Key Library Scripts (not game events — library/utility)

| File | Description |
|---|---|
| `function_common.txt` (714 lines) | Core dialog wrapper functions: `Message()`, `MessageTalk()`, `MessageSel()`, `YesNoInfo()`, `YesNoSystem()`, `WaitMessage()`, `TalkCommunication()`, `ShowCommunication()`, `OpenTutorial()`, etc. Essential reference for understanding all dialog call patterns |
| `function_field.txt` (1616 lines) | Field-specific helpers: `EVENT_START_SET()`, `EVENT_END_SET()`, `YES_NO_WARP()`, `YES_NO_CONNECT_JUMP()` — shows the standard pattern for warping with yes/no confirmation |
| `include.txt` (2355 lines) | Master include — combines function_common + all Wait* helpers + all window/flag helpers. Every event script starts with these boilerplate definitions |
| `include_battle.txt` | Battle-specific include helpers |
| `include_hackersbattle.txt` | HM battle include helpers |
| `include_love_event.txt` | Relationship event include helpers |
| `define.txt` | Global constants (file appears empty/not decompiled — no output) |
| `function_free_quest.txt` | Free quest helper functions |
| `function_hm_field.txt` | HM-specific field helpers |

### Script Categories by Prefix

**Main story events — `m##_*` (many files)**
- Named to match the message tables: `m00_t01_1501.txt` is the Squirrel script that fires for message table `m00_t01_1501.mbe`
- These scripts orchestrate cutscenes: load models, trigger Vista cinematics, load subtitles, play animations, display dialog, check flags, branch story, warp player
- Example excerpt from `m00_t01_1501.txt`:
  ```squirrel
  this.Vista.LoadSubtitle("subtitle_m00_t01_...")
  this.Vista.PlaySubtitle()
  this.Talk.Load("m00_t01_1501")
  this.Talk.LoadModel(1, "npc015", 1)
  ...
  this.MessageTalk(1)  // shows line 1 from m00_t01_1501.mbe
  ```
- Chapter range: m00–m21 (CS), m51–m68 (HM), ~400+ files

**Side story events — `s###_*` (many files)**
- Same structure as main story, for side cases and relationship events
- `s601_e00` through `s630_e00` = CS character events; `s701–s715` = HM character events

**Town/overworld NPC scripts — `t####[_add].txt`**
- Field NPC event scripts — typically short (50–200 lines)
- Control NPC dialog, item pickups, trigger battles, handle shop interactions

**Dungeon/area scripts — `d####[_add].txt`**
- Digital world and dungeon event scripts
- Handle enemies, obstacles, boss battles, environmental interactions
- Numbered dungeon areas: d01xx=EDEN, d02xx=Shibuya, d03xx=Shinjuku, d04xx=Akihabara, d05xx=Ueno, etc.

**Battle event scripts — `battle_####.txt`**
- `battle_0000.txt` (917 lines) — battle utility functions: `BattleGetLowHp()`, `BattleGetLowSp()`, `BattleGetHighHp()`, `BattleAliveCharacterCount()`, `BattleIsNormalMode()`. Essential reference for reading HP/SP.
- `battle_0001.txt` — more battle helpers
- `battle_0011.txt`, `battle_0511–0517.txt` — specific encounter scripts
- `battle_1000–1054.txt` — main story battle scripts (boss fights)
- `battle_1500–1519.txt` — special/story battles
- `battle_2000–2016.txt` — more battle events

**HM-specific battle — `hackersbattle_####.txt`**
- `hackersbattle_0000–0065.txt` = HM hacker battle event scripts
- `hackersbattle_quest_event_00–09.txt` = HM quest battle events

**Quest scripts — `quest_hucker_01–05.txt`**
- Hacker's Memory main quest event scripts

**System/utility scripts**
- `internet.txt` — EDEN / digital internet area event logic
- `gasha.txt` / `gasha_add.txt` — gacha/medal system scripts
- `kyoko_help.txt` / `mirei_help.txt` — helper character tutorial scripts
- `npc_text.txt` — NPC generic text script
- `debug.txt` / `debug_hm.txt` / `debug_quest.txt` — developer debug scripts
- `test.txt` / `test_keyword.txt` / `test_talk.txt` / `test_vista.txt` / `scene_test.txt` / `script_test.txt` / `motion_test.txt` — developer test scripts (useful for understanding API usage)
- `takihara_test.txt` — NPC-specific test script

### Key Squirrel API Patterns (from script analysis)

```squirrel
// Dialog
this.Talk.Load("message_table_name")
this.Talk.Message(id)          // plain text box
this.Talk.MessageTalk(id)      // with speaker portrait
this.Talk.MessageSel(count, id) // choice menu
this.Talk.ResultSelectedNum()  // returns 1-based selection
this.WaitMessage()

// Cutscene
this.Vista.Start()
this.Vista.LoadSubtitle("subtitle_m##_...")
this.Vista.PlaySubtitle()
this.Vista.LoadCameraAnimation("name")
this.Vista.LoadLocater("name")
this.Vista.Play()

// Window (Yes/No, etc.)
this.Window.OpenYesNoInfo(textId)
this.Window.GetResultYesNoInfo()   // 1=YES, 2=NO, 3=CANCEL
this.Window.OpenYesNoSystem(textId)

// Field
this.Field.ChangeField(mapName, area, locator, angle)
this.Field.MoveOffPlayer() / MoveOnPlayer()
this.Field.SetPlayerVisible(bool)

// Battle
this.Battle.GetParameter(chr, stat)  // stat: 0=HP, 1=SP, 2=MaxHP, 3=MaxSP, 4-8=ATK/DEF/INT/SPD/ABI

// Flags
this.Flag.Set(n)
this.Flag.Get(n)
```

---

## 4. Ghidra Analysis Outputs

### In-repo Ghidra folder (empty — placeholder)
`C:\Users\Amethyst\projects\digimon story cyber slooth\ghidra\output\` — **empty**
`C:\Users\Amethyst\projects\digimon story cyber slooth\ghidra\scripts\` — **empty**

### Ghidra work directory: `C:\ghidra_work\dscs\`
This directory is outside the repo but contains all actual analysis outputs. Access was denied during catalog generation, but per CLAUDE.md the following files exist there:

| File | Contents |
|---|---|
| `cui_vtables.txt` | All 128 CUi* vtable addresses extracted from RTTI scan |
| `subtitle_player_full.txt` | Full Ghidra decompilation of the subtitle player system |
| `data_table_api.txt` | Ghidra analysis of text/data table lookup API |
| `clean_scripts/ExtractCUiVtables.java` | Ghidra headless script that produced cui_vtables.txt |
| `clean_scripts/FindTextPipeline.java` | Ghidra headless script for text pipeline analysis |

### Compiled RE Results (in plugin repo)
The most important Ghidra findings are already encoded in the plugin:

**`C:\Users\Amethyst\projects\cyber-sleuth-accessibility\include\offsets.h`** — Complete reference:
- All 128 CUi* vtable RVAs
- Tick function RVAs for hooked classes
- Member offset namespaces for all implemented handlers
- Text system RVA (LookupText, GetTextTableManager, language singleton)
- Vista/subtitle system RVAs and cue data layout
- Game context RVAs

---

## 5. Game Data Tables (`extracted/csv/`) — 214 files

These are non-text data CSVs (numeric parameters, AI tables, spawn tables, etc.). Each is an MBE directory with one or more CSVs. Column names vary by table; Japanese column names are partially preserved.

### Digimon Core Data

| File | Contents |
|---|---|
| `digimon_list.mbe` (`000_digimon.csv`) | Digimon roster — columns: id, then numeric IDs. Row IDs match charname.mbe. Starts at ID 2. |
| `mon_para.mbe` (`000_Monster.csv`) | Monster battle parameters — columns: type, variation, baseHP, baseATK, baseDEF, baseINT, baseSPD, level growth rates, resistances, EXP/YEN drop, item drops, scan rate. Covers all 214+ Digimon |
| `mon_para_hard.mbe` | Hard mode monster parameters (same schema as mon_para) |
| `digimon_common_para.mbe` | Common Digimon parameters (per-Digimon global settings) |
| `digimon_farm_para.mbe` | Farm-specific Digimon parameters |
| `digimon_market_para.mbe` | DigiBank market/trade parameters |
| `mon_cpl.mbe` | Digimon completion/scan parameters |
| `mon_design_para.mbe` | Digimon model/visual design parameters |
| `model_attach_para.mbe` | Model attachment parameters |
| `model_default_effect.mbe` | Default visual effects per Digimon model |
| `model_default_scale.mbe` | Default model scale parameters |
| `model_position_offset.mbe` | Model position offsets |

### Evolution / Digivolution Data

| File | Contents |
|---|---|
| `evolution_condition_para.mbe` (`000_digimon.csv`) | Digivolution requirements per Digimon — columns: id, up to 10 condition slots each with condType/condValue/condUnk. CondTypes map to: 1=Level, 2=stat threshold, 4=ABI threshold, 5=CAM threshold |
| `evolution_direction_para.mbe` | Which Digimon can evolve into which (digivolution paths) |
| `evolution_next_para.mbe` | Next evolution targets |
| `degeneration_para.mbe` | De-digivolution parameters |
| `experience_table.mbe` | XP thresholds per level |
| `lvup_para.mbe` | Level-up stat growth parameters |

### Item / Equipment Data

| File | Contents |
|---|---|
| `item_para.mbe` (`000_table.csv`) | Item parameters — columns: id, nameId, itemType, sortValue, buyPrice, sellPrice, inBattle, inOverworld, effectType, effectValue, farmEffects, medalId, medalRarity, medalPrice, skin flags, descriptionId, iconId |
| `equip_para.mbe` | Equipment parameters |

### Battle System Data

| File | Contents |
|---|---|
| `battle_ai.mbe` | Enemy AI behavior scripts (action weights/patterns) |
| `battle_bgm.mbe` | Battle BGM selection parameters |
| `battle_camera.mbe` | Battle camera parameters |
| `battle_command.mbe` | Battle command definitions |
| `battle_command_effect.mbe` | Battle command visual effects |
| `battle_effect.mbe` | Battle visual effect definitions |
| `battle_field.mbe` / `battle_field_add.mbe` | Battle field/arena parameters |
| `battle_se.mbe` | Battle sound effect parameters |
| `battle_support_skill.mbe` | Support skill battle parameters |
| `battle_voice.mbe` / `battle_voice_add.mbe` | Battle voice clip assignments |

### Hacking System Data

| File | Contents |
|---|---|
| `hacking_skill_para.mbe` | Hacking skill parameters (unlock levels, effects) |
| `hacking_skill_conditions.mbe` | Hacking skill unlock conditions |
| `learn_hacking_skill.mbe` | When/how hacking skills are learned |
| `digital_space_para.mbe` | Digital space (dungeon) parameters |

### Field / Map Data

| File | Contents |
|---|---|
| `field_area_para.mbe` / `field_area_para_add.mbe` | Field area parameters |
| `field_ap_para.mbe` / `field_ap_para_add.mbe` | Access point parameters |
| `field_cp_para.mbe` / `field_cp_para_add.mbe` | Control point parameters |
| `field_ef_para.mbe` / `field_ef_para_add.mbe` | Field event flag parameters |
| `field_gk_para.mbe` / `field_gk_para_add.mbe` | Field gate/key parameters |
| `field_npc_para.mbe` / `field_npc_para_add.mbe` | NPC placement/behavior parameters |
| `field_se_para.mbe` | Field sound effect parameters |
| `field_tr_para.mbe` / `field_tr_para_add.mbe` | Field trigger/trap parameters |
| `field_common_param.mbe` | Field system global parameters |
| `field_visualizer_para_add.mbe` | HM field visual additions |
| `map_encount_param.mbe` / `map_encount_param_add.mbe` | Random encounter parameters |
| `map_select.mbe` / `map_select_add.mbe` | Map selection parameters |
| `selectmapparam.mbe` | Select map system parameters |
| `tp_table.mbe` | Teleport destination table |
| `talk_position.mbe` | NPC talk position offsets |

### Farm / DigiBank Data

| File | Contents |
|---|---|
| `farm_constant.mbe` | Farm system constants |
| `farm_development.mbe` | Farm island development parameters |
| `farm_give_food.mbe` | Farm feeding mechanic parameters |
| `farm_goods.mbe` | Farm goods/items available |
| `farm_investigation.mbe` / `farm_investigation_add.mbe` | Farm investigation missions |
| `farm_talk.mbe` | Farm NPC dialog parameters |
| `farm_training.mbe` | Farm training activity parameters |

### Quest / Keyword Data

| File | Contents |
|---|---|
| `quest_para.mbe` / `quest_para_add.mbe` | Quest parameters (requirements, rewards, flags) |
| `keyword_para.mbe` / `keyword_para_add.mbe` | Keyword item parameters |
| `keyword_message_para.mbe` / `keyword_message_para_add.mbe` | Keyword message trigger parameters |
| `keyword_npc_para.mbe` / `keyword_npc_para_add.mbe` | Keyword NPC response parameters |

### Shop / Economy Data

| File | Contents |
|---|---|
| `shop_para.mbe` (`000_shop.csv`, `001_lineup.csv`, `002_limit_lineup.csv`) | Shop definitions, item lineups, and time-limited lineups |
| `digimon_market_para.mbe` | DigiBank market parameters |

### Colosseum / Network Data

| File | Contents |
|---|---|
| `colosseum_event_battle.mbe` | Colosseum event battle definitions |
| `colosseum_item.mbe` | Colosseum item/costume definitions |
| `hackers_battle_battle.mbe` | HM hacker battle definitions |
| `hackers_battle_result.mbe` | HM battle result parameters |
| `hackers_memory_para.mbe` | HM system global parameters |
| `network_dungeon_offset.mbe` | Network dungeon parameters |

### UI / System Data

| File | Contents |
|---|---|
| `multi_select_para.mbe` / `multi_select_para_add.mbe` | CUiMultiSelectWindow configuration (which options appear, their text IDs) |
| `yes_no_window_para.mbe` | CUiYesNoWindow configuration (which yes_no_message ID to show per context) |
| `ui_mon_param_info.mbe` | Digimon status screen parameter display configuration |
| `tutorial_para.mbe` / `tutorial_para_add.mbe` | Tutorial step definitions |
| `help_icon.mbe` | Help icon definitions |
| `newgame_para.mbe` | New game initialization parameters |
| `ng_word.mbe` | Prohibited word filter list |
| `personality_para.mbe` | Personality system parameters |
| `skill_use_group_set.mbe` | Skill usage group definitions |

### Subtitle Schedule Data
All subtitle timing/schedule tables are in `csv/` (not `csv_text/`). Naming: `subtitle_[scene_id].mbe`.

- ~90 subtitle schedule files covering main story + HM cutscenes
- Pattern: `subtitle_m##_xxx_####[_##].mbe` (matches the Vista.LoadSubtitle() call)
- Example: `subtitle_m00_e00_0001_01.mbe` = first cutscene, first subtitle sequence
- Column schema: `id, StartTime(ms), EndTime(ms), SubtitleTextID, VoiceAudioID`
- SubtitleTextID references `subtitle_text.mbe` in csv_text/
- Covers: all CS main story cutscenes (m00–m21), HM story (m51–m68), opening movies (s01–s21), DLC content

### Other Data

| File | Contents |
|---|---|
| `animation_loop_data.mbe` | Animation loop parameters |
| `same_animation_data.mbe` | Shared animation mappings |
| `bbs_post_para.mbe` | BBS post parameters (which posts appear when) |
| `custom_sound_bgm_para.mbe` / `custom_sound_scene_para.mbe` | Custom sound system parameters |
| `debug_event_view.mbe` | Developer debug event viewer parameters |
| `digiline_constant.mbe` / `digiline_para.mbe` / `digiline_para_add.mbe` | DigiLine social system parameters |
| `eden_text_para.mbe` / `eden_text_para_add.mbe` | EDEN billboard/sign text parameters |
| `join_digimon_para.mbe` / `join_digimon_para_add.mbe` | Party-joining Digimon parameters |
| `medal_collection_para.mbe` / `medal_gasha_para.mbe` | Medal collection and gacha parameters |
| `mirror_dungeon_para.mbe` / `mirror_dungeon_para_add.mbe` | Mirror dungeon floor parameters |
| `warp_item_disable_para.mbe` / `warp_item_disable_para_add.mbe` | Warp item restriction parameters |

---

## 6. DSDB Archive Structure (`extracted/DSDB/`, `DSDBA/`, `DSDBS/`)

The raw extracted archives (not individual CSVs). These contain the original MBE binary files alongside the extracted CSVs.

- `DSDB/` — Main data archive (data/, text/, message/, script64/ subdirs)
- `DSDBA/` — Supplemental character model data (no text tables)
- `DSDBS/` — UI images and textures (used to confirm title menu items are pre-baked textures)

---

## 7. Quick Reference: Which Tables Are Most Useful for Handlers

### For Phase 2 handlers being implemented next:

| Handler | Primary text tables needed | Notes |
|---|---|---|
| CUiTalkWindow (NPC dialog) | `m##_xxx_####.mbe` message files | TextCapture timing works; Speaker field gives NPC identity |
| CUiMultiSelectWindow | `multi_select_text.mbe`, `multi_select_para.mbe` | para tells which options exist; text has the labels |
| CUiOption / Settings | `menu_text.mbe`, `common_message.mbe` | Settings labels are in menu_text and common_message |
| CUiSaveload | `common_message.mbe` (IDs 1000-1001: "Select a save location") | Save slot data is numeric |
| CUiItemMenu (Items screen) | `item_name.mbe`, `item_explanation.mbe`, `common_message.mbe` | Item IDs via item_para.mbe |
| CUiStatusMenu (Digimon status) | `charname.mbe`, `digimon_type.mbe`, `generation.mbe`, `element.mbe`, `support_skill_name.mbe`, `skill_name.mbe` | Numeric stats from mon_para.mbe |
| Battle commands | `skill_name.mbe` (IDs 1-7 are Attack/Guard/Escape/Change/Strategy/Auto) | battle_info_message for status messages |
| Quest Board | `quest_text.mbe`, `quest_text_add.mbe` | quest_para.mbe for requirements |

### For subtitle work (already implemented):
- Timing schedule: `subtitle_*.mbe` in `csv/` — has StartTime/EndTime/SubtitleTextID
- Text content: `subtitle_text.mbe` in `csv_text/` — ID → displayed text
- SubtitleHandler polls memory directly, no CSV reading needed at runtime

---

*Generated: 2026-03-03. Based on full directory scan of `C:\Users\Amethyst\projects\digimon story cyber slooth\extracted\` and `C:\Users\Amethyst\projects\cyber-sleuth-accessibility\include\offsets.h`.*
