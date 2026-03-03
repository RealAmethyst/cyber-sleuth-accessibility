# Digimon Story Cyber Sleuth Complete Edition — Raw Game Info

Gathered from extracted game data (csv_text/, csv_message/, decompiled_scripts/) and
the project CLAUDE.md. Intended for use when designing accessibility handler coverage.

---

## Game Overview

Digimon Story Cyber Sleuth Complete Edition (Steam/PC) is a JRPG set in near-future Tokyo
and cyberspace. The player is a young hacker who gets dragged into investigating mysterious
events linking the digital world (EDEN) and the real world. The game includes both campaigns
from the original PS4/Vita releases:

- **Cyber Sleuth (CS)**: Protagonist is Aiba (male or female), a young hacker who becomes a
  "Cyber Sleuth" detective after losing their physical body. Story revolves around the corporation
  Kamishiro Enterprises, the digital world EDEN, and entities called "Eaters."
- **Hacker's Memory (HM)**: Protagonist is Keisuke Amazawa, a new member of a hacker team
  called Hudie. A separate but parallel story set in the same world.

Both campaigns share Digimon storage (DigiBank), farm, and unlocks. Save data can be transferred
between them to unlock bonuses.

---

## Two-Campaign Architecture

The game uses `this.Common.GetStoryMode()` to distinguish campaigns:
- Mode 0 = Hacker's Memory (Keisuke / Hudie)
- Mode 1 = Cyber Sleuth (Aiba / detective)

Campaign selection happens at the intro cutscene (ScenarioSelect screen), before save data is
created. The scenario select screen uses text IDs from scenario_select.mbe:
- ID 2: "Story of the novice of a certain hacker team. Another view of this world through
  hackers' eyes." (Hacker's Memory)
- ID 3: "Story of Cyber Sleuths in worlds real and digital. Jump into this world for the first
  time here." (Cyber Sleuth)

UI class names are duplicated per campaign:
- CS: `CUiMainMenu`, `uiMainmenu`
- HM: `CUiHmMainMenu`?, `uihmMainmenu`

---

## Game Screens / Menus

### 1. Title Screen (CUiTitle)
Pre-baked texture atlas items (not text table lookups). Items:
- 0: "New Game"
- 1: "Continue"
- 2: "New Game +" (locked if not cleared once)
- 3: "Exit Game"

State machine (19 states). State 12 = interactive menu. Cursor at this+0x114, item count at
this+0x124, locked flag at this+0xc4.

### 2. Scenario Select (CUiScenarioSelect)
Appears after title when starting a new game. Player chooses which campaign to play.
Items are pre-baked textures — hardcode their names:
- ID 2 = "Hacker's Memory"
- ID 3 = "Cyber Sleuth"
TextCapture detects `scenario_select` table row 1 (the prompt text) as signal that the screen
is interactive.

### 3. Main Menu (CUiMainMenu)
Opened with Start/Menu button during field exploration. 8 items maximum.
Items (main_menu.mbe, cursor+1 = ID):
- 1: Organize
- 2: Items
- 3: Status
- 4: Options
- 5: Save/Load
- 6: Sort Digimon
- 7: Farm
- 8: Exit

Cursor at this+0x27D8 (int32), state at this+0x2928 (int16: 0=closed, 3=interactive,
5=closing), item count at this+0x2A08.

Wallet (money) and Party Memory (Digimon storage used vs total) are shown in the main menu
HUD (menu_text.mbe IDs 1-4: Wallet, Organize, Settings, Party Memory).

### 4. Organize Menu (CUiOrganize)
Party management. Allows swapping Digimon between party (up to 3 active) and bench.
Triggered by "Organize" from main menu. Also accessible from DigiBank.

Party Digimon are shown with:
- Name, Level, HP/Max HP, SP/Max SP
- ATK, DEF, INT, SPD stats
- Type (Virus/Vaccine/Data/Free)
- Element (Fire/Water/Plant/Electric/Earth/Wind/Light/Dark/Neutral)
- Generation (Training I, Training II, Rookie, Champion, Ultimate, Mega, Ultra, Armor, Boss)

### 5. Items Menu (CUiItemMenu)
Item inventory. Items are categorized. Each item has name (item_name.mbe) and
explanation (item_explanation.mbe). Player can use or check items.

### 6. Status Menu (CUiStatusMenu)
Displays detailed stats for party Digimon. Stats shown:
- Lv, Max HP, Max SP, ATK, DEF, INT, SPD (from common_message.mbe IDs 3200-3206)
- ABI (Ability — accumulated stat gains through digivolution chains)
- CAM (Camaraderie — friendship percentage 0-100%)
- Personality type (Durable/Lively/Fighter/Defender/Brainy/Nimble/Builder/Searcher)
- Type (Virus/Vaccine/Data/Free)
- Element
- Generation
- Skills equipped (up to 20, split between Special and Inherited)
- Support skill

### 7. Options Menu (CUiOption / CUiSettingMenu)
Settings accessible from main menu. Options (common_message.mbe IDs 2200-2232):
- Music Volume (0%-100%)
- Effect Volume (0%-100%)
- Voice Volume (0%-100%)
- Minimap (Show/Hide)
- Partner Digimon (show companion in field — Show/Hide)
- Battle Cutscenes (Normal/Short)
- Digimon Voices (On/Off)
- Restore Default Settings
- Apply Settings
- Battle Difficulty (Normal/Hard/Very Hard)
- Show One Only (display one partner Digimon)
- Custom Sound (custom BGM)
- Begin With These Settings (first-time startup option menu)

### 8. Save/Load Menu (CUiSaveload)
Save and load game data. Features:
- Multiple save slots
- Shows play time, date of update
- Online Storage Data (upload/download)
- Prompts: "Are you sure you want to overwrite?", "Load this data?", "Delete this saved data?"
  (from yes_no_message.mbe IDs 1000-1002)

### 9. Sort Digimon Menu (CUiDigibankTopReplace / related)
Sort or rearrange Digimon in the DigiBank. Accessed from main menu.

### 10. Farm Menu (CUiDigifarm)
The DigiBank/Farm interface. Accessible from main menu or in-field Farm Islands.
Sub-menus: digifarm_text.mbe

Farm has up to 5 Farm Islands (Party, Farm Island 1-5, DigiBank).
Commands:
- Command (give a command to Digimon on the island)
- Change Leader (set which Digimon is shown as island leader)
- Farm Goods (items produced by the island)
- Farm Expansion (increase island capacity)

Farm Commands that Digimon can be assigned (digifarm_text.mbe):
- Training (increases stats; status shown as "Training + X m")
- Developing (produces Farm Goods; status "Developing + X m")
- Investigating (finds new cases/quests; status "Investigating + X m")
- Standby

Digimon on the farm can level up. DigiLine messages from them
(digiline_text.mbe) notify when training completes, when max level is reached, etc.

Farm Island food items improve different stats:
- CAM UP, MAJOR CAM UP, BIG CAM UP (Camaraderie)
- HP UP, SP UP, ATK UP, DEF UP, INT UP, SPD UP (stats)
- BIG CAM & ABI UP

### 11. DigiBank (CUiDigibankTop and sub-screens)
Central Digimon management hub. Located in DigiLab (EDEN) and accessible via main menu.
Sub-menus (CUiDigibankTopXxx):
- Convert (DigiConvert — hatch new Digimon from scan data)
- Evolution (Digivolve / De-Digivolve)
- Load (absorb a Digimon to boost stats of another)
- Replace (swap Digimon between party and bank)
- Return Data (release a Digimon back to data)

Tutorial topics (tutorial_title.mbe IDs 13-17, 62):
- Moving Digimon
- Digivolution and De-Digivolution
- Loading Digimon
- DigiConverting
- Returning to Data
- Sending Digimon (HM feature)

### 12. DigiBank Evolution Screen (CUiSelectEvolution)
Shows available Digivolution and De-Digivolution options for a selected Digimon.
Requirements listed: Level X or higher, Max HP X, ATK X, etc. (common_message.mbe IDs
1215-1233).
Yes/No confirmation prompts (yes_no_message.mbe IDs 1201-1204):
- "Digivolve to [d0]?"
- "Consume [d0] for DNA Digivolution?"
- "De-Digivolve to [d0]?"
- "Mode Change to [d0]?"

### 13. Digimon Field Guide (CUiDigimonBookIndex, CUiDigimonBookDetail)
Encyclopedia of all Digimon. Shows scan rate per species (0-100%, increments as player
fights more of that species in the field). At 100% a new Digimon can be hatched via
DigiConvert.

Each entry shows:
- Name, generation, type, element
- Description text (digimon_book_explanation.mbe)
- Scan rate
- Stats at various levels

### 14. Quest Board / Case Board (CUiQuestBoard, CUiQuestDetail)
Cases/quests taken from the agency whiteboard. Each case has:
- Title (quest_text.mbe odd IDs)
- Description (next ID)
- Client and location
- Objective
- Reward (money + CSP)

Status: Active, Reportable, Completed.

Quest result shown after turning in: "X cleared! / Reward Money: X Yen / CSP: X"
(common_message.mbe IDs 1300-1308)

### 15. Shop (CUiShopMenu, CUiShopBuyList, CUiShopSellList)
Player can buy and sell items. Shop types:
- Regular shops (shop_text.mbe IDs 1000-1021)
- DigiMarket (shop_text.mbe IDs 1057-1059) — buy Digimon scan data
- Medal Master (shop_text.mbe IDs 1067-1073) — buy/sell Digimon medals

Shop prompts: "Buy", "Sell", "Exit" (shop_text.mbe IDs 1-3).
"How many do you want to buy?", "How many do you want to sell?"

### 16. Battle System (CUi_Btl_* classes)

#### How battle works
Turn-based. Up to 3 Digimon on each side (3 player, up to 3 enemies). There is also a
"Timeline" bar showing turn order for all combatants (order determined by SPD stat).

#### Battle commands (per Digimon, each turn)
From skill_name.mbe (low IDs):
- Attack (1): use the Digimon's special skill (costs SP)
- Guard (3): reduce incoming damage
- Escape (4): attempt to flee (costs turn)
- Change (5): swap with a sub-member Digimon
- Strategy (6): set AI behavior for remaining Digimon
- Auto (7): let AI control that Digimon

Skills (1001+): Digimon-specific named skills with element, damage, target, and effects.

#### Battle info displayed
- HP and SP bars for each Digimon (current/max)
- Turn order timeline
- Enemy scan rate (how much data collected)
- Status effects when active:
  - Paralyzed (skill_name.mbe ID 20)
  - Asleep (ID 21)
  - Immobilized by recoil (ID 22)
  - Critical Hit (battle_info_message.mbe ID 1)
- Cross Combo — special combo attacks when multiple party Digimon act cooperatively

#### Battle Difficulty
Normal, Hard, Very Hard (from common_message.mbe IDs 2226-2229).
Also accessible via `this.Battle.GetDifficulty()` (0=Normal, 1=Hard, VeryHard not present
in scripts).

#### Battle Result Screen (CUiBattleResult, CUiBattleResultScan)
After winning:
- EXP gained per Digimon
- Level up notifications
- Scan data sent to DigiBank/farm
- Skill learned notification (common_message.mbe ID 1700)
- If level up: new stats shown

#### Level Up (CUiLevelUp)
Shown after battle. Stats increased. If Digimon can now Digivolve, that is flagged.

#### Choose Skills (CUiChooseHoldSkill)
If a Digimon exceeds its max skill slots from a level-up, player must choose which skill
to discard (common_message.mbe IDs 1702-1710).

#### Game Over (CUi_Btl_GameOver)
"Retry" or "Back to Title" (common_message.mbe IDs 3000-3001).

### 17. Field Exploration

#### Field HUD (CUiField_001, CUiField_002)
During field exploration the player walks through 3D environments. The minimap is shown.
Digimon encounter on contact with enemy models in the field (random encounters).

Hacking Skills used in the field (from field HUD or hacking menu):
- Wall Crack Lv.1-4: removes firewall barriers blocking paths (yellow, green, red, purple)
- Archiver: compresses/expands "spread boxes" (changes the hitbox for random encounters)
- Code Scan Lv.1-2: decrypts encrypted data / locked objects
- Copy and Paste: copies data plate attributes
- Stealth Hide: makes player transparent (reduces/eliminates random encounters)
- High Security Lv.1-3: increases random encounter rate (used to grind)
- Function Call Lv.1-2: increases encounter rate / forces encounter
- Route Access: creates/deletes floor sections
- Power Control: toggles power for switches
- Restoration: repairs broken objects
- Visualizer: reveals hidden traps
- Free Gate: returns to dungeon entrance
- Acceleration: increases movement speed

#### Chapter Display (CUiFieldChapter)
Chapter name shown when entering a new story chapter.

#### Town Map / Map Select (CUiSelectMap, CUiMapSelect)
Map of Tokyo real-world areas:
- Nakano (protagonist's base: Kuremi Detective Agency at Broadway 1F, K-Cafe 4F)
- Central Hospital
- Shinjuku (Shinjuku Station Square, Downtown)
- Shibuya (Scramble Intersection, Tower Records)
- Akihabara (Electric Town)
- Asakusa
- Ueno (Museum Park)
- Odaiba Bay
- Toyosu Condos
- Tokyo Metropolitan Office
- Nakano Underpass
- Ikebukuro

Digital world areas:
- EDEN (Entrance, Community Area, Free Area, Comimani Forum, Open Space)
- DigiLab (inside EDEN)
- Colosseum (inside EDEN)
- Zaxon Forum
- Demons' Den
- Kamishiro Enterprises
- Kowloon Lv.1-5 (dangerous digital slum)
- Digital Spaces 01-22 (numbered dungeon areas)
- Digital Networks I-XII (dungeon networks)
- Special areas: Valhalla Server, Avalon Server, Digital World, King Drasil Core, etc.

Hacker's Memory also adds:
- Hacker Forum
- Power Plant Server, Financial Institution Server, Cyber Crimes Investigation Unit Server
- Under Kowloon, Abyss Server, K's Server

#### DigiLine (CUiDigiline, CUiFieldDigiLine)
In-game messaging system — Digimon on the farm can send messages to the player. Also
used for story NPC communications. The player receives notifications when new messages arrive
(digiline_text.mbe ID 2: "Received N new messages."). Farm Digimon chat messages are in
digiline_text.mbe IDs 1101+.

#### Digital Fields / Hacking (CUiFieldHacking)
In some areas the player encounters interactive hacking puzzles where skills are used.

### 18. EDEN Hub (CUiEden, CUiInternetTop)
EDEN is the virtual social network / cyberspace the game takes place in.
The "Internet Top" (CUiInternetTop) is a hub with options:
- DigiLab (Digimon management)
- Colosseum (PvP battles)
- Digital Network (dungeon access)
- BBS (bulletin board for messages)
- DigiMarket (online shop)
- Player Settings (avatar customization)

EDEN areas from eden_text.mbe / eden_group_text.mbe (para format — non-standard CSV).

### 19. Colosseum (CUiColosseum, CUiRanking, CUiRankingBattle)
Online/local PvP battle modes (colosseum_text.mbe):
- Ranking Battle (online ladder)
- Local Battle (same device)
- Opponent Search
- See Top 100
- See My Rank

Match types:
- Fight Freely
- Flat Rules (all Digimon set to Lv.50 during battle)
- Restriction (custom rules)

Colosseum events with limited-time items.

### 20. Yes/No Dialog (CUiYesNoWindow)
Used throughout the game for confirmation prompts.
- State at +0x0C: 0=idle, 4=interactive, 5=closing, 6=done
- Cursor at +0x81 (byte): 0=No (bottom), 1=Yes (top) — SWAPPED from intuitive order
- Yes text ID at +0x78, No text ID at +0x7C (look up from common_message)
- Default IDs: 2100="Yes", 2101="No"
- Cancel flag at +0x80 (B button dismiss)

Custom labels are used for special dialogs (e.g., "Cyber Sleuth"/"Hacker's Memory").
The message text itself comes from yes_no_message.mbe (captured by TextCapture).

### 21. Multi-Select Dialog (CUiMultiSelectWindow)
For dialogs with 3 or more options (e.g., choosing between party members, medicine types,
elevator floors). Uses multi_select_text.mbe. Examples of uses:
- Choosing which character to bring on a mission (IDs 600-620: Erika, Ryuji, Chitose, etc.)
- Choosing a floor in a building (elevator selections with floor names)
- Choosing which type of medicine to give a Digimon (virus/data/vaccine/free medicine)
- Choosing who the murderer is (hospital investigation scene)

### 22. Info Window (CUiInfoWindow)
Pop-up notification shown briefly. Used for:
- Item obtained/lost
- Keyword obtained
- Quest progress messages
- Scan completion
- Level up notifications within field
- Story progress markers

### 23. Money Window (CUiMoneyWindow)
Shows current wallet amount. Appears overlaid when buying/selling or receiving rewards.

### 24. Tutorial Window (CUiTutorialWindow)
Slide-in tutorial panels triggered by script. Topics (tutorial_title.mbe):
- Battle: Stats and Timeline
- Battle: Battle Commands
- Battle: Digimon Scanner
- Battle: Types and Attributes
- Battle: Abnormal Statuses
- Battle: Cross Combos and CAM
- Battle: Guest Characters
- Setting Skills
- Support Skills
- DigiLab
- Encounters with Digimon
- DigiBank: Moving/Evolution/Loading/DigiConverting/Returning/Sending Digimon
- DigiFarm: About/Commands/Farm Goods/Expansion/Leader/Contact
- DigiLine
- Digimon Medals
- Mirror Dungeons
- Access Points
- White Board (Cases)
- Keywords
- Connect Jump
- Moving around EDEN
- Saving
- Hacking Skills and specific skills
- Digimon Field Guide
- Network features (Player Settings, Battle Box, Custom Battle, Ranking, Event, Coin Shop)
- Cyber Sleuth and Hacker's Memory (save data sharing)
- PC Menu, BBS, DigiMarket, Domination, Hacking Emblems, etc.

### 25. Save/Load Screen Details (CUiSaveload)
From common_message.mbe:
- "Current Progression" (ID 3500)
- "Online Storage Data" (ID 3501)
- Date of Update: [date]
- Play Time: X hrs Y mins
- "Upload data?", "Uploading...", "Upload failed. Try again?", "Upload complete."
- "Begin game using downloaded data?"

### 26. Digimon Book (CUiDigimonBookIndex, CUiDigimonBookDetail)
Full Digimon encyclopedia. Each entry has:
- Name
- Generation, Type, Element
- Long lore description
- Scan rate (percentage collected toward 100% for DigiConvert)

Alphabetical filter (common_message.mbe IDs 1601-1610: "A to B", "C to F", etc.)

### 27. Keyword Menu (CUiKeywordMenu)
Player collects "keywords" throughout the story — clues and topic names related to NPCs
and investigation. Keywords are named and explained in keyword_name.mbe and
keyword_explanation.mbe. Used to unlock information or progress quests.

### 28. Medal List (CUiMedalList)
Collection of Digimon medals obtained. Medals can be bought from the Medal Master in
K-Cafe (shop_text.mbe IDs 1067-1073). Used in the Colosseum or for collection completion.

### 29. Player Setting (CUiPlayerSetting)
Avatar customization. Player can change their in-game avatar appearance (costume) and
accessories. Costume changes: CUiCostumeSelect.

### 30. BBS (Bulletin Board System)
In-game social network BBS. Players post to and read the BBS.
bbs_post_text.mbe contains post content. Post IDs 9900+ are named after team usernames
(e.g., "000DENNOUTANNTEI", "001TYPHOONGIRLSN") — these are NPC team names.

### 31. Hacker's Memory Specific Screens
- PC Menu ("PC選單") — Keisuke's home computer interface, accessed differently from CS's menus
- Hacker emblems — collectible insignia in HM
- Domination — a turn-based strategy mini-game unique to HM (explained in tutorial IDs 57-60, 70):
  - Players take turns moving on a grid
  - Capture spaces
  - Fight battles over contested spaces
  - Win by eliminating enemies or capturing all spaces
- Hudie team base and HM-specific story locations
- HM campaign has Hacker Rank progression (hacker_rank.mbe IDs 101-120) vs CS's
  Cyber Sleuth Rank (IDs 1-20)
- Hackers Battle Mission (hackers_battle_mission.mbe) — HM-exclusive battle missions

### 32. Loading Screen (CUiLoading)
Appears during area transitions.

### 33. Authorization Screen (CUiAuthorization)
Data use agreement shown at first startup (common_message.mbe IDs 2251-2253:
"Data Use Agreement", "Accept", "Do not accept").

### 34. First Sequence (CUiFirstSequence)
The opening sequence when launching the game for the first time. Includes the intro
cutscene and campaign selection flow.

---

## Control Scheme (PC/Controller)

The game is primarily designed for controller (PlayStation-style layout). On PC it supports
keyboard but UI is described with button labels. Observed from game strings:
- A button: confirm/interact
- B button: cancel/back (also "cancel" in yes/no dialogs — separate from cursor)
- Menu/Start: open main menu
- D-pad / left stick: navigate menus and move in field
- R button / right stick: camera control in field

Keyboard controls exist but are not well-documented in the game text files.

---

## Battle System Deep Dive

### Turn Order (Timeline)
SPD stat determines order. Displayed as a timeline bar at top of screen. Higher SPD = acts
sooner. Guest characters (story NPCs accompanying the player) also appear in the timeline.

### Stats
From common_message.mbe:
- Lv (Level, 1-99 normally, 50 for online battles)
- Max HP (Hit Points — health)
- Max SP (Skill Points — mana/energy for skills)
- ATK (Attack — physical damage)
- DEF (Defense — physical resistance)
- INT (Intelligence — magic damage and magic resistance)
- SPD (Speed — turn order)
- ABI (Ability — permanent accumulated bonus from digivolution chains, increases stat caps)
- CAM (Camaraderie — friendship 0-100%, increases through combat and farm care)

### Types (Digimon type — determines battle affinities)
- Virus (strong vs Data, weak vs Vaccine)
- Vaccine (strong vs Virus, weak vs Data)
- Data (strong vs Vaccine, weak vs Virus)
- Free (neutral — no strengths/weaknesses by type)

### Elements (attribute)
- Neutral, Fire, Water, Plant, Electric, Earth, Wind, Light, Dark
- Skills have elements. Hitting an elemental weakness deals more damage.

### Skill Types
Skills have:
- Name (skill_name.mbe)
- Description with element, power, target, and effects (skill_content_name.mbe)
- Target type (skill_target_name.mbe): None, Self, Single Ally, Single Ally/All Allies,
  Single Foe, All Foes, Single Foe/Single Ally, All

Support Skills (passive, one equipped per Digimon) have group names suggesting families:
- Dragon's Wrath (Dragon family bonus)
- Deep Savers, Nature Spirits, Metal Empire, Animal Colosseum, Wind Guardians,
  Justice Genome, Nightmare Soldiers, Flower Guard (species families)
- Supreme Focus, Numbing Waves, Full Functionality, Phoenix Feathers, etc. (stat effects)

### Status Effects
From skill_name.mbe:
- Paralyzed (can't act that turn)
- Asleep (can't act, may last multiple turns)
- Immobilized by recoil (post-action skip — after using certain heavy skills)

### Cross Combo
When multiple Digimon with high Camaraderie (CAM) act consecutively, they may trigger
a Cross Combo — a cooperative attack dealing additional damage. Referenced in tutorial ID 6.

### Guest Characters
In story battles, NPC allies may join. They fight with their own Digimon and appear in
the battle timeline. They are not player-controlled by default. Referenced in tutorial ID 7.
`this.Battle.ChangeGuest()` and `this.Battle.SetParameter(-4, ...)` are used in scripts
to configure guest Digimon stats.

---

## Progression Systems

### Digivolution (Evolution)
Digimon evolve into more powerful forms when requirements are met (accessed via DigiBank).
Requirements from common_message.mbe (IDs 1215-1233):
- Level X or higher
- Max HP is X or higher
- Max SP is X or higher
- ATK, DEF, INT, SPD, ABI, CAM thresholds
- Must have discovered the target form before (scanned in the field)

Stages (generation.mbe):
Training I → Training II → Rookie → Champion → Ultimate → Mega → Ultra
(Armor and Boss are special categories)

### De-Digivolution
Can revert to a lower form. The Digimon keeps its ABI (Ability) points, allowing stat caps
to be raised by re-evolving. Common strategy: "ABI grinding" — evolve up and de-evolve down
repeatedly to accumulate ABI and unlock high-stat evolutions.

### DNA Digivolution (Jogress)
Two Digimon combine into one super-powerful form. One Digimon is consumed (removed from
storage). Confirmation: "Consume [d0] for DNA Digivolution?"

### Mode Change
Some Digimon have alternate modes. Stats don't change during mode change — only appearance
and potentially skills. Unconditional.

### Loading
"Loading" a Digimon into another sacrifices the loaded Digimon to permanently increase
the host Digimon's stats. Confirmation: "Load the Digimon?"

### DigiConvert
When a Digimon species reaches 100% scan rate in the field guide, a copy can be hatched
at the DigiLab. The new Digimon starts from its lowest evolution stage.

### ABI (Ability)
A hidden-ish stat that accumulates when Digimon digivolve/de-digivolve. Higher ABI allows
certain high-end Digimon to be evolved into (unlocking otherwise locked paths).

### CAM (Camaraderie)
Friendship percentage. Increases through:
- Winning battles with that Digimon in party
- Feeding food items on the farm
- Completing farm contact interactions

High CAM (100%) is required for certain evolutions and enables Cross Combo in battle.

### Hacker Rank (CS) / Rank (HM)
The player has a rank title that increases as quests are completed (hacker_rank.mbe):
- CS ranks: "Baby Cyber Sleuth" (rank 1) → "Best Cyber Sleuth" (rank 20)
- HM ranks: "Rookie Hacker" (rank 101) → "Vanguard" (rank 120)

### Medals
Collectible Digimon-themed medals. Bought from Medal Master in K-Cafe (4F).
Can be sold or kept for collection. Colosseum rewards coins that can buy special medals.

### Farm Goods
Items produced when Digimon on a Farm Island are assigned the "Developing" command.
Items are then collected from the farm (digifarm_text.mbe ID 50: "Get Received Items").

### Equipment (Equip)
Attachments that boost specific stats. Named "X Attach Y" where X is the stat and Y is
the tier (E/D/C/B/A from weakest to strongest):
- HP Attach E through A
- SP Attach E through A
- ATK Attach E through A
- DEF Attach E through A
- INT Attach E through A
- SPD Attach E through A
(equip_name.mbe, equip_explanation.mbe)

---

## NPC Dialog System (CUiTalkWindow)

All story dialog and NPC conversations go through CUiTalkWindow (vtable 0xaaef00).
The script loads a message file (`this.Talk.Load(fileName)`), then calls message IDs.

Dialog types (from script API):
- `Message(id)`: standard dialog box — player presses button to advance
- `MessageTalk(id)`: dialog with speaker portrait/name
- `Message_C(id)`: centered text
- `Message_Auto(id)` / `MessageTalk_Auto(id)`: auto-advances after a set time
- `MessageSel(count, id)` / `MessageTalkSel(count, id)`: dialog with selection choices
  (the count specifies how many options, IDs follow in sequence)
- `MessageComic(id, position)`: comic-style speech bubble at a position

Player confirms dialog with A button (IsNext check). Selection result from
`ResultSelectedNum()` (0-indexed).

Message files named like `m01_t01_1501` (area m01, talk t01, script 1501) or
`d0101` (dungeon 01, area 01).

---

## Info Windows and Pop-up Types

From Squirrel API (m00_e00_0001.txt):
- `this.Window.OpenInfo(id)` — info window (tap to close)
- `this.Window.OpenInfoGetItem(itemNo, num)` — "You got X"
- `this.Window.OpenInfoReleaseItem(itemNo, num)` — "You lost X"
- `this.Window.OpenInfoGetKeyword(keyNo)` — "You got Keyword X"
- `this.Window.OpenYesNoInfo(id)` — Yes/No question (info style)
- `this.Window.OpenYesNoSystem(id)` — Yes/No question (system style, modal)
- `this.Window.OpenSystemWindow(id)` — system message (non-dismissable popup)
- `this.Window.OpenTutorial(id)` — tutorial panel
- `this.Window.OpenMultiSelect(id)` — multi-choice dialog
- `this.Window.CloseMoneyWindow()` — closes money HUD overlay

Result retrieval:
- `this.Window.GetResultYesNoInfo()` → 1=YES, 2=NO, 3=CANCEL
- `this.Window.IsNextMultiSelect()` waits for selection
- `this.Talk.ResultSelectedNum()` — result index for MessageSel

---

## Vista (Cutscene / Subtitle) System

Full-motion cutscene system with animated character models and timed subtitles.

Key calls from scripts:
- `this.Vista.Start()` — initialize Vista
- `this.Vista.LoadBackGround("t3001f")` — load background
- `this.Vista.LoadCameraAnimation("CamName")` — camera path
- `this.Vista.LoadAnimation("npcId", "animName")` — character animation
- `this.Vista.LoadSubtitle("subtitle_m00_e00_0001_01")` — load subtitle schedule
- `this.Vista.PlaySubtitle()` — start playback (subtitles appear on timed cues)
- `this.Vista.Play()` — start the scene
- `this.Vista.Exit()` / `this.Vista.End()` — cleanup

Subtitle files are named `subtitle_SCENEID`. The subtitle_text.mbe table has the actual
text. Subtitle schedule MBEs (in DSDB/data/) have columns:
`{id, StartTime, EndTime, SubtitleTextID, VoiceAudioID}`

SubtitleHandler polls the Vista subtitle loader singleton (RVA 0xf205d0) for cue changes.

Other Vista modes:
- `this.Vista.EnableScenarioSelect()` — shows campaign selection during opening cutscene
- `this.Vista.OpenOptionForStart()` — shows initial options screen at first launch
- `this.Vista.OpenLoadAnotherScenario()` — import data from other campaign

---

## Music and Sound

BGM tracks referenced in scripts using codes like "M24", "M74", etc. (custom_sound_bgm.mbe
maps these). Sound effects use filenames like "com_033", "ev_101", "ev_102".

Options screen allows BGM volume, SE volume, and Voice volume (0-100%).
Custom Sound feature lets players assign custom BGM to scenes.

---

## Key Game Terms

| Term | Meaning |
|---|---|
| EDEN | The virtual social network / digital world |
| DigiLab | The central Digimon management facility inside EDEN |
| DigiBank | Where Digimon are stored (like a PC box) |
| DigiFarm / Farm | Farm Island where Digimon train and produce items |
| DigiConvert | Hatching a new Digimon from 100% scan data |
| DigiLine | In-game messaging app (Digimon can message the player) |
| DigiMarket | Online marketplace to buy Digimon scan data |
| CSP | Cyber Sleuth Points — earned from quests, used for ranking |
| HP | Hit Points |
| SP | Skill Points |
| ATK | Attack |
| DEF | Defense |
| INT | Intelligence (magic offense + resistance) |
| SPD | Speed (turn order) |
| ABI | Ability (permanent stat modifier from digivolution cycles) |
| CAM | Camaraderie (friendship percentage with a Digimon) |
| Wall Crack | Hacking skill to remove firewalls in dungeon paths |
| Connect Jump | Traveling through a digital portal from a physical device |
| Hacker Points | HM-exclusive progression currency |
| Domination | HM-exclusive strategy mini-game mode |
| Cross Combo | Battle combo triggered by high-CAM Digimon acting together |
| Scan Rate | How much data collected on a Digimon species (0-100%) |
| Timeline | Battle UI showing upcoming turn order |
| Personality | Digimon's growth type (Durable/Lively/Fighter/etc.) affects which stats grow faster on farm |

---

## Screen-Reader Priority / Coverage Notes

Screens most critical for playability (rough priority order):

1. **Title Screen** — entry point (done: TitleHandler)
2. **Scenario Select** — must choose campaign (done: ScenarioSelectHandler)
3. **Cutscenes / Subtitles** — story understanding (done: SubtitleHandler)
4. **Yes/No Dialog** — pervasive, blocks all progress (done: YesNoHandler)
5. **Main Menu** — top-level navigation (done: MainMenuHandler)
6. **NPC Dialog (CUiTalkWindow)** — story and quest text, most of the game content
7. **Multi-Select Dialog (CUiMultiSelectWindow)** — story choices and NPC branching
8. **Battle Commands** — CUi_Btl_Skill, CUi_Btl_Item, CUi_Btl_Chang, CUi_Btl_Order
9. **Battle Status** — HP/SP/turn order readout
10. **Battle Result / Level Up** — critical feedback after fights
11. **Info Window (CUiInfoWindow)** — item/keyword/progress notifications
12. **Save/Load (CUiSaveload)** — essential for persistence
13. **Options (CUiOption)** — accessibility settings
14. **Organize (CUiOrganize)** — party management
15. **DigiBank** — Digimon management hub
16. **Shop** — buying/selling items
17. **Quest Board** — quest tracking
18. **Status Menu** — Digimon stats
19. **Field hacking UI** — dungeon navigation
20. **Farm** — long-term progression
21. **Item Menu** — item management
22. **Digimon Book** — encyclopedia (nice to have)
23. **Colosseum/Online** — multiplayer (lower priority for accessibility)

---

## MBE Text Table Summary

| Table (mbe file) | Contents |
|---|---|
| main_menu | 8 main menu items |
| menu_text | HUD labels (Wallet, Organize, Party Memory, etc.) |
| common_message | Shared UI: stats, options labels, Yes/No, volume, digivolution messages |
| info_message | Field info notifications, tutorial popups |
| yes_no_message | Yes/No dialog body text (the question itself) |
| battle_info_message | Battle-specific messages (Critical Hit, etc.) |
| shop_text | Shop dialog text |
| quest_text | Quest titles and descriptions |
| digifarm_text | Farm menu labels |
| digifarm_food_text | Farm food item names |
| digiline_text | DigiLine messages and farm Digimon chat |
| digimon_type | Virus/Vaccine/Data/Free |
| element | Fire/Water/Plant/Electric/Earth/Wind/Light/Dark/Neutral |
| generation | Training I-II/Rookie/Champion/Ultimate/Mega/Ultra/Armor/Boss |
| personality | Durable/Lively/Fighter/Defender/Brainy/Nimble/Builder/Searcher |
| skill_name | All skill names (battle commands at low IDs, Digimon skills at 1001+) |
| skill_target_name | Target types for skills |
| skill_content_name | Skill descriptions |
| support_skill_name | Passive support skill names |
| hacking_skill_name | Field hacking skill names |
| hacking_skill_explanation | Hacking skill descriptions |
| item_name | All item names |
| item_explanation | Item descriptions |
| equip_name | Equipment (attachment) names |
| equip_explanation | Equipment descriptions |
| keyword_name | Investigation keyword names |
| keyword_explanation | Keyword descriptions |
| fieldname | Field/dungeon/area internal names |
| map_select_town_name | Location names for map travel menu |
| charname | NPC/character names |
| scenario_select | Campaign select screen text |
| colosseum_text | Colosseum menu and match options |
| medal_name | Digimon medal names |
| hacker_rank | Player rank titles (CS and HM) |
| subtitle_text | Cutscene subtitle text (via Vista system) |
| tutorial_title | Tutorial panel topic names |
| multi_select_text | Multi-choice dialog options (elevator, party choice, etc.) |
| evolution | Story completion messages (case solved, etc.) |
| bbs_post_text | BBS post content |
| story_progress | Story milestone messages |
| mirror_dungeon_text | Mirror Dungeon location names |
| eden_text | EDEN area info (para format) |
| eden_group_text | EDEN group info (para format) |
| tournament_name | Tournament event names |
| custom_sound_bgm | BGM track listing for custom sound |
| custom_sound_scene | Scene names for custom sound |
| hackers_battle_mission | HM-exclusive battle mission text |
| colosseum_event_battle_mission | Colosseum event mission text |
| colosseum_free_rule_comment | Colosseum free rule comments |
| colosseum_item_name | Colosseum item names |
| colosseum_item_explanation | Colosseum item descriptions |
| digimon_book_explanation | Digimon encyclopedia lore text |
| hacking_skill_condition_exp | Hacking skill condition explanations |
| help_message | Help text |
| dig_farm | Farm data names |
| bgm | BGM track names |
| quest_text_add | Additional quest text (HM?) |

---

## Notes on CUi Classes vs Game Screens

From the 128 CUi* vtable dump, these are the classes that correspond to major interactive
screens. Each needs a handler for full coverage:

**Already handled (v0.11.0):**
- CUiTitle
- CUiScenarioSelect (vtable 0xab07d8, tick RVA 0x4c89a0)
- CUiMainMenu (vtable 0xab0588, tick RVA 0x4b6270)
- SubtitleHandler (no CUi hook — pure memory polling of Vista loader)
- CUiYesNoWindow (vtable listed in offsets.h, tick RVA 0x426C90)

**Priority remaining:**
- CUiTalkWindow (vtable 0xaaef00) — NPC dialog
- CUiMultiSelectWindow (vtable 0xaaf4b0) — multi-choice
- CUiInfoWindow — info notifications
- CUi_Btl_bace / CUi_Btl_Skill / CUi_Btl_Item / CUi_Btl_Chang / CUi_Btl_Order — battle
- CUiBattleResult / CUiLevelUp / CUiChooseHoldSkill — post-battle
- CUi_Btl_GameOver — game over
- CUiSaveload — save/load
- CUiStatusMenu — Digimon stats
- CUiOrganize — party organize
- CUiOption / CUiSettingMenu — settings
- CUiShopMenu / CUiShopBuyList / CUiShopSellList — shop
- CUiQuestBoard / CUiQuestDetail — quests
- CUiDigibankTop and sub-screens — DigiBank
- CUiDigifarm and sub-screens — Farm
- CUiMenuTop (vtable 0xab0120) — top-level menu wrapper?

---

## Squirrel Script Naming Convention

Scripts are named by area and type:
- `m{major}_{minor}_{sequence}.txt` — story event scripts
  - e.g., `m00_e00_0001.txt` = chapter 0, EDEN event 00, sequence 001
  - `m00_t01_1501.txt` = chapter 0, Talk file t01, message 1501
- `battle_{id}.txt` — battle event scripts (enemy AI, boss behavior)
- `d{dungeon}{area}.txt` — dungeon dialog
- `m00_e00_common.txt` — shared common functions (include in all event scripts)
- `battle_0000.txt` — shared battle common functions

The battle common script defines the callback structure:
- `Battle_Init()` — set BGM
- `Battle_Boot()` — pre-battle setup
- `Battle_Start()` — battle begins
- `Battle_Command()` — per-command phase
- `Battle_Victory()` — win
- `Battle_Defeat()` — lose
- `Battle_Turn_End()` — end of turn
- `Battle_Direction_End()` — animation end
- `Battle_Direction_Result()` — animation result

---

*End of raw game info. Last updated from game data extraction: 2026-03-03.*
