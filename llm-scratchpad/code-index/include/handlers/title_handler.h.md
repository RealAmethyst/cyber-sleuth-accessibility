// File: include/handlers/title_handler.h
// Purpose: Accessibility handler for the title screen (CUiTitle).
//
// Hooks CUiTitle's tick function (vtable[3], RVA 0x4CC2F0) via MinHook ONLY
// to capture the this pointer atomically. All state reading and speech happen
// in OnFrame() (SwapBuffers context) — never inside the tick detour.
//
// Title menu items are pre-baked textures (identical in all languages — hardcoded):
//   cursor 0 -> "New Game"
//   cursor 1 -> "Continue"
//   cursor 2 -> "New Game +"  (may be locked: +0xc4 == 0 means locked)
//   cursor 3 -> "Exit Game"
//
// CUiTitle member layout (from Ghidra):
//   +0xa8:  uint32 state index (state 12 = interactive menu)
//   +0xb8:  byte   done flag
//   +0xc4:  byte   locked-item flag (0 = "New Game +" is locked)
//   +0x114: int32  cursor index (0-3)
//   +0x124: int32  item count

class TitleHandler : public IFrameHandler  // (line 26)
{
public:
    static TitleHandler* Get();  // (line 29) — singleton accessor

    // Install MinHook on CUiTitle's tick. Call after hooks_init().
    void Install();    // (line 32)
    void Uninstall();  // (line 33)

    bool IsInstalled() const;  // (line 36)

    // IFrameHandler: reads state and speaks changes. Called every frame from SwapBuffers.
    void OnFrame() override;  // (line 39)

    // Inner logic separated so a free-function SEH wrapper can call it without friendship.
    void OnFrameInner(void* thisPtr);  // (line 42)

private:
    TitleHandler() = default;  // (line 45)

    bool m_installed = false;               // (line 47)
    int32_t m_lastCursorIndex = -1;         // (line 48)
    uint32_t m_lastState = 0xFFFFFFFF;      // (line 49)
    bool m_menuActive = false;              // (line 50)

    // Atomic this pointer: written by HookedTick, read by OnFrame.
    static inline std::atomic<void*> s_thisPtr{nullptr};  // (line 53)

    using TickFunc = void(__fastcall*)(void* thisPtr, void* param2);  // (line 56)
    static inline TickFunc s_originalTick = nullptr;  // (line 57)
    static inline void* s_hookTarget = nullptr;       // (line 58)

    // Minimal tick detour: calls original then stores this pointer. Nothing else.
    static void __fastcall HookedTick(void* thisPtr, void* param2);  // (line 61)

    // State readers — called from OnFrame/OnFrameInner only (never from tick detour).
    static uint32_t ReadState(void* thisPtr);      // (line 64)
    static int32_t  ReadCursor(void* thisPtr);     // (line 65)
    static int32_t  ReadItemCount(void* thisPtr);  // (line 66)

    // Map a 0-based cursor index to the hardcoded English item string.
    static const char* GetTitleMenuItem(int cursorIndex);  // (line 69)

    // Speak menu name then queue first item (called when state transitions to 12).
    void AnnounceMenuOpened(void* thisPtr);   // (line 72)

    // Speak the item at the current cursor position: "name, N of M"
    void AnnounceCurrentItem(void* thisPtr);  // (line 73)
}
