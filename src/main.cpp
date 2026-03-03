#include <modloader/plugin.h>
#include <modloader/utils.h>
#include <windows.h>
#include "logger.h"
#include "speech_manager.h"
#include "hooks.h"
#include "memory_inspector.h"
#include "offsets.h"
#include "handlers/main_menu_handler.h"
#include "handlers/title_handler.h"
#include "handlers/subtitle_handler.h"
#include "handlers/yesno_handler.h"
#include "handlers/scenario_select_handler.h"
#include "text_capture.h"
#include "game_text.h"

class CyberSleuthAccessibility : public BasePlugin
{
public:
    using BasePlugin::BasePlugin;
    void onEnable() override;
    void onDisable() override;
    const PluginInfo getPluginInfo() override;
};

void CyberSleuthAccessibility::onEnable()
{
    Logger_Init();
    Logger_Log("Main", "Plugin onEnable — v0.11.0 (ScenarioSelectHandler)");

    auto* speech = SpeechManager::Get();
    if (speech->Initialize()) {
        speech->Speak("Cyber Sleuth Accessibility loaded", true);
    } else {
        Logger_Log("Main", "SpeechManager init failed — no screen reader output");
    }

    if (!hooks_init()) {
        Logger_Log("Main", "Hook initialization FAILED");
        speech->Speak("Warning: hook initialization failed", true);
        return;
    }

    // Initialize shared text lookup utility (must be after hooks_init so module base is valid)
    GameText_Init();

    // Register MemoryInspector for F5 hotkey polling
    RegisterFrameHandler(MemoryInspector::Get());

    // Install TitleHandler — hooks CUiTitle tick via MinHook,
    // tracks state (+0xa8) and cursor (+0x114) for title menu.
    TitleHandler::Get()->Install();
    RegisterFrameHandler(TitleHandler::Get());

    // Install MainMenuHandler — hooks CUiMainMenu tick via MinHook,
    // reads cursor from this+0x27D8, announces menu items via text API.
    MainMenuHandler::Get()->Install();
    RegisterFrameHandler(MainMenuHandler::Get());

    // Install TextCapture — hooks LookupText to capture all text the game
    // fetches from MBE tables (subtitles, dialogs, info messages, etc.).
    TextCapture::Get()->Install();
    RegisterFrameHandler(TextCapture::Get());

    // Install SubtitleHandler — polls Vista singleton for subtitle player,
    // reads elapsed time + schedule, speaks active subtitle line.
    SubtitleHandler::Get()->Install();
    RegisterFrameHandler(SubtitleHandler::Get());

    // Install YesNoHandler — hooks CUiYesNoWindow tick via MinHook,
    // reads state + cursor, announces dialog message and selection.
    YesNoHandler::Get()->Install();
    RegisterFrameHandler(YesNoHandler::Get());

    // Install ScenarioSelectHandler — hooks CUiScenarioSelect tick via MinHook,
    // uses TextCapture for campaign descriptions and prompt text.
    ScenarioSelectHandler::Get()->Install();
    RegisterFrameHandler(ScenarioSelectHandler::Get());

    Logger_Log("Main", "Plugin startup complete. Press F5 to dump active CUi memory.");
}

void CyberSleuthAccessibility::onDisable()
{
    Logger_Log("Main", "Plugin onDisable");

    UnregisterFrameHandler(ScenarioSelectHandler::Get());
    ScenarioSelectHandler::Get()->Uninstall();

    UnregisterFrameHandler(YesNoHandler::Get());
    YesNoHandler::Get()->Uninstall();

    UnregisterFrameHandler(SubtitleHandler::Get());
    SubtitleHandler::Get()->Uninstall();

    UnregisterFrameHandler(TextCapture::Get());
    TextCapture::Get()->Uninstall();

    UnregisterFrameHandler(MainMenuHandler::Get());
    MainMenuHandler::Get()->Uninstall();

    UnregisterFrameHandler(TitleHandler::Get());
    TitleHandler::Get()->Uninstall();

    UnregisterFrameHandler(MemoryInspector::Get());

    hooks_shutdown();
    SpeechManager::Get()->Shutdown();
    Logger_Shutdown();
}

const PluginInfo CyberSleuthAccessibility::getPluginInfo()
{
    PluginInfo info;
    info.apiVersion = {0, 0, 0};
    info.version    = {0, 8, 0};
    info.name       = "Cyber Sleuth Accessibility";
    return info;
}

extern "C" __declspec(dllexport) CyberSleuthAccessibility* getPlugin(DSCSModLoader& modLoader)
{
    return new CyberSleuthAccessibility(modLoader);
}
