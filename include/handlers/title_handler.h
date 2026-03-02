#pragma once

#include "hooks.h"
#include <cstdint>

// Handles accessibility for the title screen (CUiTitle).
//
// Uses a global vtable hook (patches vtable[2] in .rdata) so ALL instances
// of CUiTitle trigger our hook -- no instance discovery needed.
//
// Captures `this` pointer and feeds it to MemoryInspector for offset discovery.
class TitleHandler : public IFrameHandler
{
public:
    static TitleHandler* Get();

    // Install global vtable hook (patches vtable in .rdata, no instance needed)
    void InstallGlobal();
    void Uninstall();

    bool IsInstalled() const { return m_installed; }

    // IFrameHandler
    void OnFrame() override;

private:
    TitleHandler() = default;

    bool m_installed = false;
    void* m_lastThisPtr = nullptr;

    using UpdateFunc = void(__fastcall*)(void* thisPtr);
    static inline UpdateFunc s_originalUpdate = nullptr;

    static void __fastcall HookedUpdate(void* thisPtr);
};
