// File: include/ui_probe.h
// Purpose: Development/research discovery tool.
//          Hooks the update (tick) functions of multiple CUi* classes via MinHook,
//          identifies which objects are active by inspecting their vtable pointer at
//          runtime, then registers them with MemoryInspector for F5 memory dumps.
//
//          Uses per-index template detours (ProbeDetourN<Index>) so each hook has its
//          own statically-typed trampoline — avoids fragile runtime vtable lookups.
//          SEH wrappers isolate crashes: a faulting hook sets disabled=true and stops
//          logging to avoid log spam.
//
// NOTE: This was an earlier broad-hooking approach. The current production code uses
//       TextCapture + targeted per-handler hooks instead. UiProbe remains as a research tool.

class UiProbe  // (line 18)
{
public:
    static UiProbe* Get();  // (line 21) — singleton accessor

    // Input descriptor: a class name and its vtable RVA.
    struct ClassInfo {  // (line 23)
        const char* name;
        uintptr_t vtableRva;
    }

    // Install MinHook on the tick function of each class in `classes`.
    void Install(const std::vector<ClassInfo>& classes);  // (line 28)

    void Uninstall();  // (line 29)

    static constexpr size_t MAX_HOOKS = 32;  // (line 31)

    // Per-hook state. POD-safe (no std::string) so it is safe inside SEH regions.
    struct HookEntry {  // (line 33)
        void* trampoline = nullptr;       // MinHook trampoline for calling original
        uintptr_t targetFunc = 0;         // RVA of hooked function
        char classNames[128] = {};        // label, POD char array (no std::string in SEH path)
        bool disabled = false;            // set on access fault — skips all logic thereafter
    }

    static HookEntry s_entries[MAX_HOOKS];                          // (line 40)
    static size_t s_entryCount;                                     // (line 41)
    static std::unordered_map<uintptr_t, std::string> s_vtableToName; // (line 42) — vtable addr -> class name

private:
    UiProbe() = default;  // (line 45)

    std::vector<void*> m_hookedTargets;  // (line 47) — installed MinHook targets for Uninstall

    // Per-index template detour — one instantiation per hook slot.
    // Calls ProbeCommonSEH with its compile-time Index.
    template<size_t Index>
    static void __fastcall ProbeDetourN(void* thisPtr, void* param2);  // (line 51)

    // SEH wrapper: calls ProbeCommonInner inside __try/__except.
    // Must contain no C++ objects (no destructors) to be SEH-safe.
    static void ProbeCommonSEH(size_t hookIndex, void* thisPtr, void* param2);  // (line 54)

    // Inner logic: may use C++ objects, string operations, MemoryInspector calls.
    static void ProbeCommonInner(size_t hookIndex, void* thisPtr, void* param2);  // (line 57)

    // Table of function pointers to ProbeDetourN<0..MAX_HOOKS-1>, initialized once.
    static void* s_detourTable[MAX_HOOKS];  // (line 60)
    static bool s_detourTableInit;          // (line 61)
    static void InitDetourTable();          // (line 62)

    // Helper to expand the index sequence and populate s_detourTable.
    template<size_t... Is>
    static void InitDetourTableImpl(std::index_sequence<Is...>);  // (line 65)

    static std::unordered_map<uintptr_t, bool> s_seenVtables;  // (line 69) — dedup: log each vtable once
}

// Template detour body defined inline in the header.
// Calls ProbeCommonSEH with the compile-time Index, which dispatches to s_entries[Index].
template<size_t Index>
void __fastcall UiProbe::ProbeDetourN(void* thisPtr, void* param2);  // (line 73-78)
