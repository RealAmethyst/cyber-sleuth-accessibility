// UiProbe -- discovery tool for CUi* class activity.
//
// Hooks tick functions (vtable[3]) via MinHook (function-level, not vtable patching)
// so calls are intercepted regardless of how the game dispatches them.
//
// Each unique function gets its own template-instantiated detour, so the
// trampoline lookup is direct (no fragile runtime vtable re-reads).

#include "ui_probe.h"
#include "memory_inspector.h"
#include "speech_manager.h"
#include "offsets.h"
#include "logger.h"

#include <modloader/utils.h>
#include <MinHook.h>
#include <windows.h>

// ============================================================
// Static storage
// ============================================================

UiProbe::HookEntry UiProbe::s_entries[MAX_HOOKS] = {};
size_t UiProbe::s_entryCount = 0;
std::unordered_map<uintptr_t, std::string> UiProbe::s_vtableToName;
std::unordered_map<uintptr_t, bool> UiProbe::s_seenVtables;

// Runtime detour table (initialized once)
void* UiProbe::s_detourTable[MAX_HOOKS] = {};
bool UiProbe::s_detourTableInit = false;

void UiProbe::InitDetourTable()
{
    if (s_detourTableInit) return;
    InitDetourTableImpl(std::make_index_sequence<MAX_HOOKS>{});
    s_detourTableInit = true;
}

// ============================================================
// UiProbe implementation
// ============================================================

UiProbe* UiProbe::Get()
{
    static UiProbe instance;
    return &instance;
}

void UiProbe::Install(const std::vector<ClassInfo>& classes)
{
    InitDetourTable();

    uintptr_t base = reinterpret_cast<uintptr_t>(getBaseOffset());

    Logger_Log("Probe", "Installing probes for %zu CUi classes (base: %p)",
               classes.size(), (void*)base);

    // Step 1: Read vtable[3] for each class, build dedup map
    std::unordered_map<uintptr_t, std::vector<const char*>> funcToClasses;

    for (auto& cls : classes) {
        uintptr_t vtableAddr = base + cls.vtableRva;

        uintptr_t* vtableSlot3 = reinterpret_cast<uintptr_t*>(
            vtableAddr + Offsets::VTABLE_TICK_INDEX * sizeof(uintptr_t));

        MEMORY_BASIC_INFORMATION mbi = {};
        if (!VirtualQuery(vtableSlot3, &mbi, sizeof(mbi)) ||
            mbi.State != MEM_COMMIT) {
            Logger_Log("Probe", "  %s: vtable @ 0x%llx not readable, skipping",
                       cls.name, (unsigned long long)vtableAddr);
            continue;
        }

        uintptr_t funcAddr = *vtableSlot3;

        s_vtableToName[vtableAddr] = cls.name;
        funcToClasses[funcAddr].push_back(cls.name);

        Logger_Log("Probe", "  %s: vtable=0x%llx, tick[3]=0x%llx",
                   cls.name, (unsigned long long)vtableAddr,
                   (unsigned long long)funcAddr);
    }

    // Step 2: Hook each unique function with its own detour
    s_entryCount = 0;

    for (auto& [funcAddr, classNames] : funcToClasses) {
        if (s_entryCount >= MAX_HOOKS) {
            Logger_Log("Probe", "  WARNING: hit MAX_HOOKS (%zu), skipping remaining", MAX_HOOKS);
            break;
        }

        size_t idx = s_entryCount;
        void* target = reinterpret_cast<void*>(funcAddr);
        void* detour = s_detourTable[idx];
        void* original = nullptr;

        MH_STATUS status = MH_CreateHook(target, detour, &original);
        if (status != MH_OK) {
            std::string shared;
            for (auto& n : classNames) {
                if (!shared.empty()) shared += ", ";
                shared += n;
            }
            Logger_Log("Probe", "  MH_CreateHook failed for 0x%llx: %d (classes: %s)",
                       (unsigned long long)funcAddr, status, shared.c_str());
            continue;
        }

        status = MH_EnableHook(target);
        if (status != MH_OK) {
            Logger_Log("Probe", "  MH_EnableHook failed for 0x%llx: %d",
                       (unsigned long long)funcAddr, status);
            MH_RemoveHook(target);
            continue;
        }

        // Fill the entry BEFORE incrementing count (detour might fire immediately
        // on another thread, though unlikely during DLL init)
        s_entries[idx].trampoline = original;
        s_entries[idx].targetFunc = funcAddr;

        // Build class names string (into fixed buffer)
        std::string shared;
        for (auto& n : classNames) {
            if (!shared.empty()) shared += ", ";
            shared += n;
        }
        strncpy_s(s_entries[idx].classNames, shared.c_str(), _TRUNCATE);

        m_hookedTargets.push_back(target);
        s_entryCount++;

        Logger_Log("Probe", "  Hooked[%zu] 0x%llx -> trampoline %p (classes: %s)",
                   idx, (unsigned long long)funcAddr, original, shared.c_str());
    }

    Logger_Log("Probe", "Probe install complete: %zu classes, %zu unique functions hooked",
               classes.size(), s_entryCount);
}

void UiProbe::Uninstall()
{
    for (auto* target : m_hookedTargets) {
        MH_DisableHook(target);
        MH_RemoveHook(target);
    }

    Logger_Log("Probe", "Uninstalled %zu function hooks", m_hookedTargets.size());

    m_hookedTargets.clear();
    s_entryCount = 0;
    for (size_t i = 0; i < MAX_HOOKS; i++) {
        s_entries[i] = {};
    }
    s_vtableToName.clear();
    s_seenVtables.clear();
}

// ============================================================
// SEH wrapper — NO C++ objects, safe for __try/__except
// ============================================================

void UiProbe::ProbeCommonSEH(size_t hookIndex, void* thisPtr, void* param2)
{
    // Fast path: if this hook already faulted, just call the original and bail
    if (hookIndex < s_entryCount && s_entries[hookIndex].disabled) {
        // Still call the original so game logic runs
        __try {
            if (s_entries[hookIndex].trampoline) {
                auto original = reinterpret_cast<void(__fastcall*)(void*, void*)>(
                    s_entries[hookIndex].trampoline);
                original(thisPtr, param2);
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            // Original also faults — nothing we can do, just skip silently
        }
        return;
    }

    __try {
        ProbeCommonInner(hookIndex, thisPtr, param2);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        // Log ONCE, then disable this hook to stop spam
        if (hookIndex < s_entryCount) {
            Logger_Log("Probe", "EXCEPTION in hook[%zu] '%s' (this=%p, code=0x%08lx) — disabling probe",
                       hookIndex, s_entries[hookIndex].classNames,
                       thisPtr, GetExceptionCode());
            s_entries[hookIndex].disabled = true;
        }
    }
}

// ============================================================
// Inner logic — may use C++ objects freely
// ============================================================

void UiProbe::ProbeCommonInner(size_t hookIndex, void* thisPtr, void* param2)
{
    if (!thisPtr) return;
    if (hookIndex >= s_entryCount) return;

    auto& entry = s_entries[hookIndex];

    // Call original function via trampoline (direct lookup — no vtable re-read)
    if (entry.trampoline) {
        auto original = reinterpret_cast<void(__fastcall*)(void*, void*)>(entry.trampoline);
        original(thisPtr, param2);
    }

    // Read vtable pointer for class identification
    uintptr_t vtable = *reinterpret_cast<uintptr_t*>(thisPtr);

    // Identify the class from vtable address
    auto nameIt = s_vtableToName.find(vtable);
    std::string className = (nameIt != s_vtableToName.end())
        ? nameIt->second
        : "UnknownCUi";

    // Log first time we see each unique vtable
    if (!s_seenVtables.count(vtable)) {
        s_seenVtables[vtable] = true;
        Logger_Log("Probe", "ACTIVE: %s (this=%p, vtable=0x%llx, hook[%zu]=%s)",
                   className.c_str(), thisPtr,
                   (unsigned long long)vtable,
                   hookIndex, entry.classNames);
    }

    // Register with MemoryInspector for F5 dumps
    MemoryInspector::Get()->SetActivePointer(className, thisPtr);
}
