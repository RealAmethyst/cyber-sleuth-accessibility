// File: include/handlers/yesno_handler.h
// Purpose: Accessibility handler for CUiYesNoWindow — Yes/No/Cancel dialog prompts.
//
// Hooks CUiYesNoWindow's tick (vtable[3], RVA 0x426c90) via MinHook ONLY to
// capture the this pointer. All state reading and speech in OnFrame().
//
// Message text comes from TextCapture (yes_no_message table lookups).
// Button labels come from LookupText("common_message", id, language):
//   YES_TEXT_ID  (+0x78): usually row 2100 -> "Yes"
//   NO_TEXT_ID   (+0x7C): usually row 2101 -> "No"
//   Custom dialogs may use non-standard rows (e.g. campaign select uses brand names).
//
// CUiYesNoWindow member layout (from Ghidra):
//   +0x0c: int32  state (0=idle, 1=ready, 2=opening, 3=anim, 4=interactive, 5=closing, 6=done)
//   +0x40: ptr    active window widget
//   +0x48: ptr    active Yes cursor widget
//   +0x50: ptr    active No cursor widget
//   +0x58: int32  style type (1=info, 2=system)
//   +0x80: byte   cancel enabled (B button — NOT a third cursor position)
//   +0x84: int32  result (0=none, 1=Yes, 2=No, 3=Cancel)
//   +0x88: byte   active/visible
//
// Speech rules:
//   state -> 4 (dialog opened): Speak(message, interrupt=true), then Speak(option, interrupt=false)
//   cursor moved in state 4:    Speak(option, interrupt=true)
//   state 5/6 (closing/done):  silence (speech is stale)

class YesNoHandler : public IFrameHandler  // (line 32)
{
public:
    static YesNoHandler* Get();  // (line 35) — singleton accessor

    void Install();    // (line 37)
    void Uninstall();  // (line 38)

    bool IsInstalled() const;  // (line 40)

    // IFrameHandler: reads state and cursor, speaks changes.
    void OnFrame() override;  // (line 43)

    // Public so a SEH wrapper free function can call it without friendship.
    void OnFrameInner(void* thisPtr);  // (line 46)

private:
    YesNoHandler() = default;  // (line 49)

    bool m_installed = false;           // (line 51)
    int32_t m_lastState = -1;           // (line 52)
    int32_t m_lastCursor = -1;          // (line 53)
    bool m_dialogActive = false;        // (line 54)
    std::string m_currentMessage;       // (line 55) — cached from TextCapture::ConsumeYesNoMessage()

    static inline std::atomic<void*> s_thisPtr{nullptr};  // (line 58) — written by tick detour
    static inline std::atomic<bool> s_tickFired{false};   // (line 59) — signals OnFrame that tick ran

    using TickFunc = void(__fastcall*)(void* thisPtr, void* param2);  // (line 61)
    static inline TickFunc s_originalTick = nullptr;  // (line 62)
    static inline void* s_hookTarget = nullptr;       // (line 63)

    // Minimal tick detour: calls original, stores this pointer, sets tickFired.
    static void __fastcall HookedTick(void* thisPtr, void* param2);  // (line 65)

    using GetTextTableManagerFunc = void* (*)();  // (line 68)
    using LookupTextFunc = const char* (*)(void* manager, const char* tableName,
                                           int rowId, unsigned int language);  // (line 69)
    static inline GetTextTableManagerFunc s_getTextTableManager = nullptr;  // (line 70)
    static inline LookupTextFunc s_lookupText = nullptr;                    // (line 71)

    // State readers.
    static int32_t ReadState(void* thisPtr);         // (line 74)
    static int32_t ReadResult(void* thisPtr);        // (line 75)
    static bool    ReadCancelEnabled(void* thisPtr); // (line 76)
    static int32_t ReadCursor(void* thisPtr);        // (line 77)
    // NOTE: cursor 0 = Yes (top button), cursor 1 = No (bottom button) — counter-intuitive ordering

    // Call LookupText("common_message", commonMessageId, language) for a button label.
    std::string LookupButtonLabel(int32_t commonMessageId);  // (line 80)

    // Speak the dialog message (interrupt=true) then queue the current option (interrupt=false).
    void AnnounceDialogOpened();  // (line 83)

    // Speak "Yes/No label, N of M" for the given cursor position (interrupt=true).
    void AnnounceCurrentOption(int32_t cursor);  // (line 84)
}
