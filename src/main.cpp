#include <modloader/plugin.h>
#include <modloader/utils.h>
#include <windows.h>
#include "logger.h"
#include "speech_manager.h"
#include "hooks.h"
#include "memory_inspector.h"
#include "ui_probe.h"
#include "offsets.h"
#include "handlers/main_menu_handler.h"
#include "handlers/title_handler.h"

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
    Logger_Log("Main", "Plugin onEnable — v0.5.0 (vtable[3] tick hooks + text API)");

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

    // Register MemoryInspector for F5 hotkey polling
    RegisterFrameHandler(MemoryInspector::Get());

    // Install MainMenuHandler — hooks CUiMainMenu vtable[3] tick via MinHook,
    // reads cursor from this+0x27D8, announces menu items via text API.
    MainMenuHandler::Get()->Install();
    RegisterFrameHandler(MainMenuHandler::Get());

    // Probe a broad set of CUi classes to discover which are active.
    // MinHook patches function prologues (vtable[3] tick) so ALL calls are
    // intercepted regardless of dispatch mechanism.
    UiProbe::Get()->Install({
        // Title / startup screens
        {"CUiTitle",           Offsets::VTABLE_CUiTitle},
        {"CUiTitleLogo",       Offsets::VTABLE_CUiTitleLogo},
        {"CUiScenarioSelect",  Offsets::VTABLE_CUiScenarioSelect},
        {"CUiFirstSequence",   Offsets::VTABLE_CUiFirstSequence},

        // Main / top-level menus
        {"CUiMenuTop",         Offsets::VTABLE_CUiMenuTop},
        {"CUiMainMenu",        Offsets::VTABLE_CUiMainMenu},
        {"CUiBaseMenu",        Offsets::VTABLE_CUiBaseMenu},

        // System menus
        {"CUiOption",          Offsets::VTABLE_CUiOption},
        {"CUiSaveload",        Offsets::VTABLE_CUiSaveload},
        {"CUiSettingMenu",     Offsets::VTABLE_CUiSettingMenu},

        // Dialog / selection
        {"CUiTalkWindow",      Offsets::VTABLE_CUiTalkWindow},
        {"CUiYesNoWindow",     Offsets::VTABLE_CUiYesNoWindow},
        {"CUiMultiSelectWindow", Offsets::VTABLE_CUiMultiSelectWindow},
        {"CUiInfoWindow",      Offsets::VTABLE_CUiInfoWindow},

        // Field
        {"CUiField_001",       Offsets::VTABLE_CUiField_001},
        {"CUiField_002",       Offsets::VTABLE_CUiField_002},

        // Loading
        {"CUiLoading",         Offsets::VTABLE_CUiLoading},
    });

    Logger_Log("Main", "Plugin startup complete. Press F5 to dump active CUi memory.");
}

void CyberSleuthAccessibility::onDisable()
{
    Logger_Log("Main", "Plugin onDisable");

    UnregisterFrameHandler(MainMenuHandler::Get());
    MainMenuHandler::Get()->Uninstall();

    UnregisterFrameHandler(MemoryInspector::Get());
    UiProbe::Get()->Uninstall();

    hooks_shutdown();
    SpeechManager::Get()->Shutdown();
    Logger_Shutdown();
}

const PluginInfo CyberSleuthAccessibility::getPluginInfo()
{
    PluginInfo info;
    info.apiVersion = {0, 0, 0};
    info.version    = {0, 5, 0};
    info.name       = "Cyber Sleuth Accessibility";
    return info;
}

extern "C" __declspec(dllexport) CyberSleuthAccessibility* getPlugin(DSCSModLoader& modLoader)
{
    return new CyberSleuthAccessibility(modLoader);
}
