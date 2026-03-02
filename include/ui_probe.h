#pragma once

#include "hooks.h"

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

// Discovery tool: hooks update functions for many CUi* classes via MinHook,
// identifies which are active by checking vtable pointers at runtime,
// and registers them with MemoryInspector for F5 dumps.
//
// MinHook patches the function prologue so ALL calls are intercepted
// regardless of dispatch mechanism (vtable, direct call, table-driven).
//
// NOTE: Requires unpacked exe (Steamless). With stock Steam DRM, the
// unpacker overwrites MinHook patches. Development/testing uses the
// unpacked exe; shipping plugin will use delayed hook installation.
//
// Multiple classes may share the same update function (inherited from base).
// UiProbe handles this by deduplicating hooks and identifying the class
// from this->vtable in the detour.
class UiProbe
{
public:
    static UiProbe* Get();

    struct ClassInfo {
        const char* name;
        uintptr_t vtableRva;  // RVA of the vtable (not the function)
    };

    // Install MinHook detours for all given classes.
    // Reads vtable[3] (tick) for each to find the actual function address,
    // deduplicates, and hooks each unique function.
    // Call AFTER hooks_init() (MinHook must be initialized).
    void Install(const std::vector<ClassInfo>& classes);

    void Uninstall();

private:
    UiProbe() = default;

    // Absolute vtable address -> class name (for identification in detour)
    std::unordered_map<uintptr_t, std::string> m_vtableToName;

    // Hooked function address -> trampoline (original function)
    std::unordered_map<uintptr_t, void*> m_funcToOriginal;

    // List of hooked function addresses (for cleanup)
    std::vector<void*> m_hookedTargets;

    // Single detour for all probed classes.
    // Identifies class from this->vtable, calls correct original via trampoline.
    // vtable[3] tick takes (this, param2).
    static void __fastcall ProbeDetour(void* thisPtr, void* param2);
};
