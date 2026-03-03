#pragma once

// CRTP base class for tick-based UI handlers.
//
// Eliminates duplicated tick hook infrastructure across handlers.
// Each handler inherits TickHandler<Derived> and provides:
//
//   const char* GetHandlerName() const;     — e.g. "Title", "MainMenu"
//   uintptr_t GetTickRVA() const;           — tick function RVA from Offsets
//   void OnFrameInner(void* thisPtr);       — handler-specific per-frame logic
//   void OnScreenClosed();                  — reset handler state when tick stops
//
// The base provides: Install(), Uninstall(), OnFrame(), HookedTick(),
// SEH wrapping, MemoryInspector integration, and tick-fired detection.
//
// Handlers that need custom OnFrame logic (e.g. ScenarioSelectHandler)
// can override OnFrame() — it's virtual, not final.

#include "hooks.h"
#include "memory_inspector.h"
#include "logger.h"

#include <modloader/utils.h>
#include <MinHook.h>
#include <windows.h>

#include <atomic>
#include <cstdint>

template<typename Derived>
class TickHandler : public IFrameHandler
{
public:
    bool IsInstalled() const { return m_installed; }

    void Install()
    {
        if (m_installed) return;

        auto* self = static_cast<Derived*>(this);
        uintptr_t base = reinterpret_cast<uintptr_t>(getBaseOffset());
        uintptr_t rva = self->GetTickRVA();

        s_hookTarget = reinterpret_cast<void*>(base + rva);

        MH_STATUS status = MH_CreateHook(s_hookTarget, (void*)&HookedTick,
                                          reinterpret_cast<void**>(&s_originalTick));
        if (status != MH_OK) {
            Logger_Log(self->GetHandlerName(),
                       "MH_CreateHook failed for tick @ RVA 0x%llx: %d",
                       (unsigned long long)rva, status);
            return;
        }

        status = MH_EnableHook(s_hookTarget);
        if (status != MH_OK) {
            Logger_Log(self->GetHandlerName(), "MH_EnableHook failed: %d", status);
            MH_RemoveHook(s_hookTarget);
            return;
        }

        m_installed = true;
        Logger_Log(self->GetHandlerName(),
                   "MinHook installed on tick (RVA 0x%llx)",
                   (unsigned long long)rva);
    }

    void Uninstall()
    {
        if (!m_installed) return;

        auto* self = static_cast<Derived*>(this);

        if (s_hookTarget) {
            MH_DisableHook(s_hookTarget);
            MH_RemoveHook(s_hookTarget);
        }

        s_originalTick = nullptr;
        s_hookTarget = nullptr;
        s_thisPtr.store(nullptr, std::memory_order_relaxed);
        s_tickFired.store(false, std::memory_order_relaxed);
        m_screenActive = false;
        m_installed = false;

        MemoryInspector::Get()->ClearPointer(self->GetHandlerName());
        Logger_Log(self->GetHandlerName(), "MinHook uninstalled");
    }

    // Default OnFrame: tick-fired detection, SEH wrapper, MemoryInspector.
    // Override in handlers that need custom pre-tick logic (e.g. ScenarioSelect).
    void OnFrame() override
    {
        if (!m_installed) return;

        bool tickFired = s_tickFired.exchange(false, std::memory_order_relaxed);

        if (!tickFired) {
            if (m_screenActive) {
                auto* self = static_cast<Derived*>(this);
                Logger_Log(self->GetHandlerName(), "Screen closed (tick stopped firing)");
                self->OnScreenClosed();
                m_screenActive = false;
            }
            return;
        }

        void* thisPtr = s_thisPtr.load(std::memory_order_relaxed);
        if (!thisPtr) return;

        m_screenActive = true;
        auto* self = static_cast<Derived*>(this);
        MemoryInspector::Get()->SetActivePointer(self->GetHandlerName(), thisPtr);
        CallWithSEH(thisPtr);
    }

protected:
    TickHandler() = default;

    bool m_installed = false;
    bool m_screenActive = false;

    // Access the captured this pointer (for handlers that override OnFrame)
    void* LoadThisPtr() const
    {
        return s_thisPtr.load(std::memory_order_relaxed);
    }

    bool ExchangeTickFired()
    {
        return s_tickFired.exchange(false, std::memory_order_relaxed);
    }

    // Call OnFrameInner wrapped in SEH (for handlers that override OnFrame)
    void CallWithSEH(void* thisPtr)
    {
        OnFrameSEH(static_cast<Derived*>(this), thisPtr);
    }

private:
    // Per-template-instantiation statics — each handler class gets its own set.
    static inline std::atomic<void*> s_thisPtr{nullptr};
    static inline std::atomic<bool> s_tickFired{false};

    using TickFunc = void(__fastcall*)(void* thisPtr, void* param2);
    static inline TickFunc s_originalTick = nullptr;
    static inline void* s_hookTarget = nullptr;

    // Minimal tick detour — captures this pointer only.
    static void __fastcall HookedTick(void* thisPtr, void* param2)
    {
        if (s_originalTick) {
            s_originalTick(thisPtr, param2);
        }
        if (thisPtr) {
            s_thisPtr.store(thisPtr, std::memory_order_relaxed);
            s_tickFired.store(true, std::memory_order_relaxed);
        }
    }

    // SEH wrapper — separate function, no C++ objects with destructors.
    static void OnFrameSEH(Derived* handler, void* thisPtr)
    {
        __try {
            handler->OnFrameInner(thisPtr);
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            Logger_Log(handler->GetHandlerName(),
                       "EXCEPTION in OnFrame (this=%p, code=0x%08lx)",
                       thisPtr, GetExceptionCode());
        }
    }
};
