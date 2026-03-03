// File: src/ui_probe.cpp
// Purpose: Discovery/diagnostic tool that hooks vtable[3] (tick) on multiple
//          CUi* classes simultaneously via MinHook function-level hooks.
//          Deduplicates by target function address so shared tick functions are
//          only hooked once. Each unique function gets its own template-
//          instantiated detour (compile-time index) for direct trampoline lookup.
//          Identifies active objects by vtable address and registers them with
//          MemoryInspector for F5 dumps.
//          Not used in production — used during RE/offset discovery phases.

// --- Static storage ---

UiProbe::HookEntry UiProbe::s_entries[MAX_HOOKS]  (line 23)
size_t UiProbe::s_entryCount  (line 24)
std::unordered_map<uintptr_t, std::string> UiProbe::s_vtableToName  (line 25)
std::unordered_map<uintptr_t, bool> UiProbe::s_seenVtables  (line 26)
void* UiProbe::s_detourTable[MAX_HOOKS]  (line 29)
bool UiProbe::s_detourTableInit  (line 30)

// Populates s_detourTable with template-instantiated detour function pointers
// (one per index up to MAX_HOOKS). Called once before Install().
void UiProbe::InitDetourTable()  (line 32)

// Singleton accessor.
UiProbe* UiProbe::Get()  (line 43)

// Reads vtable[3] for each ClassInfo entry, deduplicates by function address,
// then installs one MinHook per unique function using the pre-built detour table.
// Logs class name, vtable address, and tick function address for each entry.
void UiProbe::Install(const std::vector<ClassInfo>& classes)  (line 49)

// Disables and removes all installed function hooks, resets all static state.
void UiProbe::Uninstall()  (line 143)

// SEH wrapper for per-tick logic. If a hook has previously faulted (entry.disabled),
// still calls the original via SEH but skips probe logic. On new faults, logs
// once and sets disabled=true to prevent spam.
void UiProbe::ProbeCommonSEH(size_t hookIndex, void* thisPtr, void* param2)  (line 165)

// Inner per-tick logic (called from ProbeCommonSEH). Calls original via trampoline,
// reads vtable pointer from thisPtr to identify the class, logs ACTIVE message on
// first sighting of each unique vtable, and registers pointer with MemoryInspector.
void UiProbe::ProbeCommonInner(size_t hookIndex, void* thisPtr, void* param2)  (line 199)
