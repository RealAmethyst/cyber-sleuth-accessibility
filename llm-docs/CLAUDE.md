# llm-docs — Game Reference Documentation

This directory contains reference documentation about Digimon Story Cyber Sleuth Complete Edition, gathered from extracted game data and organized for use when building accessibility handlers.

## Files

### game-screens.md
Complete enumeration of all 34 game screens with:
- CUi* class names and vtable addresses
- What information is displayed on each screen
- What the player interacts with
- Screen-reader priority ranking (which screens to implement first)
- Battle system breakdown (commands, stats, types, elements)
- Progression systems (digivolution, ABI, CAM, equipment)

### text-tables.md
Reference for all 61 MBE text tables the game uses, organized by category:
- Core UI text (main_menu, common_message, yes_no_message, etc.)
- Battle text (skill_name, battle_info_message, etc.)
- Items and equipment
- Digimon/world data
- Which tables each handler needs

### local-data-catalog.md
Catalog of all extracted game data available in the research repo:
- 61 text CSVs, 1,451 message CSVs, 2,623 decompiled scripts, 214 data CSVs
- Naming conventions for message tables and scripts
- Key Squirrel API patterns from script analysis
- Ghidra analysis output locations

## How to Use
When implementing a new handler, check `game-screens.md` for the screen's CUi class and what it displays, then check `text-tables.md` for which MBE tables provide the text you need. For understanding game flow or script patterns, consult `local-data-catalog.md` for the relevant decompiled scripts.
