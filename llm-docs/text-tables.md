# MBE Text Table Reference

All text tables use columns: `ID, Japanese, English, Chinese, EnglishCensored, Korean, German`.
Message tables add a `Speaker` column.

Looked up at runtime via: `LookupText(manager, "table_name", rowId, language)` (RVA 0x1b9260).
TextCapture hooks this function to capture all text the game displays.

## Core UI / Menu Text

| Table | Key IDs | Contents |
|---|---|---|
| `main_menu` | 1-8 | Organize, Items, Status, Options, Save/Load, Sort Digimon, Farm, Exit |
| `menu_text` | 1-4 | Wallet, Organize, Settings, Party Memory |
| `common_message` | 2100+ IDs | YES/NO buttons, stat labels, evolution requirements, option labels, difficulty, rank labels |
| `yes_no_message` | varies | Dialog prompts (the question above Yes/No buttons) |
| `info_message` | varies | Field overlay notifications, tutorial triggers |
| `multi_select_text` | varies | 3+ option dialog choices (elevators, party picks, medicine) |
| `scenario_select` | 1-3 | Campaign selection prompt and descriptions |
| `shop_text` | 1-3, 1000+ | Buy/Sell/Exit, shop dialog text |
| `tutorial_title` | 1-70 | Tutorial panel topic names |
| `story_progress` | varies | Chapter names |
| `help_message` | 1000+ | Helper character text |

## Battle Text

| Table | Key IDs | Contents |
|---|---|---|
| `skill_name` | 1-7 | Attack, Guard, Escape, Change, Strategy, Auto (battle commands) |
| `skill_name` | 20-22 | Status effect messages: Paralyzed, Asleep, Immobilized |
| `skill_name` | 1001+ | Digimon-specific skill names |
| `skill_content_name` | varies | Skill descriptions with element/power/target |
| `skill_target_name` | varies | None, Self, Single Ally, All Foes, etc. |
| `support_skill_name` | varies | Passive support skill names |
| `support_skill_content_name` | varies | Support skill effect descriptions |
| `battle_info_message` | 1-3 | Critical Hit, Cannot learn, No sub-members |

## Items & Equipment

| Table | Key IDs | Contents |
|---|---|---|
| `item_name` | 1-19+ | HP Capsule, SP Capsule, Medical Spray, recovery items |
| `item_explanation` | matches item_name | Item descriptions |
| `equip_name` | 301+ | HP/SP/ATK/DEF/INT/SPD Attach E through A |
| `equip_explanation` | matches equip_name | Equipment descriptions |

## Digimon / World Data

| Table | Contents |
|---|---|
| `charname` | NPC/character names by ID |
| `digimon_type` | 0=Free, 1=Virus, 2=Vaccine, 3=Data |
| `generation` | 1=Training I, 2=Training II, 3=Rookie, 4=Champion, ... |
| `element` | Fire, Water, Plant, Electric, Earth, Wind, Light, Dark, Neutral |
| `personality` | Durable, Lively, Fighter, Defender, Brainy, Nimble, Builder, Searcher |
| `digimon_book_explanation` | Digimon encyclopedia lore |
| `fieldname` | Field/area internal names |
| `map_select_town_name` | 1=Nakano, 2=Central Hospital, 3=Shinjuku, ... |

## Farm / DigiBank

| Table | Contents |
|---|---|
| `dig_farm` | Farm, Reserve, Party labels |
| `digifarm_text` | Farm menu: Command, Change Leader, Farm Goods, Expansion |
| `digifarm_food_text` | CAM UP, stat boost food names |
| `digiline_text` | DigiLine messages and farm Digimon chat |

## Other

| Table | Contents |
|---|---|
| `colosseum_text` | Ranking/Local Battle, Opponent Search |
| `quest_text` | Quest titles and descriptions |
| `keyword_name` / `keyword_explanation` | Investigation keywords |
| `medal_name` | Digimon medal names |
| `hacker_rank` | CS ranks (1-20), HM ranks (101-120) |
| `subtitle_text` | All cutscene subtitle text |
| `bbs_post_text` | BBS bulletin board content |
| `hacking_skill_name` / `explanation` | Field hacking skills |
| `bgm` / `custom_sound_bgm` | Music track names |

## Per-Handler Table Needs

| Handler | Primary Tables |
|---|---|
| TitleHandler | None (pre-baked textures) |
| MainMenuHandler | `main_menu` |
| YesNoHandler | `common_message`, `yes_no_message` |
| ScenarioSelectHandler | `scenario_select` |
| SubtitleHandler | `subtitle_text` |
| CUiTalkWindow (next) | Message files (`m##_*.mbe`) via TextCapture |
| CUiMultiSelectWindow (next) | `multi_select_text` |
| Battle handlers | `skill_name`, `battle_info_message` |
| CUiStatusMenu | `charname`, `digimon_type`, `generation`, `element`, `skill_name`, `support_skill_name` |
| CUiItemMenu | `item_name`, `item_explanation` |
| CUiShopMenu | `shop_text`, `item_name` |
| CUiSaveload | `common_message` |
| CUiOption | `common_message`, `menu_text` |
