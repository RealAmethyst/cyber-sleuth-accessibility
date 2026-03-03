// File: src/hooks.cpp
// Purpose: Hook infrastructure — vtable helpers, per-frame handler registry,
//          and SwapBuffers (OpenGL) hook for per-frame dispatch.
//
// SwapBuffers (gdi32.dll) is hooked via MinHook to get a reliable per-frame
// callback on the game thread. Frame handlers register to receive OnFrame()
// calls, where they can poll UI state and announce changes.
//
// VTable hooks use VirtualProtect to swap individual vtable entries,
// following the same pattern as the Skyrim accessibility mod.
// NOTE: vtable patching does NOT work for per-frame interception in this game
// (game uses table-driven dispatch). These helpers are kept for completeness
// but per-frame hooks must use MinHook function-level hooks instead.

// --- VTable hook helpers ---

// Swaps vtable entry [index] on a live object instance using VirtualProtect.
// Returns the original function pointer.
void* HookVTableEntry(void* object, int index, void* newFunc)  (line 27)

// Swaps vtable entry [index] directly by vtable address (no object needed).
// Returns the original function pointer.
void* HookVTableByAddress(uintptr_t vtableAddr, int index, void* newFunc)  (line 48)

// --- Frame handler registry ---

static std::mutex g_handlerMutex  (line 70)
static std::vector<IFrameHandler*> g_handlers  (line 71)

void RegisterFrameHandler(IFrameHandler* handler)  (line 73)
void UnregisterFrameHandler(IFrameHandler* handler)  (line 81)

// --- SwapBuffers hook ---

using SwapBuffersFunc = BOOL(WINAPI*)(HDC)  (line 95)
static SwapBuffersFunc g_origSwapBuffers  (line 96)
static void* g_addrSwapBuffers  (line 97)
static int g_frameCount  (line 98)

// SwapBuffers detour — flushes the SpeechManager queue, then calls OnFrame()
// on every registered handler, then calls the original SwapBuffers.
static BOOL WINAPI hooked_SwapBuffers(HDC hdc)  (line 100)

// --- Public API ---

// Initialises MinHook, locates SwapBuffers in gdi32.dll (falls back to
// wglSwapBuffers in opengl32.dll), installs and enables the hook.
bool hooks_init()  (line 127)

// Clears all registered handlers, disables and uninitialises all MinHook hooks.
void hooks_shutdown()  (line 180)
