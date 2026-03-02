#pragma once

#include "hooks.h"

#include <cstdint>
#include <string>
#include <vector>
#include <array>
#include <unordered_map>

// Discovery tool: hooks update functions for many CUi* classes via MinHook,
// identifies which are active by checking vtable pointers at runtime,
// and registers them with MemoryInspector for F5 dumps.
//
// Uses per-function template detours so each hook knows exactly which
// trampoline to call -- no fragile runtime vtable lookups.

class UiProbe
{
public:
    static UiProbe* Get();

    struct ClassInfo {
        const char* name;
        uintptr_t vtableRva;
    };

    void Install(const std::vector<ClassInfo>& classes);
    void Uninstall();

    static constexpr size_t MAX_HOOKS = 32;

    struct HookEntry {
        void* trampoline = nullptr;
        uintptr_t targetFunc = 0;
        char classNames[128] = {};  // POD-safe, no std::string in SEH path
        bool disabled = false;      // Set on fault — skips all logic, stops log spam
    };

    static HookEntry s_entries[MAX_HOOKS];
    static size_t s_entryCount;
    static std::unordered_map<uintptr_t, std::string> s_vtableToName;

private:
    UiProbe() = default;

    std::vector<void*> m_hookedTargets;

    // Per-index template detour
    template<size_t Index>
    static void __fastcall ProbeDetourN(void* thisPtr, void* param2);

    // SEH wrapper (no C++ objects — safe for __try/__except)
    static void ProbeCommonSEH(size_t hookIndex, void* thisPtr, void* param2);

    // Inner logic (may use C++ objects)
    static void ProbeCommonInner(size_t hookIndex, void* thisPtr, void* param2);

    // Runtime-initialized detour table
    static void* s_detourTable[MAX_HOOKS];
    static bool s_detourTableInit;
    static void InitDetourTable();

    template<size_t... Is>
    static void InitDetourTableImpl(std::index_sequence<Is...>) {
        ((s_detourTable[Is] = reinterpret_cast<void*>(&ProbeDetourN<Is>)), ...);
    }

    static std::unordered_map<uintptr_t, bool> s_seenVtables;
};

// Template detour implementations
template<size_t Index>
void __fastcall UiProbe::ProbeDetourN(void* thisPtr, void* param2)
{
    static_assert(Index < MAX_HOOKS, "Hook index out of range");
    ProbeCommonSEH(Index, thisPtr, param2);
}
