// File: include/hooks.h
// Purpose: VTable hooking utilities and the per-frame handler system.
//          Provides two vtable hook helpers, the IFrameHandler interface,
//          registration/unregistration, and lifecycle init/shutdown.

// --- VTable hooking utilities ---

// Swap a vtable entry for a live object instance (object's first qword is vtable ptr).
// Returns the original function pointer.
void* HookVTableEntry(void* object, int index, void* newFunc);  // (line 13)

// Same as HookVTableEntry but takes a known vtable address directly (no object needed).
void* HookVTableByAddress(uintptr_t vtableAddr, int index, void* newFunc);  // (line 16)

// --- Frame handler system ---

// Interface: implementations are called once per rendered frame from the SwapBuffers hook.
class IFrameHandler {  // (line 21)
    virtual ~IFrameHandler() = default;  // (line 23)
    virtual void OnFrame() = 0;          // (line 24)
}

void RegisterFrameHandler(IFrameHandler* handler);    // (line 27)
void UnregisterFrameHandler(IFrameHandler* handler);  // (line 28)

// --- Lifecycle ---

// Initialize MinHook and install the SwapBuffers frame callback.
// Must be called from onEnable() after SpeechManager is initialized.
bool hooks_init();     // (line 34)

// Remove all hooks and free MinHook resources. Call from onDisable().
void hooks_shutdown();  // (line 38)
