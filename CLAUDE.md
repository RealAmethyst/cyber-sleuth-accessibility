# Digimon Story Cyber Sleuth - Accessibility Mod

## Project Goal
Screen-reader accessibility mod for Digimon Story Cyber Sleuth Complete Edition (Steam/PC), enabling blind/visually impaired players to play the game.

## Game Overview
Digimon Story Cyber Sleuth Complete Edition is a JRPG with two campaigns (Cyber Sleuth and Hacker's Memory) set in near-future Tokyo and the digital world EDEN. Turn-based combat with up to 3 Digimon per side, 340+ Digimon species with branching digivolution trees, and extensive menus for party management, farming, shopping, and questing. Primarily controller-based (PlayStation-style) with keyboard support. See `llm-docs/` for detailed game screen enumeration, text table reference, and data catalog.

## Repositories

### This Repo (Plugin Code)
- **GitHub:** https://github.com/RealAmethyst/cyber-sleuth-accessibility
- C++ plugin: CMakeLists.txt, src/, include/, cmake/

### Research & Data Repo
- **Local:** `C:\Users\Amethyst\projects\digimon story cyber slooth`
- NOT on GitHub — contains extracted game data, Ghidra scripts, decompiled Squirrel scripts, CSVs
- Has its own CLAUDE.md with full game RE knowledge (vtable dumps, member layouts, text tables)
- **Extracted data:** `extracted/csv_text/` (61 text CSVs), `extracted/csv_message/` (1,451 dialog CSVs), `extracted/decompiled_scripts/` (2,623 Squirrel scripts)

## Architecture Overview

### Mod Framework: DSCSModLoader
- **Repo:** https://github.com/SydMontague/DSCSModLoader
- Replaces `freetype.dll` in `app_digister/` to inject into the game at startup
- Game exe: `Digimon Story CS.exe` (64-bit PE, image base 0x140000000)
- **Game install:** `C:\Program Files (x86)\Steam\steamapps\common\Digimon Story Cyber Sleuth Complete Edition`
- Plugin DLLs go in `resources/plugins/` (at install root, sibling to `app_digister/`)
- GPL-3.0 licensed — our plugin must be compatible

### How the Plugin Works
1. Loads as a DSCSModLoader plugin (C++ DLL, entry point in `src/main.cpp`)
2. Integrates **SRAL** for screen reader output (NVDA/JAWS/SAPI)
3. Hooks CUi* tick functions via MinHook to capture `this` pointers
4. Hooks LookupText (TextCapture) to universally capture all text the game displays
5. Uses a handler-based architecture (one handler per game screen)
6. SwapBuffers (OpenGL) hooked for per-frame handler dispatch

### Game Rendering & Input
- **Rendering:** OpenGL (via Cg/cgGL), NOT DirectX
- **Input:** DirectInput8 + XInput
- **Text rendering:** Pre-built font atlas + OpenGL (GDI is never used at runtime)
- **Binary is frozen** — no updates expected, RVA offsets are stable across all users

## Critical Patterns

### Handler Pattern (IFrameHandler)
Each game screen gets a handler that:
1. Hooks the CUi* tick function (vtable[3]) via MinHook **only to capture the `this` pointer**
2. Gets called every frame via SwapBuffers hook (`OnFrame()`)
3. ALL state reading, string building, and speech happens in `OnFrame()` — **never in the tick detour**
4. Tracks changes from previous frame (state-diff)
5. Announces changes via SpeechManager

**Tick-presence detection:** YesNoHandler and ScenarioSelectHandler use a `s_tickFired` atomic bool alongside `s_thisPtr`. When the tick stops firing (bool stays false for a frame), the handler knows the dialog/screen was closed. This is more reliable than checking state fields for detecting screen dismissal.

### CRITICAL: Tick Detours Must Be Minimal
Doing ANY work beyond capturing a pointer inside a tick detour freezes the game. The tick detour must be:
```cpp
void __fastcall HookedTick(void* thisPtr, void* param2) {
    s_originalTick(thisPtr, param2);          // call original
    s_thisPtr.store(thisPtr, relaxed);        // atomic store — NOTHING ELSE
}
```
All logic goes in `OnFrame()` (SwapBuffers context). Speech is also queued — `Speak()` pushes to a queue, `Flush()` dispatches from SwapBuffers.

All handlers inherit from `TickHandler<Derived>` (CRTP template in `include/handlers/tick_handler.h`), which provides the tick hook install/uninstall, `s_thisPtr`/`s_tickFired` atomics, SEH wrapping, and `OnFrame()` dispatch. Handlers only implement `OnFrameInner()`, `OnScreenClosed()`, `GetTickRVA()`, and `GetHandlerName()`.

### TextCapture + Per-Handler Hooks (Layered Approach)
- **TextCapture** (LookupText hook): Universal text source — captures ALL text the game fetches from MBE tables. Language-aware, works for all 6 languages.
- **Per-screen handlers**: Provide state awareness (menu open/close), cursor tracking, display timing.
- **The split:** TextCapture tells you "item 3 is called 'Status'" — the handler tells you "the user just moved to item 3."

### Speech Rules
All spoken text comes from the game — **never hardcode announcement strings**. Exception: text rendered as pre-baked textures (e.g., title screen menu items) must be hardcoded since it's identical in all languages.

| Situation | interrupt | Why |
|---|---|---|
| Menu opened — window name | true | Clear prior speech, announce context |
| Menu opened — first item | false | Queue after window name |
| Cursor moved to new item | true | Old speech is stale |

### Address Strategy
All hooks use `module_base + RVA_offset` (not absolute addresses). Offsets are centralized in `include/offsets.h`.

`offsets.h` also contains a `Context` namespace with `DAT_GameContext`, `FUNC_GetGameContext`, `STORY_MODE_OFFSET`, and campaign detection offsets — infrastructure for distinguishing CS vs HM mode at runtime.

## Build & Deploy

### Prerequisites
- Visual Studio Community 2026 (v18) with "Desktop development with C++" workload
- CMake 3.25+ (added to PATH)

### Build Commands
```bash
export PATH="$PATH:/c/Program Files/CMake/bin"
# Configure (once, or after CMakeLists.txt changes):
cmake -B build -G "Visual Studio 18 2026" -A x64 -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -Wno-dev
# Build:
cmake --build build --config Release --target CyberSleuthAccessibility
```
Output: `build/Release/CyberSleuthAccessibility.dll`

### Deployment
Copy `CyberSleuthAccessibility.dll` to `resources/plugins/` at the game install root.

### Dependencies (fetched automatically via CPM)
- **DSCSModLoader** — mod framework
- **MinHook** — function hooking (built from source)
- **SRAL** — screen reader abstraction (static, UIA disabled)

### CMake Notes
- Must define `BOOST_ALL_NO_LIB` to prevent vc143/vc145 auto-link mismatch
- SRAL options must be set BEFORE `CPMAddPackage`
- SRAL API: `SRAL_Initialize(0)`, `SRAL_Speak(text, interrupt)`, `SRAL_GetCurrentEngine()`, `SRAL_StopSpeech()`, `SRAL_Uninitialize()`

## Game Binary Key Facts

### SteamStub DRM
- Stock exe is wrapped with SteamStub DRM — MinHook patches get overwritten
- **Development uses an unpacked exe** (via Steamless) placed in `app_digister/`
- RVA offsets are identical between packed and unpacked versions

### Text Lookup API
```cpp
void* manager = GetTextTableManager();  // RVA 0x1b8bc0
int language = *(langSettings + 0xb4);  // 0=JP, 1=EN, 2=CN, 3=EN_Censor, 4=KR, 5=DE
const char* text = LookupText(manager, "table_name", rowId, language);  // RVA 0x1b9260
```

### UI Class Hierarchy
128 CUi* classes extracted via RTTI. Common vtable layout has 13 entries; **vtable[3] = per-frame tick/update**. Full vtable RVAs are in `include/offsets.h`. Key classes:
- **Menus:** CUiTitle, CUiMainMenu, CUiMenuTop, CUiOption, CUiSaveload, CUiSettingMenu, CUiStatusMenu, CUiItemMenu
- **Battle:** CUi_Btl_Skill, CUi_Btl_Item, CUi_Btl_Chang, CUi_Btl_Order, CUi_Btl_GameOver
- **Dialog:** CUiTalkWindow, CUiYesNoWindow, CUiMultiSelectWindow
- **Other:** CUiScenarioSelect, CUiDigibankTop, CUiShopMenu, CUiQuestBoard, CUiDigifarm

### Subtitle System (Vista Cinematic)
- Subtitles use `this.Vista.LoadSubtitle()` + `PlaySubtitle()` — NOT CUiTalkWindow
- CUiTalkWindow = NPC dialog boxes (`this.Talk.Message(id)`)
- SubtitleHandler uses pure memory polling (no hooks needed)

## Current Handlers (v0.11.0)

| Handler | Screen | Hook | Notes |
|---|---|---|---|
| TitleHandler | Title screen menu | MinHook tick | Hardcoded items (pre-baked textures) |
| MainMenuHandler | In-game main menu | MinHook tick | Dynamic text via GameText_Lookup |
| SubtitleHandler | Cutscene subtitles | None (polling) | Pure memory polling of Vista loader |
| YesNoHandler | Yes/No dialogs | MinHook tick | Dynamic button labels from common_message |
| ScenarioSelectHandler | Campaign selection | MinHook tick | TextCapture for interactive phase detection; descriptions via GameText_Lookup |
| TextCapture | All text | MinHook LookupText | Universal text source for all handlers |

## Development Phases

### Phase 1: Proof of Concept — DONE
### Phase 2: Core Menu System — IN PROGRESS
- Next: settings, save/load, CUiTalkWindow, CUiMultiSelectWindow
### Phase 3: Battle System
### Phase 4: Field & Navigation
### Phase 5: Full Coverage

## Utility Systems

| System | Purpose | Files |
|---|---|---|
| Logger | File-based debug logging with timestamps | logger.h/cpp |
| SpeechManager | Thread-safe SRAL speech queueing | speech_manager.h/cpp |
| TextCapture | Universal LookupText hook + per-frame diffing | text_capture.h/cpp |
| GameText | Shared wrapper for game's LookupText API | game_text.h/cpp |
| HandlerUtils | ReadMemory<T> template + FormatAnnouncement helpers | handlers/handler_utils.h |
| TickHandler | CRTP base class for all tick-hooked handlers | handlers/tick_handler.h |
| PluginUtil | GetPluginDir() — cached DLL directory path | plugin_util.h/cpp |
| MemoryInspector | F5 memory dumps for offset discovery | memory_inspector.h/cpp |
| UiProbe | Diagnostic-only discovery tool — hooks CUi ticks to find active classes. Not for production use (caused issues when hooking many classes at once) | ui_probe.h/cpp |

## Reference Projects
- **DSCSModLoader:** https://github.com/SydMontague/DSCSModLoader
- **Skyrim Accessibility Mod (vtable pattern reference):** `C:\Users\Amethyst\skyrim-access`
- **Next Order Accessibility Mod:** `C:\Users\Amethyst\projects\digimon world next order`
- **SRAL:** https://github.com/m1maker/SRAL

## Dead Ends (Don't Retry)
- **Squirrel VM hooking:** Game's VM never calls C API functions during execution. HSQUIRRELVM becomes invalid after init. Dead end.
- **VTable entry patching:** Game uses table-driven dispatch, not standard C++ virtual calls. VTable swaps install but never fire. Must use MinHook function hooks.
- **GetGlyphOutlineW/A hooking:** Never called at runtime. Game uses pre-built font atlas.
- **Work inside tick detours:** Any non-trivial work freezes the game. Must use atomic pointer capture + OnFrame() pattern.
