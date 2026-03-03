# Refactoring Session Status

## Branch
`claude-refactor-cleanup` (based on `master`)

## Prompts Completed
- [x] sanity-checks-setup.md — Branch created, llm-scratchpad initialized
- [x] information-gathering-and-checking.md — CLAUDE.md validated and updated, llm-docs created
- [ ] code-directory-construction.md
- [ ] large-file-handling.md (if needed)
- [ ] input-handling.md
- [ ] string-builder.md
- [ ] low-level-cleanup.md
- [ ] high-level-cleanup.md
- [ ] finalization.md

## Scratchpad Files
- `current_status.md` — This file
- `game-info-raw.md` — Raw game info from subagent (also in llm-docs/game-screens.md)
- `local-data-catalog.md` — Data catalog from subagent (also in llm-docs/local-data-catalog.md)

## llm-docs Created
- `llm-docs/CLAUDE.md` — Index of documentation
- `llm-docs/game-screens.md` — 34 game screens, battle system, progression, priority ranking
- `llm-docs/text-tables.md` — MBE text table reference with per-handler needs
- `llm-docs/local-data-catalog.md` — Catalog of all extracted game data in research repo

## CLAUDE.md Changes Made
- Added game overview section
- Added `SRAL_Uninitialize()` to API list
- Fixed ScenarioSelectHandler description (uses direct LookupText, not TextCapture for descriptions)
- Documented `s_tickFired` tick-presence detection pattern
- Noted UiProbe is diagnostic-only
- Documented Context namespace in offsets.h
- Added reference to llm-docs/

## Known Code Issues Found
- `offsets.h` comment says cursor 0=Yes for CUiYesNoWindow, but implementation treats 0=No (bottom) — comment is wrong, Japanese UI convention
- `yesno_handler.cpp` header comment has same wrong cursor direction
- PluginInfo version struct says 0.8.0 but log says v0.11.0 — struct is stale
- MainMenuHandler does all work in tick detour instead of OnFrame()
- `hooks.cpp` mentions "following the same pattern as the Skyrim accessibility mod" for VTable helpers that are essentially dead code (all real hooks use MinHook)
