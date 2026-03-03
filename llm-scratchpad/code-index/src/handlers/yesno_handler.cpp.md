// File: src/handlers/yesno_handler.cpp
// Purpose: Accessibility handler for CUiYesNoWindow (Yes/No/Cancel dialogs).
//          Hooks vtable[3] tick via MinHook ONLY to capture the this pointer.
//          All state reading and speech happen in OnFrame() (SwapBuffers context).
//
//          Message text is captured from TextCapture (yes_no_message table lookups).
//          The game looks up the message once when the dialog opens; YesNoHandler
//          caches it via ConsumeYesNoMessage() and announces on state->4 transition.
//
//          State at this+0x0C (int32), cursor byte at this+0x81 (0=No, 1=Yes —
//          confirmed swapped from intuitive order via gameplay testing).
//          Button label IDs at this+0x78 (Yes) and this+0x7C (No) — looked up
//          dynamically from common_message table for language support.

// State constants (from Ghidra decompilation of CUiYesNoWindow tick).
static constexpr int32_t STATE_IDLE        = 0  (line 27)
static constexpr int32_t STATE_READY       = 1  (line 28)
static constexpr int32_t STATE_OPENING     = 2  (line 29)
static constexpr int32_t STATE_ANIM        = 3  (line 30)
static constexpr int32_t STATE_INTERACTIVE = 4  (line 31)
static constexpr int32_t STATE_CLOSING     = 5  (line 32)
static constexpr int32_t STATE_DONE        = 6  (line 33)

// Singleton accessor.
YesNoHandler* YesNoHandler::Get()  (line 38)

// Resolves text API pointers and installs MinHook on CUiYesNoWindow tick
// (RVA from Offsets::FUNC_CUiYesNoWindow_Tick).
void YesNoHandler::Install()  (line 48)

// Disables/removes tick hook, resets all state (including atomics s_thisPtr, s_tickFired).
void YesNoHandler::Uninstall()  (line 85)

// Tick detour — MINIMAL. Calls original, then atomically stores thisPtr and
// sets s_tickFired flag. No other work done here.
void __fastcall YesNoHandler::HookedTick(void* thisPtr, void* param2)  (line 112)

// File-scope SEH wrapper for OnFrameInner.
static void OnFrameSEH(YesNoHandler* handler, void* thisPtr)  (line 128)

// IFrameHandler::OnFrame. Uses s_tickFired (atomic exchange) to detect whether
// the dialog is active. On tick stopping: resets dialog state.
// While tick fires: delegates to OnFrameSEH -> OnFrameInner.
void YesNoHandler::OnFrame()  (line 138)

// Reads state and cursor each frame. Consumes pending yes_no_message from TextCapture.
// On state->4 (interactive): calls AnnounceDialogOpened().
// On cursor change while interactive: calls AnnounceCurrentOption().
// On state->5/6 from 4: logs result value.
void YesNoHandler::OnFrameInner(void* thisPtr)  (line 162)

// --- State reading helpers ---

// Reads int32 at thisPtr + Offsets::YesNoWindow::STATE (0x0C).
int32_t YesNoHandler::ReadState(void* thisPtr)  (line 206)

// Reads int32 at thisPtr + Offsets::YesNoWindow::RESULT (0x84).
int32_t YesNoHandler::ReadResult(void* thisPtr)  (line 212)

// Reads byte at thisPtr + Offsets::YesNoWindow::CANCEL_ENABLED (0x80).
bool YesNoHandler::ReadCancelEnabled(void* thisPtr)  (line 218)

// Reads byte at thisPtr + Offsets::YesNoWindow::CURSOR_INDEX (0x81).
// Returns -1 if value > 2 (sanity check).
int32_t YesNoHandler::ReadCursor(void* thisPtr)  (line 224)

// Calls LookupText("common_message", commonMessageId, language).
// Returns empty string if API unavailable or text is empty.
std::string YesNoHandler::LookupButtonLabel(int32_t commonMessageId)  (line 239)

// --- Announcements ---

// Speaks m_currentMessage (interrupt); if no message was captured, speaks
// "Yes or No" as a fallback. Then reads and queues the current option.
void YesNoHandler::AnnounceDialogOpened()  (line 270)

// Reads Yes/No label IDs from this+0x78/0x7C, looks them up via LookupButtonLabel(),
// builds "label, N of 2" announcement. Uses interrupt=false on first announcement
// (dialog just opened, queues after message); interrupt=true on subsequent cursor moves.
void YesNoHandler::AnnounceCurrentOption(int32_t cursor)  (line 292)
