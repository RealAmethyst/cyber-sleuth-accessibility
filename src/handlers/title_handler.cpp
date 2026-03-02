// CUiTitle accessibility handler -- title screen menu.
//
// Uses a global vtable hook so no instance discovery is needed.
// Captures `this` pointer and registers it with MemoryInspector
// for runtime offset discovery.

#include "handlers/title_handler.h"
#include "memory_inspector.h"
#include "offsets.h"
#include "logger.h"

#include <modloader/utils.h>
#include <windows.h>

TitleHandler* TitleHandler::Get()
{
    static TitleHandler instance;
    return &instance;
}

void TitleHandler::InstallGlobal()
{
    if (m_installed) return;

    uintptr_t base = reinterpret_cast<uintptr_t>(getBaseOffset());
    uintptr_t vtableAddr = base + Offsets::VTABLE_CUiTitle;

    s_originalUpdate = reinterpret_cast<UpdateFunc>(
        HookVTableByAddress(vtableAddr, 2, (void*)&HookedUpdate));

    if (s_originalUpdate) {
        m_installed = true;
        Logger_Log("Title", "Global vtable[2] hook installed @ 0x%llx",
                   (unsigned long long)vtableAddr);
    } else {
        Logger_Log("Title", "FAILED to hook CUiTitle vtable");
    }
}

void TitleHandler::Uninstall()
{
    if (!m_installed) return;

    uintptr_t base = reinterpret_cast<uintptr_t>(getBaseOffset());
    uintptr_t vtableAddr = base + Offsets::VTABLE_CUiTitle;

    if (s_originalUpdate) {
        HookVTableByAddress(vtableAddr, 2, (void*)s_originalUpdate);
    }

    s_originalUpdate = nullptr;
    m_lastThisPtr = nullptr;
    m_installed = false;

    MemoryInspector::Get()->ClearPointer("CUiTitle");
    Logger_Log("Title", "VTable hook uninstalled");
}

void __fastcall TitleHandler::HookedUpdate(void* thisPtr)
{
    // Call original update first
    if (s_originalUpdate) {
        s_originalUpdate(thisPtr);
    }

    // Track instance and register with memory inspector
    auto* handler = Get();
    if (handler->m_lastThisPtr != thisPtr) {
        handler->m_lastThisPtr = thisPtr;
        Logger_Log("Title", "CUiTitle instance captured: %p", thisPtr);
    }

    MemoryInspector::Get()->SetActivePointer("CUiTitle", thisPtr);
}

void TitleHandler::OnFrame()
{
    // All work happens in HookedUpdate -- nothing needed here.
}
