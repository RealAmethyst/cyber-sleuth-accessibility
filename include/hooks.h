#pragma once

#include <functional>
#include <vector>

// --- VTable hooking utilities ---

// Swap a vtable entry at the given index for an object.
// Returns the original function pointer.
// object: pointer to the C++ object (first qword is vtable ptr)
// index: vtable slot index (0-based)
// newFunc: replacement function
void* HookVTableEntry(void* object, int index, void* newFunc);

// Same but takes a known vtable address directly (no object needed).
void* HookVTableByAddress(uintptr_t vtableAddr, int index, void* newFunc);

// --- Frame handler system ---

// Handler interface: called once per frame from the SwapBuffers hook.
class IFrameHandler {
public:
    virtual ~IFrameHandler() = default;
    virtual void OnFrame() = 0;
};

void RegisterFrameHandler(IFrameHandler* handler);
void UnregisterFrameHandler(IFrameHandler* handler);

// --- Lifecycle ---

// Initialize hooks (MinHook + SwapBuffers frame callback).
// Call from onEnable() after SpeechManager is initialized.
bool hooks_init();

// Remove all hooks and clean up.
// Call from onDisable().
void hooks_shutdown();
