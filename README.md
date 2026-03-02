# Cyber Sleuth Accessibility

A screen-reader accessibility mod for **Digimon Story Cyber Sleuth Complete Edition** (Steam/PC), enabling blind and visually impaired players to play the game.

## How It Works

This is a C++ plugin for [DSCSModLoader](https://github.com/SydMontague/DSCSModLoader). It hooks into the game's UI classes via vtable interception and announces menu state, dialog text, and battle information through screen readers (NVDA, JAWS) or SAPI.

### Architecture

- **SRAL** (Screen Reader Abstraction Layer) for speech output
- **VTable hooking** on the game's 128 CUi* UI classes (discovered via RTTI)
- **Per-frame polling** via SwapBuffers hook — tracks cursor position and UI state changes
- **Handler pattern** — one handler per game screen, announces changes via state-diff

## Building

### Prerequisites

- Visual Studio 2022+ with "Desktop development with C++" workload
- CMake 3.25+

### Build

```bash
cmake -B build -G "Visual Studio 18 2026" -A x64 -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -Wno-dev
cmake --build build --config Release --target CyberSleuthAccessibility
```

Output: `build/Release/CyberSleuthAccessibility.dll`

### Dependencies (fetched automatically via CPM)

- [DSCSModLoader](https://github.com/SydMontague/DSCSModLoader) — mod framework
- [MinHook](https://github.com/TsudaKageyu/minhook) — function hooking
- [SRAL](https://github.com/m1maker/SRAL) — screen reader abstraction

## Installation

1. Install [DSCSModLoader](https://github.com/SydMontague/DSCSModLoader) into your game directory
2. Copy `CyberSleuthAccessibility.dll` to `resources/plugins/` (next to `app_digister/`)
3. Launch the game — you should hear "Cyber Sleuth Accessibility loaded"

## Status

Early development (Phase 1 — Proof of Concept). The plugin loads, speaks via screen reader, and has vtable hook infrastructure ready. Menu cursor tracking and text announcements are in progress.

## License

GPL-3.0 (compatible with DSCSModLoader)
