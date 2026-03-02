#include <modloader/plugin.h>
#include <modloader/utils.h>
#include <windows.h>
#include "logger.h"
#include "speech_manager.h"
#include "hooks.h"
#include "handlers/main_menu_handler.h"

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
    Logger_Log("Main", "Plugin onEnable — v0.2.0 (vtable hooks + SRAL)");

    auto* speech = SpeechManager::Get();
    if (speech->Initialize()) {
        speech->Speak("Cyber Sleuth Accessibility loaded", true);
    } else {
        Logger_Log("Main", "SpeechManager init failed — no screen reader output");
    }

    if (!hooks_init()) {
        Logger_Log("Main", "Hook initialization FAILED");
        speech->Speak("Warning: hook initialization failed", true);
    }

    // Register the main menu handler for per-frame callbacks
    RegisterFrameHandler(MainMenuHandler::Get());

    Logger_Log("Main", "Plugin startup complete");
}

void CyberSleuthAccessibility::onDisable()
{
    Logger_Log("Main", "Plugin onDisable");

    // Unregister handlers
    UnregisterFrameHandler(MainMenuHandler::Get());
    MainMenuHandler::Get()->Uninstall();

    hooks_shutdown();
    SpeechManager::Get()->Shutdown();
    Logger_Shutdown();
}

const PluginInfo CyberSleuthAccessibility::getPluginInfo()
{
    PluginInfo info;
    info.apiVersion = {0, 0, 0};
    info.version    = {0, 2, 0};
    info.name       = "Cyber Sleuth Accessibility";
    return info;
}

extern "C" __declspec(dllexport) CyberSleuthAccessibility* getPlugin(DSCSModLoader& modLoader)
{
    return new CyberSleuthAccessibility(modLoader);
}
