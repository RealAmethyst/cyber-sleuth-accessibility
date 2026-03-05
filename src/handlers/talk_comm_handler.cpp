// CUiTalkCommunication accessibility handler — chat room / EDEN comm scenes.
//
// Diagnostic hooks to find which UI class drives the intro chat room:
// 1. CUiTalkCommunication message handler (0x40C770)
// 2. CUiEden tick (0x414110)
// 3. CUiDigiline tick (0x4456C0)
// 4. CUiFieldDigiLine tick (0x3E0CF0)

#include "handlers/talk_comm_handler.h"
#include "speech_manager.h"
#include "offsets.h"
#include "logger.h"

#include <modloader/utils.h>

static constexpr uintptr_t RVA_TALKCOMM_MSGHANDLER    = 0x40C770;
static constexpr uintptr_t RVA_EDEN_TICK              = 0x414110;
static constexpr uintptr_t RVA_DIGILINE_TICK          = 0x4456C0;
static constexpr uintptr_t RVA_FIELD_DIGILINE_TICK    = 0x3E0CF0;

TalkCommHandler* TalkCommHandler::Get()
{
    static TalkCommHandler instance;
    return &instance;
}

std::string TalkCommHandler::ExtractSSO(uint8_t* ssoPtr)
{
    size_t capacity = *reinterpret_cast<size_t*>(ssoPtr + 0x18);
    const char* text;

    if (capacity >= 16) {
        text = *reinterpret_cast<const char**>(ssoPtr);
    } else {
        text = reinterpret_cast<const char*>(ssoPtr);
    }

    if (!text || text[0] == '\0') return "";

    size_t size = *reinterpret_cast<size_t*>(ssoPtr + 0x10);
    if (size == 0 || size > 4096) return "";

    return std::string(text, size);
}

// Hook 1: CUiTalkCommunication message handler
uint64_t __fastcall TalkCommHandler::HookedMsgHandler(void* thisPtr, int param2, int caseNum, void* data)
{
    Logger_Log("TalkComm", "MsgHandler fired: case=%d param2=%d data=%p thisPtr=%p",
               caseNum, param2, data, thisPtr);

    if (caseNum == 2 && data) {
        auto* param4 = reinterpret_cast<uint8_t*>(data);
        std::string speaker = ExtractSSO(param4 + 8);
        std::string text = ExtractSSO(param4 + 40);

        Logger_Log("TalkComm", "  Case 2: speaker='%s' text='%s'",
                   speaker.c_str(), text.c_str());

        if (!text.empty() || !speaker.empty()) {
            auto* handler = TalkCommHandler::Get();
            std::lock_guard<std::mutex> lock(handler->m_mutex);
            handler->m_captured.speaker = speaker;
            handler->m_captured.text = text;
            handler->m_captured.caseNum = 2;
            handler->m_captured.pending = true;
        }
    }

    if (caseNum == 5 && data) {
        auto* param4 = reinterpret_cast<uint8_t*>(data);
        std::string text = ExtractSSO(param4 + 8);

        Logger_Log("TalkComm", "  Case 5: text='%s'", text.c_str());

        if (!text.empty()) {
            auto* handler = TalkCommHandler::Get();
            std::lock_guard<std::mutex> lock(handler->m_mutex);
            handler->m_captured.text = text;
            handler->m_captured.caseNum = 5;
            handler->m_captured.pending = true;
        }
    }

    if (s_originalMsgHandler) {
        return s_originalMsgHandler(thisPtr, param2, caseNum, data);
    }
    return 0;
}

// Diagnostic tick hooks — just log that they're firing
void __fastcall TalkCommHandler::HookedEdenTick(void* thisPtr, void* param2)
{
    int count = s_edenTickCount.fetch_add(1, std::memory_order_relaxed);
    if (count == 0 || (count % 300 == 0)) {
        Logger_Log("DiagTick", "CUiEden tick #%d thisPtr=%p", count, thisPtr);
    }
    s_originalEdenTick(thisPtr, param2);
}

void __fastcall TalkCommHandler::HookedDigilineTick(void* thisPtr, void* param2)
{
    int count = s_digilineTickCount.fetch_add(1, std::memory_order_relaxed);
    if (count == 0 || (count % 300 == 0)) {
        Logger_Log("DiagTick", "CUiDigiline tick #%d thisPtr=%p", count, thisPtr);
    }
    s_originalDigilineTick(thisPtr, param2);
}

void __fastcall TalkCommHandler::HookedFieldDigilineTick(void* thisPtr, void* param2)
{
    int count = s_fieldDigilineTickCount.fetch_add(1, std::memory_order_relaxed);
    if (count == 0 || (count % 300 == 0)) {
        Logger_Log("DiagTick", "CUiFieldDigiLine tick #%d thisPtr=%p", count, thisPtr);
    }
    s_originalFieldDigilineTick(thisPtr, param2);
}

// Helper to install a single MinHook
static bool InstallHook(const char* name, uintptr_t base, uintptr_t rva, void* hookFunc, void** original, void** target)
{
    *target = reinterpret_cast<void*>(base + rva);
    MH_STATUS status = MH_CreateHook(*target, hookFunc, original);
    if (status == MH_OK) {
        status = MH_EnableHook(*target);
        if (status == MH_OK) {
            Logger_Log("TalkComm", "%s installed (RVA 0x%llx)", name, (unsigned long long)rva);
            return true;
        }
        Logger_Log("TalkComm", "%s MH_EnableHook failed: %d", name, status);
        MH_RemoveHook(*target);
    } else {
        Logger_Log("TalkComm", "%s MH_CreateHook failed: %d", name, status);
    }
    *target = nullptr;
    return false;
}

void TalkCommHandler::Install()
{
    if (m_installed) return;

    uintptr_t base = reinterpret_cast<uintptr_t>(getBaseOffset());

    // Hook 1: CUiTalkCommunication message handler
    InstallHook("MsgHandler", base, RVA_TALKCOMM_MSGHANDLER,
                (void*)&HookedMsgHandler, reinterpret_cast<void**>(&s_originalMsgHandler), &s_msgHookTarget);

    // Hook 2: CUiEden tick
    InstallHook("CUiEden tick", base, RVA_EDEN_TICK,
                (void*)&HookedEdenTick, reinterpret_cast<void**>(&s_originalEdenTick), &s_edenTickTarget);

    // Hook 3: CUiDigiline tick
    InstallHook("CUiDigiline tick", base, RVA_DIGILINE_TICK,
                (void*)&HookedDigilineTick, reinterpret_cast<void**>(&s_originalDigilineTick), &s_digilineTickTarget);

    // Hook 4: CUiFieldDigiLine tick
    InstallHook("CUiFieldDigiLine tick", base, RVA_FIELD_DIGILINE_TICK,
                (void*)&HookedFieldDigilineTick, reinterpret_cast<void**>(&s_originalFieldDigilineTick), &s_fieldDigilineTickTarget);

    m_installed = true;
    Logger_Log("TalkComm", "Installation complete (4 hooks)");
}

static void RemoveHook(void*& target)
{
    if (target) {
        MH_DisableHook(target);
        MH_RemoveHook(target);
        target = nullptr;
    }
}

void TalkCommHandler::Uninstall()
{
    if (!m_installed) return;

    RemoveHook(s_msgHookTarget);
    RemoveHook(s_edenTickTarget);
    RemoveHook(s_digilineTickTarget);
    RemoveHook(s_fieldDigilineTickTarget);

    s_originalMsgHandler = nullptr;
    s_originalEdenTick = nullptr;
    s_originalDigilineTick = nullptr;
    s_originalFieldDigilineTick = nullptr;
    m_installed = false;

    Logger_Log("TalkComm", "Hooks uninstalled");
}

void TalkCommHandler::OnFrame()
{
    if (!m_installed) return;

    std::string speaker;
    std::string text;

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_captured.pending) return;
        speaker = m_captured.speaker;
        text = m_captured.text;
        m_captured.pending = false;
    }

    std::string announcement;
    bool speakerChanged = (speaker != m_announcedSpeaker);

    if (!speaker.empty() && speakerChanged) {
        m_announcedSpeaker = speaker;
        if (!text.empty()) {
            announcement = speaker + ": " + text;
        } else {
            announcement = speaker;
        }
    } else if (!text.empty()) {
        announcement = text;
    }

    if (!announcement.empty()) {
        Logger_Log("TalkComm", "Announcing: %s", announcement.c_str());
        SpeechManager::Get()->Speak(announcement, true);
    }
}
