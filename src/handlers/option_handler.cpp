// CUiOption accessibility handler.
//
// The options menu presents a list of settings: volume controls, toggles,
// sub-menus (Graphic Options, Button Settings), and action items.
//
// Detection strategy:
//   - State handler hooks (not tick!) detect menu activity:
//       State 5  (RVA 0x492DA0) — main options tab
//       State 13 (RVA 0x493A20) — button settings
//       State 17 (RVA 0x493C50) — graphic options
//     These only fire when the menu is in that interactive state.
//     CUiOption's tick (0x48DD20) runs every frame even when idle, but the
//     state handlers are dispatched from it only for non-zero states.
//   - Memory offset +0x0138 detects sub-screen (5=main, 17=graphic, 13=button)
//   - Memory offset +0x0338 reads cursor position (global index)
//   - TextCapture help_message events detect cursor movement (steady state)
//   - TextCapture common_message events detect value changes
//
// Speech format: "Name: Value, Description, N of M"
//
// Value filtering:
//   Each graphic option setting maps to specific common_message row IDs for its
//   values (e.g., Depth of Field uses 6120/6121 for Enable/Disable). During
//   initialization bursts where ALL settings' values fire at once, this filter
//   extracts only the value belonging to the currently focused item.
//   For unfiltered settings (main tab), single-value-only detection is used.
//
// Transition flow:
//   1. Sub-screen stabilizes → speak screen name (interrupt=true)
//   2. Enter SETTLING state: wait for the focused item's value to arrive
//   3. Value arrives (via filter) → speak initial item with value (queued)
//   4. Or user moves cursor → speak new item, skip initial value
//   5. Enter ACTIVE state: normal cursor/value tracking

#include "handlers/option_handler.h"
#include "handlers/yesno_handler.h"
#include "handlers/handler_utils.h"
#include "speech_manager.h"
#include "game_text.h"
#include "text_capture.h"
#include "offsets.h"
#include "logger.h"

#include <windows.h>
#include <unordered_set>
#include <unordered_map>
#include <vector>

// Known label IDs (common_message) — used to filter them OUT of value events.
static const std::unordered_set<int> kLabelIds = {
    2200, 2201, 2202, 2203, 2204, 2205, 2206, 2226,
    2207, 2208, 2230, 2253,
    2251, 2254, 2255,
    6100, 6101, 6102, 6103,
    2100, 2101,  // Yes/No dialog button labels (not option values)
};

// Known stable sub-screen state values.
static constexpr int SUBSCREEN_MAIN     = 5;
static constexpr int SUBSCREEN_GRAPHIC  = 17;
static constexpr int SUBSCREEN_BUTTON   = 13;

// Global cursor base offsets per sub-screen.
static constexpr int CURSOR_BASE_MAIN    = 0;
static constexpr int CURSOR_BASE_GRAPHIC = 10;
static constexpr int CURSOR_BASE_BUTTON  = 9;

static bool IsKnownSubScreen(int s)
{
    return s == SUBSCREEN_MAIN || s == SUBSCREEN_GRAPHIC || s == SUBSCREEN_BUTTON;
}

// Find the inner screen object for a given command ID (0x90=graphic, 0x8f=button)
// by traversing the GraphicGlobal's vector, matching +0x08 (command ID).
static uintptr_t FindInnerScreenObject(int cmdId)
{
    auto moduleBase = reinterpret_cast<uintptr_t>(GetModuleHandle(nullptr));
    auto globalPtrAddr = reinterpret_cast<uintptr_t*>(moduleBase + Offsets::Option::DAT_GraphicGlobal);
    auto globalObj = *globalPtrAddr;
    if (!globalObj) return 0;

    auto vecBegin = *reinterpret_cast<uintptr_t*>(globalObj + 0x10);
    auto vecEnd   = *reinterpret_cast<uintptr_t*>(globalObj + 0x18);
    int count = static_cast<int>((vecEnd - vecBegin) / 8);
    if (count <= 0 || count > 50) return 0;

    for (int i = 0; i < count; i++) {
        auto wrapperPtr = *reinterpret_cast<uintptr_t*>(vecBegin + i * 8);
        if (!wrapperPtr) continue;
        auto innerPtr = *reinterpret_cast<uintptr_t*>(wrapperPtr);
        if (!innerPtr) continue;
        auto id = *reinterpret_cast<int32_t*>(innerPtr + Offsets::Option::INNER_CMD_ID);
        if (id == cmdId) return innerPtr;
    }
    return 0;
}

// Read cursor from the appropriate source for the current sub-screen.
// Main tab: CUiOption +0x338 (global index). Graphic/Button: inner object +0x1A0 (0-based local).
static int ReadSubScreenCursor(void* optionThis, int subScreen)
{
    if (subScreen == SUBSCREEN_MAIN) {
        return HandlerUtils::ReadMemory<int32_t>(optionThis, Offsets::Option::CURSOR_INDEX);
    }

    int cmdId = (subScreen == SUBSCREEN_GRAPHIC) ? Offsets::Option::GRAPHIC_CMD_ID
                                                  : Offsets::Option::BUTTON_CMD_ID;
    auto innerObj = FindInnerScreenObject(cmdId);
    if (!innerObj) return -1;
    return *reinterpret_cast<int32_t*>(innerObj + Offsets::Option::INNER_CURSOR);
}

// Action items that never have a value (sub-menus, restore, apply).
static bool IsActionItem(int helpId)
{
    static const std::unordered_set<int> kActionIds = {
        // Title-screen IDs
        2232, 2233,       // Button Settings, Graphic Options (main tab sub-menus)
        2217, 2218,       // Restore Defaults, Apply (main tab)
        // In-game IDs
        2222, 2223,       // Button Settings, Graphic Options (main tab sub-menus)
        2207, 2208,       // Restore Defaults, Apply (both in-game main tab & graphic sub-menu)
    };
    return kActionIds.count(helpId) > 0;
}

// Valid value row IDs per help_message ID.
// Used to extract the correct value from initialization bursts where all
// settings' values fire at once. Returns nullptr for settings without
// a specific filter (main tab — uses single-value-only heuristic).
static const std::unordered_set<int>* GetValueFilter(int helpId)
{
    static const std::unordered_map<int, std::unordered_set<int>> filters = {
        // Volume settings (title-screen + in-game help IDs)
        {2210, {2209,2210,2211,2212,2213,2214,2215,2216,2217,2218,2219}},
        {2211, {2209,2210,2211,2212,2213,2214,2215,2216,2217,2218,2219}},
        {2212, {2209,2210,2211,2212,2213,2214,2215,2216,2217,2218,2219}},
        {2200, {2209,2210,2211,2212,2213,2214,2215,2216,2217,2218,2219}},
        {2201, {2209,2210,2211,2212,2213,2214,2215,2216,2217,2218,2219}},
        {2202, {2209,2210,2211,2212,2213,2214,2215,2216,2217,2218,2219}},
        // Minimap (title 2213, in-game 2203)
        {2213, {2220, 2221}}, {2203, {2220, 2221}},
        // Partner Digimon (title 2214, in-game 2204)
        {2214, {2220, 2221, 2231}}, {2204, {2220, 2221, 2231}},
        // Battle Cutscenes (title 2215, in-game 2205)
        {2215, {2222, 2223}}, {2205, {2222, 2223}},
        // Digimon Voices (title 2216, in-game 2206)
        {2216, {2224, 2225}}, {2206, {2224, 2225}},
        // Battle Difficulty (title 2219, in-game 2209)
        {2219, {2227, 2228, 2229}}, {2209, {2227, 2228, 2229}},
        // Data Use Agreement (title 2231, in-game 2221)
        {2231, {2252, 2253}}, {2221, {2252, 2253}},
        // Graphic sub-menu (same IDs in both contexts)
        {4000, {6120, 6121}}, {4001, {6120, 6121}},
        {4002, {6122, 6123, 6124}}, {4003, {6125, 6126, 6127, 6128}},
    };
    auto it = filters.find(helpId);
    return (it != filters.end()) ? &it->second : nullptr;
}

// help_message IDs ordered by position within each sub-screen.
static const std::vector<int>& GetMainTabHelpIds()
{
    static const std::vector<int> ids = {
        2210, 2211, 2212, 2213, 2214, 2215, 2216, 2219,
        2231, 2232, 2233, 2217, 2218,
    };
    return ids;
}

static const std::vector<int>& GetInGameMainTabHelpIds()
{
    static const std::vector<int> ids = {
        2200, 2201, 2202, 2203, 2204, 2205, 2206, 2209,
        2221, 2222, 2223, 2207, 2208,
    };
    return ids;
}

static const std::vector<int>& GetGraphicHelpIds()
{
    static const std::vector<int> ids = {
        4000, 4001, 4002, 4003, 2207, 2208,
    };
    return ids;
}

OptionHandler* OptionHandler::Get()
{
    static OptionHandler instance;
    return &instance;
}

// ============================================================
// Three-hook install/uninstall
// ============================================================

void OptionHandler::Install()
{
    if (m_installed) return;

    uintptr_t base = reinterpret_cast<uintptr_t>(getBaseOffset());

    // State handler RVAs and their detour functions
    struct HookDef {
        uintptr_t rva;
        void* detour;
        const char* label;
    };
    HookDef defs[HOOK_COUNT] = {
        { Offsets::Option::FUNC_State5_Main,     (void*)&HookedState<0>, "state5_main"    },
        { Offsets::Option::FUNC_State13_Button,   (void*)&HookedState<1>, "state13_button" },
        { Offsets::Option::FUNC_State17_Graphic,  (void*)&HookedState<2>, "state17_graphic"},
    };

    int installed = 0;
    for (int i = 0; i < HOOK_COUNT; i++) {
        s_hooks[i].target = reinterpret_cast<void*>(base + defs[i].rva);
        MH_STATUS status = MH_CreateHook(
            s_hooks[i].target,
            defs[i].detour,
            reinterpret_cast<void**>(&s_hooks[i].original));
        if (status != MH_OK) {
            Logger_Log("Option", "MH_CreateHook failed for %s (RVA 0x%llx): %d",
                       defs[i].label, (unsigned long long)defs[i].rva, status);
            continue;
        }
        status = MH_EnableHook(s_hooks[i].target);
        if (status != MH_OK) {
            Logger_Log("Option", "MH_EnableHook failed for %s: %d", defs[i].label, status);
            MH_RemoveHook(s_hooks[i].target);
            s_hooks[i].target = nullptr;
            s_hooks[i].original = nullptr;
            continue;
        }
        installed++;
    }

    if (installed == 0) {
        Logger_Log("Option", "All hooks failed — handler inactive");
        return;
    }

    m_installed = true;
    Logger_Log("Option", "Installed %d/%d state handler hooks", installed, HOOK_COUNT);
}

void OptionHandler::Uninstall()
{
    if (!m_installed) return;

    for (int i = 0; i < HOOK_COUNT; i++) {
        if (s_hooks[i].target) {
            MH_DisableHook(s_hooks[i].target);
            MH_RemoveHook(s_hooks[i].target);
        }
        s_hooks[i].target = nullptr;
        s_hooks[i].original = nullptr;
    }

    s_thisPtr.store(nullptr, std::memory_order_relaxed);
    s_tickFired.store(false, std::memory_order_relaxed);
    m_screenActive = false;
    m_installed = false;

    MemoryInspector::Get()->ClearPointer("Option");
    Logger_Log("Option", "Uninstalled");
}

void OptionHandler::OnScreenClosed()
{
    if (m_menuOpened) {
        Logger_Log("Option", "Menu closed");
    }
    m_menuOpened = false;
    m_lastHelpId = -1;
    m_lastSubScreen = -1;
    m_settling = false;
    m_settleFrames = 0;
    m_resuming = false;
    m_yesNoShown = false;
    m_cachedValue.clear();

    TextCapture::Get()->SetOptionTrackingActive(false);
    TextCapture::Get()->ConsumeHelpMessages();
    TextCapture::Get()->ConsumeOptionValues();
}

// SEH wrapper — separate function, no C++ objects with destructors.
void OptionHandler::OnFrameSEH(OptionHandler* handler, void* thisPtr)
{
    __try {
        handler->OnFrameInner(thisPtr);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        Logger_Log("Option", "EXCEPTION in OnFrame (this=%p, code=0x%08lx)",
                   thisPtr, GetExceptionCode());
    }
}

const std::unordered_map<int, int>& OptionHandler::GetHelpToLabelMap()
{
    static const std::unordered_map<int, int> map = {
        // Title-screen help IDs
        {2210, 2200}, {2211, 2201}, {2212, 2202}, {2213, 2203},
        {2214, 2204}, {2215, 2205}, {2216, 2206}, {2219, 2226},
        {2232, 2254}, {2233, 2255},
        {2217, 2207}, {2218, 2208}, {2231, 2251},
        // In-game help IDs (help ID == label ID for 2200-2206)
        {2200, 2200}, {2201, 2201}, {2202, 2202}, {2203, 2203},
        {2204, 2204}, {2205, 2205}, {2206, 2206}, {2209, 2226},
        {2222, 2254}, {2223, 2255}, {2221, 2251},
        {2207, 2207}, {2208, 2208},
        // Graphic sub-menu (same in both contexts)
        {4000, 6100}, {4001, 6101}, {4002, 6102}, {4003, 6103},
    };
    return map;
}

// Sub-screen-aware position lookup.
// 2207/2208 are used as both graphic sub-menu positions (4,5) and
// in-game main tab positions (11,12) — requires sub-screen context.
static int GetPositionForHelp(int subScreen, int helpId)
{
    static const std::unordered_map<int, int> mainPos = {
        // Title-screen help IDs
        {2210,  0}, {2211,  1}, {2212,  2}, {2213,  3},
        {2214,  4}, {2215,  5}, {2216,  6}, {2219,  7},
        {2231,  8}, {2232,  9}, {2233, 10}, {2217, 11}, {2218, 12},
        // In-game help IDs
        {2200,  0}, {2201,  1}, {2202,  2}, {2203,  3},
        {2204,  4}, {2205,  5}, {2206,  6}, {2209,  7},
        {2221,  8}, {2222,  9}, {2223, 10}, {2207, 11}, {2208, 12},
    };
    static const std::unordered_map<int, int> graphicPos = {
        {4000,  0}, {4001,  1}, {4002,  2}, {4003,  3},
        {2207,  4}, {2208,  5},
    };
    const auto& map = (subScreen == SUBSCREEN_GRAPHIC) ? graphicPos : mainPos;
    auto it = map.find(helpId);
    return (it != map.end()) ? it->second : -1;
}

int OptionHandler::GetItemCount(int subScreen)
{
    switch (subScreen) {
    case SUBSCREEN_MAIN:    return 13;
    case SUBSCREEN_GRAPHIC: return 6;
    default:                return 0;
    }
}

static int GlobalToLocal(int subScreen, int globalCursor)
{
    switch (subScreen) {
    case SUBSCREEN_MAIN:    return globalCursor - CURSOR_BASE_MAIN;
    case SUBSCREEN_GRAPHIC: return globalCursor - CURSOR_BASE_GRAPHIC;
    case SUBSCREEN_BUTTON:  return globalCursor - CURSOR_BASE_BUTTON;
    default:                return -1;
    }
}

static int HelpIdForCursor(int subScreen, int globalCursor, bool inGame = false)
{
    int local = GlobalToLocal(subScreen, globalCursor);
    const std::vector<int>* ids = nullptr;
    if (subScreen == SUBSCREEN_MAIN)
        ids = inGame ? &GetInGameMainTabHelpIds() : &GetMainTabHelpIds();
    else if (subScreen == SUBSCREEN_GRAPHIC)
        ids = &GetGraphicHelpIds();
    if (!ids || local < 0 || local >= (int)ids->size()) return -1;
    return (*ids)[local];
}

std::string OptionHandler::LookupSettingName(int helpId)
{
    auto& map = GetHelpToLabelMap();
    auto it = map.find(helpId);
    if (it != map.end()) {
        std::string name = GameText_Lookup("common_message", it->second);
        if (!name.empty()) return name;
    }
    return "";
}

// Extract value text from TextCapture, filtered for the currently focused item.
//
// If the focused item has a value row filter (graphic options), only accept
// values from those specific row IDs. This correctly handles init bursts where
// all settings' values fire at once — only the focused item's value passes.
//
// If no filter exists (main tab settings), accept the value only if exactly
// one non-label value fired this frame (rejects multi-value init bursts).
std::string OptionHandler::FindValueText()
{
    auto values = TextCapture::Get()->ConsumeOptionValues();

    auto* filter = GetValueFilter(m_lastHelpId);

    if (filter) {
        // Filtered: return first value matching the focused item's valid row IDs
        for (auto& v : values) {
            if (filter->count(v.rowId)) return v.text;
        }
        return "";
    }

    // Unfiltered: accept only if exactly one non-label value (rejects bursts)
    std::string best;
    int count = 0;
    for (auto& v : values) {
        if (kLabelIds.find(v.rowId) == kLabelIds.end()) {
            best = v.text;
            count++;
        }
    }
    return (count == 1) ? best : "";
}

std::string OptionHandler::AppendPosition(const std::string& base, int helpId)
{
    int pos = GetPositionForHelp(m_lastSubScreen, helpId);
    if (pos >= 0) {
        int total = GetItemCount(m_lastSubScreen);
        if (total > 0) {
            return base + ", " + std::to_string(pos + 1) +
                   " of " + std::to_string(total);
        }
    }
    return base;
}

std::string OptionHandler::BuildAnnouncement(int helpId, const std::string& helpText,
                                              const std::string& valueText)
{
    std::string name = LookupSettingName(helpId);
    std::string announcement = !name.empty() ? name : helpText;

    if (!valueText.empty())
        announcement += ": " + valueText;
    if (!name.empty() && !helpText.empty())
        announcement += ", " + helpText;

    return AppendPosition(announcement, helpId);
}

std::string OptionHandler::BuildInitialAnnouncement(void* thisPtr,
                                                     const std::string& valueText)
{
    int cursor = ReadSubScreenCursor(thisPtr, m_lastSubScreen);
    int helpId = -1;
    int local = -1;

    if (m_lastSubScreen == SUBSCREEN_MAIN) {
        helpId = HelpIdForCursor(m_lastSubScreen, cursor);
        local = GlobalToLocal(m_lastSubScreen, cursor);
    } else if (m_lastSubScreen == SUBSCREEN_GRAPHIC && cursor >= 0) {
        const auto& ids = GetGraphicHelpIds();
        if (cursor < (int)ids.size()) {
            helpId = ids[cursor];
            local = cursor;
        }
    }

    Logger_Log("Option", "BuildInitial: subScreen=%d, cursor=%d, helpId=%d, value='%s'",
               m_lastSubScreen, cursor, helpId, valueText.c_str());

    if (helpId < 0) return "";

    m_lastHelpId = helpId;

    std::string name = LookupSettingName(helpId);
    std::string helpText = GameText_Lookup("help_message", helpId);

    std::string announcement;
    if (!name.empty())
        announcement = name;
    else if (!helpText.empty())
        announcement = helpText;
    else
        return "";

    if (!valueText.empty())
        announcement += ": " + valueText;
    if (!name.empty() && !helpText.empty())
        announcement += ", " + helpText;

    int total = GetItemCount(m_lastSubScreen);
    if (total > 0 && local >= 0)
        announcement += ", " + std::to_string(local + 1) + " of " + std::to_string(total);

    return announcement;
}

// State handler hooks: OnFrame checks if any of the three hooked state
// functions fired.  CUiOption is a persistent singleton whose tick runs every
// frame from game start — sub-screen 0 is its idle/no-op state, but it also
// passes through 0 during Yes/No dialogs and sub-screen transitions.  Rather
// than trying to distinguish "truly closed" from "temporarily idle", we simply
// stay in transition mode whenever our hooked states stop firing.  When they
// resume the existing sub-screen-change logic handles re-announcement.
void OptionHandler::OnFrame()
{
    if (!m_installed) return;

    bool tickFired = s_tickFired.exchange(false, std::memory_order_relaxed);

    if (tickFired) {
        // A hooked state handler fired — menu is active
        void* thisPtr = s_thisPtr.load(std::memory_order_relaxed);
        if (!thisPtr) return;

        m_screenActive = true;
        MemoryInspector::Get()->SetActivePointer("Option", thisPtr);
        OnFrameSEH(this, thisPtr);
    }
    else if (m_screenActive) {
        // No hooked state fired — enter/stay in transition mode.
        // Covers: Yes/No dialog overlays, sub-screen transitions, opening
        // animation, and actual menu close.  No attempt to detect close here;
        // when the menu truly closes, this branch runs harmlessly each frame
        // (draining empty event buffers) until a state handler fires again.
        if (!m_resuming && m_lastSubScreen >= 0) {
            Logger_Log("Option", "Handlers paused from subScreen=%d", m_lastSubScreen);
            m_preTransitionSubScreen = m_lastSubScreen;
            m_resuming = true;
            m_lastSubScreen = -1;
        }
        TextCapture::Get()->ConsumeHelpMessages();
        TextCapture::Get()->ConsumeOptionValues();
    }
    else {
        // Menu not active — disable auto-enabled tracking to stop accumulation,
        // but do NOT consume events (they may be needed when the state handler
        // fires on the next frame — values arrive before the hook fires).
        TextCapture::Get()->SetOptionTrackingActive(false);
    }
}

void OptionHandler::OnFrameInner(void* thisPtr)
{
    int subScreen = HandlerUtils::ReadMemory<int32_t>(thisPtr, Offsets::Option::SUB_SCREEN);

    // During transition (unknown sub-screen): wait for known sub-screen
    if (!IsKnownSubScreen(subScreen)) {
        if (!m_menuOpened) {
            // Don't enable tracking yet — tick may fire before menu is actually open
            return;
        }
        if (m_lastSubScreen >= 0) {
            // Transitioning away from a known sub-screen (e.g., Yes/No dialog).
            // Reset so the return to the known sub-screen triggers re-announcement.
            Logger_Log("Option", "Entering transition from subScreen=%d", m_lastSubScreen);
            m_lastSubScreen = -1;
            m_resuming = true;
        }
        TextCapture::Get()->ConsumeHelpMessages();
        return;
    }

    // First frame at a known sub-screen — menu is definitely open
    if (!m_menuOpened) {
        m_menuOpened = true;
        m_settling = true;
        TextCapture::Get()->SetOptionTrackingActive(true);
        Logger_Log("Option", "Menu opened (subScreen=%d)", subScreen);
    }

    // Sub-screen stabilized at a new known value?
    if (subScreen != m_lastSubScreen) {
        m_lastSubScreen = subScreen;
        m_lastHelpId = -1;
        m_settling = true;
        m_settleFrames = 0;

        // Drain stale help events from transition/init burst.
        // The init burst fires help for ALL sub-menus at once, so event order
        // is unreliable for detecting the focused item. Use cursor instead.
        auto helpEvents = TextCapture::Get()->ConsumeHelpMessages();

        // Detect title-screen vs in-game context from drained help events
        if (subScreen == SUBSCREEN_MAIN) {
            for (auto& h : helpEvents) {
                if (h.rowId >= 2200 && h.rowId <= 2209) { m_inGameIds = true; break; }
                if (h.rowId >= 2210 && h.rowId <= 2219) { m_inGameIds = false; break; }
            }
        }

        // Read cursor and map to help ID — reliable because cursor memory
        // is always valid when the state handler fires.
        // Note: ReadSubScreenCursor returns LOCAL cursor for graphic/button
        // (from inner object), so index directly into help ID arrays for those.
        int cursor = ReadSubScreenCursor(thisPtr, subScreen);
        if (subScreen == SUBSCREEN_GRAPHIC) {
            const auto& ids = GetGraphicHelpIds();
            if (cursor >= 0 && cursor < (int)ids.size())
                m_lastHelpId = ids[cursor];
        } else if (subScreen == SUBSCREEN_MAIN) {
            int helpId = HelpIdForCursor(subScreen, cursor, m_inGameIds);
            if (helpId >= 0)
                m_lastHelpId = helpId;
        }

        // Announce screen name (interrupt = true).
        // Suppress only when resuming to the SAME sub-screen (e.g., returning
        // from Yes/No dialog). Always announce when entering a different sub-screen
        // (e.g., main→graphic transition also goes through m_resuming path).
        bool suppressName = m_resuming && (subScreen == m_preTransitionSubScreen);
        m_resuming = false;

        if (!suppressName) {
            if (subScreen == SUBSCREEN_GRAPHIC) {
                std::string name = GameText_Lookup("common_message", 2255);
                SpeechManager::Get()->Speak(name.empty() ? "Graphic Options" : name, true);
            } else if (subScreen == SUBSCREEN_BUTTON) {
                std::string name = GameText_Lookup("common_message", 2254);
                SpeechManager::Get()->Speak(name.empty() ? "Button Settings" : name, true);
            } else if (subScreen == SUBSCREEN_MAIN) {
                SpeechManager::Get()->Speak("Options", true);
            }
        }

        Logger_Log("Option", "Sub-screen %d: settling (cursor=%d, help=%d)", subScreen, cursor, m_lastHelpId);
        return;
    }

    // --- SETTLING: waiting for the focused item's help ID and value ---
    if (m_settling) {
        m_settleFrames++;
        auto helpEvents = TextCapture::Get()->ConsumeHelpMessages();

        if (!helpEvents.empty()) {
            // Find first event relevant to current sub-screen
            // (rejects cross-sub-screen help IDs from transition/init bursts)
            const TextCapture::HelpMessageEvent* relevant = nullptr;
            for (auto& h : helpEvents) {
                if (GetPositionForHelp(m_lastSubScreen, h.rowId) >= 0) {
                    relevant = &h;
                    break;
                }
            }

            if (relevant) {
                if (m_lastHelpId < 0) {
                    // Fallback: learn from event when cursor read failed
                    m_lastHelpId = relevant->rowId;
                    m_rememberedHelpId[m_lastSubScreen] = relevant->rowId;
                    Logger_Log("Option", "Settling: learned help=%d from event", relevant->rowId);

                    // Action items — announce immediately (no value to wait for)
                    if (IsActionItem(relevant->rowId)) {
                        m_settling = false;
                        std::string announcement = BuildAnnouncement(relevant->rowId, relevant->text, "");
                        SpeechManager::Get()->Speak(announcement, false);
                        Logger_Log("Option", "Initial item (action): '%s'", announcement.c_str());
                        return;
                    }
                } else if (relevant->rowId != m_lastHelpId) {
                    // User moved cursor before value arrived
                    m_settling = false;
                    m_lastHelpId = relevant->rowId;
                    m_rememberedHelpId[m_lastSubScreen] = relevant->rowId;
                    std::string valueText = FindValueText();
                    std::string announcement = BuildAnnouncement(relevant->rowId, relevant->text, valueText);
                    SpeechManager::Get()->Speak(announcement, true);
                    Logger_Log("Option", "Cursor moved (during settle): help=%d, '%s'",
                               relevant->rowId, announcement.c_str());
                    return;
                }
            }
        }

        // Action item set from cursor — announce immediately (no value to wait for)
        if (m_lastHelpId >= 0 && IsActionItem(m_lastHelpId)) {
            m_settling = false;
            std::string helpText = GameText_Lookup("help_message", m_lastHelpId);
            std::string announcement = BuildAnnouncement(m_lastHelpId, helpText, "");
            SpeechManager::Get()->Speak(announcement, false);
            Logger_Log("Option", "Initial item (action, from cursor): '%s'", announcement.c_str());
            return;
        }

        // Value arrived for focused item (need help ID first)
        if (m_lastHelpId >= 0) {
            std::string valueText = FindValueText();
            if (!valueText.empty()) {
                m_settling = false;
                m_cachedValue[m_lastHelpId] = valueText;
                std::string helpText = GameText_Lookup("help_message", m_lastHelpId);
                std::string announcement = BuildAnnouncement(m_lastHelpId, helpText, valueText);
                SpeechManager::Get()->Speak(announcement, false);
                Logger_Log("Option", "Initial item (settled): '%s'", announcement.c_str());
                return;
            }
        }

        // Keep waiting until help ID + value arrive or user moves cursor.
        return;
    }

    // --- ACTIVE: normal operation ---

    // Track Yes/No dialog overlay via YesNoHandler's state (frame-accurate).
    // YesNoHandler processes before OptionHandler each frame, so its
    // m_dialogActive is already up-to-date when we read it.
    {
        bool yesNoActive = YesNoHandler::Get()->IsDialogActive();

        if (yesNoActive && !m_yesNoShown) {
            m_yesNoShown = true;
            Logger_Log("Option", "Yes/No dialog opened (YesNoHandler active)");
        }
        else if (!yesNoActive && m_yesNoShown) {
            // Dialog just closed — re-announce current item immediately.
            // The game does NOT re-fire text events after the dialog closes
            // (screen is already rendered), so use the cached value.
            m_yesNoShown = false;
            Logger_Log("Option", "Yes/No dialog closed, re-announcing (help=%d)", m_lastHelpId);

            std::string helpText = GameText_Lookup("help_message", m_lastHelpId);
            auto it = m_cachedValue.find(m_lastHelpId);
            std::string valueText = (it != m_cachedValue.end()) ? it->second : "";
            std::string announcement = BuildAnnouncement(m_lastHelpId, helpText, valueText);
            SpeechManager::Get()->Speak(announcement, false);
            Logger_Log("Option", "Re-announced: '%s'", announcement.c_str());
            return;
        }

        if (m_yesNoShown) {
            // Dialog still open — drain events silently
            TextCapture::Get()->ConsumeOptionValues();
            TextCapture::Get()->ConsumeHelpMessages();
            return;
        }
    }

    // Normal value change detection
    {
        std::string valueText = FindValueText();
        if (!valueText.empty()) {
            m_cachedValue[m_lastHelpId] = valueText;
            SpeechManager::Get()->Speak(valueText, true);
            Logger_Log("Option", "Value changed: '%s'", valueText.c_str());
        }
    }

    auto helpEvents = TextCapture::Get()->ConsumeHelpMessages();

    // Cursor moved? Filter to events relevant to current sub-screen.
    if (!helpEvents.empty()) {
        const TextCapture::HelpMessageEvent* relevant = nullptr;
        for (auto& h : helpEvents) {
            if (GetPositionForHelp(m_lastSubScreen, h.rowId) >= 0)
                relevant = &h;
        }
        if (relevant) {
            if (relevant->rowId != m_lastHelpId) {
                m_lastHelpId = relevant->rowId;
                m_rememberedHelpId[m_lastSubScreen] = relevant->rowId;
                std::string valueText = FindValueText();
                if (!valueText.empty())
                    m_cachedValue[m_lastHelpId] = valueText;
                std::string announcement = BuildAnnouncement(relevant->rowId, relevant->text, valueText);
                SpeechManager::Get()->Speak(announcement, true);
                Logger_Log("Option", "Cursor moved: help=%d, announcement='%s'",
                           relevant->rowId, announcement.c_str());
            }
            return;
        }
    }
}
