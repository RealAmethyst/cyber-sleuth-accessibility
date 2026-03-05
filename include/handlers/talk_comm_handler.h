#pragma once

#include "hooks.h"
#include "logger.h"

#include <MinHook.h>
#include <modloader/utils.h>
#include <windows.h>

#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>

// Handles accessibility for CUiTalkCommunication — chat room / EDEN comm scenes.
//
// Diagnostic hooks to find which UI class drives the chat room:
// 1. CUiTalkCommunication message handler (0x40C770) — existing
// 2. CUiEden tick (0x414110)
// 3. CUiDigiline tick (0x4456C0)
// 4. CUiFieldDigiLine tick (0x3E0CF0)
class TalkCommHandler : public IFrameHandler
{
public:
    static TalkCommHandler* Get();

    void Install();
    void Uninstall();
    void OnFrame() override;

private:
    TalkCommHandler() = default;

    bool m_installed = false;
    std::string m_announcedSpeaker;

    // Thread-safe captured data from the hook
    struct CapturedData {
        std::string speaker;
        std::string text;
        int caseNum = 0;
        bool pending = false;
    };
    std::mutex m_mutex;
    CapturedData m_captured;

    // Hook 1: CUiTalkCommunication message handler (0x40C770)
    using MsgHandlerFunc = uint64_t(__fastcall*)(void* thisPtr, int param2, int caseNum, void* data);
    static inline MsgHandlerFunc s_originalMsgHandler = nullptr;
    static inline void* s_msgHookTarget = nullptr;

    static uint64_t __fastcall HookedMsgHandler(void* thisPtr, int param2, int caseNum, void* data);
    static std::string ExtractSSO(uint8_t* ssoPtr);

    // Diagnostic tick hooks — just log when they fire (rate-limited)
    using TickFunc = void(__fastcall*)(void* thisPtr, void* param2);

    // Hook 2: CUiEden tick (0x414110)
    static inline TickFunc s_originalEdenTick = nullptr;
    static inline void* s_edenTickTarget = nullptr;
    static inline std::atomic<int> s_edenTickCount{0};
    static void __fastcall HookedEdenTick(void* thisPtr, void* param2);

    // Hook 3: CUiDigiline tick (0x4456C0)
    static inline TickFunc s_originalDigilineTick = nullptr;
    static inline void* s_digilineTickTarget = nullptr;
    static inline std::atomic<int> s_digilineTickCount{0};
    static void __fastcall HookedDigilineTick(void* thisPtr, void* param2);

    // Hook 4: CUiFieldDigiLine tick (0x3E0CF0)
    static inline TickFunc s_originalFieldDigilineTick = nullptr;
    static inline void* s_fieldDigilineTickTarget = nullptr;
    static inline std::atomic<int> s_fieldDigilineTickCount{0};
    static void __fastcall HookedFieldDigilineTick(void* thisPtr, void* param2);

    // Rate-limited logging state
    int m_logTimer = 0;
};
