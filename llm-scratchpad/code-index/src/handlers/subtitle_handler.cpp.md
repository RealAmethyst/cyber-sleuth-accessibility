// File: src/handlers/subtitle_handler.cpp
// Purpose: Accessibility handler for Vista cutscene subtitles.
//          Pure memory polling — no MinHook or tick detour needed.
//          Polls the subtitle loader singleton (DAT_140f205d0, RVA 0xf205d0)
//          each frame to detect cue index changes, then calls the game's own
//          LookupText("subtitle_text", textId, language) to retrieve text.
//
//          Loader state 2 = playing; cue index derived from (cursor - begin) / 8
//          where begin/cursor are pointers into the cue vector.
//          All memory reads are SEH-protected (__try/__except).

// Singleton accessor.
SubtitleHandler* SubtitleHandler::Get()  (line 19)

// Caches module base address and LookupText function pointer. No hooks installed.
void SubtitleHandler::Install()  (line 29)

// Resets state; clears lookupText pointer.
void SubtitleHandler::Uninstall()  (line 43)

// --- Memory access helpers (all SEH-protected) ---

// Dereferences DAT_SubtitleLoader (module_base + RVA) to get the loader object pointer.
// Returns nullptr on any memory fault.
void* SubtitleHandler::GetLoader() const  (line 60)

// Reads int32 at loader + Offsets::Vista::Loader::STATE (0x4C).
// Returns 0 on fault or null loader.
int SubtitleHandler::GetLoaderState() const  (line 71)

// Computes current cue index as (cursor_ptr - vector_begin_ptr) / 8.
// Reads VECTOR_BEGIN (0x80) and CURSOR (0x98) from the loader.
// Returns -1 on fault or invalid pointers.
int SubtitleHandler::GetCueIndex() const  (line 84)

// Navigates the cue data chain: vector[cueIndex] -> cueStruct[0] -> rowData + 0x0C
// to read the SubtitleTextID (int32). Returns -1 on any fault or null pointer.
int SubtitleHandler::GetCueTextId(int cueIndex) const  (line 102)

// Calls the game's LookupText (via cached m_lookupText pointer) with the text
// table manager singleton and current language. Returns nullptr on any fault.
const char* SubtitleHandler::LookupSubtitleText(int textId) const  (line 127)

// File-scope SEH helper — reads the schedule name string pointer from
// loader + Offsets::Vista::Loader::NAME (0x68). Returns nullptr on fault.
static const char* ReadLoaderName(void* loader)  (line 153)

// IFrameHandler::OnFrame. Polls loader state each frame:
//   - state != 2: if was playing, log end and clear state.
//   - state == 2, just started: log schedule name, reset cue tracking.
//   - state == 2, cue index changed: get text ID, look up text, speak (interrupt).
//     Deduplicates: skips if text matches m_lastSpokenText.
void SubtitleHandler::OnFrame()  (line 169)
