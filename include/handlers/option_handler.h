#pragma once

#include "hooks.h"
#include "memory_inspector.h"
#include "logger.h"

#include <modloader/utils.h>
#include <MinHook.h>
#include <windows.h>

#include <atomic>
#include <cstdint>
#include <string>
#include <unordered_map>

// Handles accessibility for the options menu (CUiOption).
//
// Uses TextCapture help_message events to detect cursor movement and
// value row ID filtering to extract the correct value per setting.
//
// Speech format: "Name: Value, Description, N of M"
//
// STATE HANDLER HOOKS (not tick hook):
//   CUiOption's tick (vtable[3]) fires every frame from game start, even when
//   the menu isn't visible. The tick dispatches through a function table at
//   this+0x10 indexed by the sub-screen state at this+0x138. State 0 is idle
//   (no-op). States 5/13/17 are the interactive menu states.
//
//   Instead of hooking the always-running tick, we hook the individual state
//   handler functions that only fire when the menu is actually interactive:
//     State 5:  FUN_140492da0 — main options tab
//     State 13: FUN_140493a20 — button settings
//     State 17: FUN_140493c50 — graphic options
//
//   These hooks have zero overhead when the menu is closed (the functions
//   simply aren't called), matching how CUiTitle/CUiMainMenu behave.
//
//   A grace period handles transition states (e.g., opening animation,
//   yes/no dialog) where none of the three hooked states fire temporarily.
class OptionHandler : public IFrameHandler
{
public:
    static OptionHandler* Get();

    void Install();
    void Uninstall();
    bool IsInstalled() const { return m_installed; }

    void OnFrame() override;
    void OnFrameInner(void* thisPtr);
    void OnScreenClosed();

    const char* GetHandlerName() const { return "Option"; }

private:
    OptionHandler() = default;

    bool m_installed = false;
    bool m_screenActive = false;

    bool m_menuOpened = false;
    int m_lastHelpId = -1;
    int m_lastSubScreen = -1;
    bool m_settling = false;
    int m_settleFrames = 0;
    bool m_resuming = false;
    int m_preTransitionSubScreen = -1;  // sub-screen before entering transition/dialog
    bool m_inGameIds = false;  // true when in-game help IDs (2200-2209) are active
    bool m_yesNoShown = false; // true when YesNoHandler dialog is active

    std::unordered_map<int, int> m_rememberedHelpId;
    std::unordered_map<int, std::string> m_cachedValue;  // last known value per help ID

    // --- Three-hook infrastructure ---
    // One hook per stable interactive state, sharing the same thisPtr/tickFired.
    static constexpr int HOOK_COUNT = 3;

    struct HookSlot {
        void* target = nullptr;
        using StateFunc = void(__fastcall*)(void*, void*);
        StateFunc original = nullptr;
    };

    static inline HookSlot s_hooks[HOOK_COUNT];
    static inline std::atomic<void*> s_thisPtr{nullptr};
    static inline std::atomic<bool> s_tickFired{false};

    // Per-slot detour — captures thisPtr and calls original.
    template<int N>
    static void __fastcall HookedState(void* thisPtr, void* param2)
    {
        if (s_hooks[N].original) s_hooks[N].original(thisPtr, param2);
        if (thisPtr) {
            s_thisPtr.store(thisPtr, std::memory_order_relaxed);
            s_tickFired.store(true, std::memory_order_relaxed);
        }
    }

    // SEH wrapper
    static void OnFrameSEH(OptionHandler* handler, void* thisPtr);

    // --- Announcement helpers ---
    static const std::unordered_map<int, int>& GetHelpToLabelMap();
    static int GetItemCount(int subScreen);

    std::string LookupSettingName(int helpId);
    std::string FindValueText();
    std::string AppendPosition(const std::string& base, int helpId);
    std::string BuildAnnouncement(int helpId, const std::string& helpText,
                                  const std::string& valueText);
    std::string BuildInitialAnnouncement(void* thisPtr, const std::string& valueText);
};
