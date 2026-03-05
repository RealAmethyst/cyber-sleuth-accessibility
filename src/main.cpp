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
#include "handlers/title_logo_handler.h"
#include "handlers/option_handler.h"
#include "handlers/talk_handler.h"
#include "handlers/talk_comm_handler.h"
#include "handlers/player_setting_handler.h"
#include "text_capture.h"
#include "game_text.h"

#include <functional>
#include <vector>

// Handler entry: frame handler + optional install/uninstall callbacks.
// Handlers without Install/Uninstall (e.g. MemoryInspector) use empty lambdas.
struct HandlerEntry {
    IFrameHandler* handler;
    std::function<void()> install;
    std::function<void()> uninstall;
};

// All handlers in registration order.
// To add a new handler: add one line here. That's it.
static std::vector<HandlerEntry> GetHandlers()
{
    return {
        { MemoryInspector::Get(),        {}, {} },
        { TitleLogoHandler::Get(),       [](){ TitleLogoHandler::Get()->Install(); },
                                         [](){ TitleLogoHandler::Get()->Uninstall(); } },
        { TextCapture::Get(),            [](){ TextCapture::Get()->Install(); },
                                         [](){ TextCapture::Get()->Uninstall(); } },
        { TitleHandler::Get(),           [](){ TitleHandler::Get()->Install(); },
                                         [](){ TitleHandler::Get()->Uninstall(); } },
        { MainMenuHandler::Get(),        [](){ MainMenuHandler::Get()->Install(); },
                                         [](){ MainMenuHandler::Get()->Uninstall(); } },
        { SubtitleHandler::Get(),        [](){ SubtitleHandler::Get()->Install(); },
                                         [](){ SubtitleHandler::Get()->Uninstall(); } },
        { YesNoHandler::Get(),           [](){ YesNoHandler::Get()->Install(); },
                                         [](){ YesNoHandler::Get()->Uninstall(); } },
        { ScenarioSelectHandler::Get(),  [](){ ScenarioSelectHandler::Get()->Install(); },
                                         [](){ ScenarioSelectHandler::Get()->Uninstall(); } },
        // OptionHandler hooks state handler functions (not the tick) that only fire
        // when the menu is interactive. Safe to install permanently — zero overhead
        // when menu is closed. See option_handler.h for details.
        { OptionHandler::Get(),          [](){ OptionHandler::Get()->Install(); },
                                         [](){ OptionHandler::Get()->Uninstall(); } },
        { TalkHandler::Get(),            [](){ TalkHandler::Get()->Install(); },
                                         [](){ TalkHandler::Get()->Uninstall(); } },
        { TalkCommHandler::Get(),        [](){ TalkCommHandler::Get()->Install(); },
                                         [](){ TalkCommHandler::Get()->Uninstall(); } },
        { PlayerSettingHandler::Get(),   [](){ PlayerSettingHandler::Get()->Install(); },
                                         [](){ PlayerSettingHandler::Get()->Uninstall(); } },
    };
}

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
    Logger_Log("Main", "Plugin onEnable — v0.11.0");

    auto* speech = SpeechManager::Get();
    if (!speech->Initialize()) {
        Logger_Log("Main", "SpeechManager init failed — no screen reader output");
    }

    if (!hooks_init()) {
        Logger_Log("Main", "Hook initialization FAILED");
        speech->Speak("Warning: hook initialization failed", true);
        return;
    }

    GameText_Init();

    auto handlers = GetHandlers();
    for (auto& entry : handlers) {
        if (entry.install) entry.install();
        RegisterFrameHandler(entry.handler);
    }

    Logger_Log("Main", "Plugin startup complete — %zu handlers active.", handlers.size());
}

void CyberSleuthAccessibility::onDisable()
{
    Logger_Log("Main", "Plugin onDisable");

    // Unregister in reverse order
    auto handlers = GetHandlers();
    for (auto it = handlers.rbegin(); it != handlers.rend(); ++it) {
        UnregisterFrameHandler(it->handler);
        if (it->uninstall) it->uninstall();
    }

    hooks_shutdown();
    SpeechManager::Get()->Shutdown();
    Logger_Shutdown();
}

const PluginInfo CyberSleuthAccessibility::getPluginInfo()
{
    PluginInfo info;
    info.apiVersion = {0, 0, 0};
    info.version    = {0, 11, 0};
    info.name       = "Cyber Sleuth Accessibility";
    return info;
}

extern "C" __declspec(dllexport) CyberSleuthAccessibility* getPlugin(DSCSModLoader& modLoader)
{
    return new CyberSleuthAccessibility(modLoader);
}
