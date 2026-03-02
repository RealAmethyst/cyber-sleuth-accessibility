// UiProbe -- discovery tool for CUi* class activity.
//
// Hooks tick functions (vtable[3]) via MinHook (function-level, not vtable patching)
// so calls are intercepted regardless of how the game dispatches them.
// Identifies classes by checking this->vtable against known addresses.
//
// vtable[3] is the per-frame tick/update (confirmed via Ghidra decompilation).
// vtable[2] is cleanup/teardown — only fires on screen transitions.

#include "ui_probe.h"
#include "memory_inspector.h"
#include "speech_manager.h"
#include "offsets.h"
#include "logger.h"

#include <modloader/utils.h>
#include <MinHook.h>
#include <windows.h>

// ============================================================
// Static state accessible from the detour
// ============================================================

// Absolute vtable address -> class name
static std::unordered_map<uintptr_t, std::string> s_vtableToName;

// Function address (target) -> trampoline pointer
static std::unordered_map<uintptr_t, void*> s_funcToOriginal;

// Tracks first-seen instances to avoid log spam
static std::unordered_map<uintptr_t, bool> s_seenVtables;

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
    uintptr_t base = reinterpret_cast<uintptr_t>(getBaseOffset());

    Logger_Log("Probe", "Installing probes for %zu CUi classes (base: %p)",
               classes.size(), (void*)base);

    // Step 1: Read vtable[2] for each class, build maps
    // funcAddr -> list of classes using that function
    std::unordered_map<uintptr_t, std::vector<const char*>> funcToClasses;

    for (auto& cls : classes) {
        uintptr_t vtableAddr = base + cls.vtableRva;

        // Read vtable[3] -- the per-frame tick function pointer
        uintptr_t* vtableSlot3 = reinterpret_cast<uintptr_t*>(
            vtableAddr + Offsets::VTABLE_TICK_INDEX * sizeof(uintptr_t));

        // Verify readability
        MEMORY_BASIC_INFORMATION mbi = {};
        if (!VirtualQuery(vtableSlot3, &mbi, sizeof(mbi)) ||
            mbi.State != MEM_COMMIT) {
            Logger_Log("Probe", "  %s: vtable @ 0x%llx not readable, skipping",
                       cls.name, (unsigned long long)vtableAddr);
            continue;
        }

        uintptr_t funcAddr = *vtableSlot3;

        // Store vtable -> name mapping (absolute vtable address)
        m_vtableToName[vtableAddr] = cls.name;

        // Track which functions are shared
        funcToClasses[funcAddr].push_back(cls.name);

        Logger_Log("Probe", "  %s: vtable=0x%llx, tick[3]=0x%llx",
                   cls.name, (unsigned long long)vtableAddr,
                   (unsigned long long)funcAddr);
    }

    // Copy to static map for detour access
    s_vtableToName = m_vtableToName;

    // Step 2: Hook each unique function
    size_t hooked = 0;
    for (auto& [funcAddr, classNames] : funcToClasses) {
        // Skip if already hooked (shouldn't happen, but safety)
        if (m_funcToOriginal.count(funcAddr)) continue;

        void* target = reinterpret_cast<void*>(funcAddr);
        void* original = nullptr;

        MH_STATUS status = MH_CreateHook(target, (void*)&ProbeDetour, &original);
        if (status != MH_OK) {
            Logger_Log("Probe", "  MH_CreateHook failed for 0x%llx: %d (shared by: %s)",
                       (unsigned long long)funcAddr, status, classNames[0]);
            continue;
        }

        status = MH_EnableHook(target);
        if (status != MH_OK) {
            Logger_Log("Probe", "  MH_EnableHook failed for 0x%llx: %d",
                       (unsigned long long)funcAddr, status);
            MH_RemoveHook(target);
            continue;
        }

        m_funcToOriginal[funcAddr] = original;
        m_hookedTargets.push_back(target);

        // Log which classes share this function
        std::string shared;
        for (auto& n : classNames) {
            if (!shared.empty()) shared += ", ";
            shared += n;
        }
        Logger_Log("Probe", "  Hooked 0x%llx -> trampoline %p (classes: %s)",
                   (unsigned long long)funcAddr, original, shared.c_str());
        hooked++;
    }

    // Copy to static map for detour access
    s_funcToOriginal = m_funcToOriginal;

    Logger_Log("Probe", "Probe install complete: %zu classes, %zu unique functions hooked",
               classes.size(), hooked);
}

void UiProbe::Uninstall()
{
    for (auto* target : m_hookedTargets) {
        MH_DisableHook(target);
        MH_RemoveHook(target);
    }

    Logger_Log("Probe", "Uninstalled %zu function hooks", m_hookedTargets.size());

    m_hookedTargets.clear();
    m_funcToOriginal.clear();
    m_vtableToName.clear();
    s_funcToOriginal.clear();
    s_vtableToName.clear();
    s_seenVtables.clear();
}

// ============================================================
// Detour -- called for ALL probed update functions
// ============================================================

void __fastcall UiProbe::ProbeDetour(void* thisPtr, void* param2)
{
    if (!thisPtr) return;

    // Read vtable pointer (first qword of object)
    uintptr_t vtable = *reinterpret_cast<uintptr_t*>(thisPtr);

    // Identify the class from vtable address
    auto nameIt = s_vtableToName.find(vtable);
    std::string className = (nameIt != s_vtableToName.end())
        ? nameIt->second
        : "UnknownCUi";

    // Call original function via trampoline
    // Look up the function address from vtable[3] (tick) to find the right trampoline
    uintptr_t funcAddr = reinterpret_cast<uintptr_t*>(vtable)[Offsets::VTABLE_TICK_INDEX];
    auto origIt = s_funcToOriginal.find(funcAddr);
    if (origIt != s_funcToOriginal.end()) {
        auto original = reinterpret_cast<void(__fastcall*)(void*, void*)>(origIt->second);
        original(thisPtr, param2);
    }

    // Log first time we see each class
    if (!s_seenVtables.count(vtable)) {
        s_seenVtables[vtable] = true;
        Logger_Log("Probe", "ACTIVE: %s (this=%p, vtable=0x%llx)",
                   className.c_str(), thisPtr,
                   (unsigned long long)vtable);
    }

    // Register with MemoryInspector for F5 dumps
    MemoryInspector::Get()->SetActivePointer(className, thisPtr);
}
