// File: src/main.cpp
// Purpose: DSCSModLoader plugin entry point — wires up all handlers and manages
//          plugin lifetime (onEnable / onDisable).

class CyberSleuthAccessibility : public BasePlugin  (line 16)
    void onEnable() override  (line 20)
    void onDisable() override  (line 21)
    const PluginInfo getPluginInfo() override  (line 22)

// onEnable: initialises Logger, SpeechManager, MinHook infrastructure, then
// registers handlers in order: MemoryInspector, TitleHandler, MainMenuHandler,
// TextCapture, SubtitleHandler, YesNoHandler, ScenarioSelectHandler.
void CyberSleuthAccessibility::onEnable()  (line 25)

// onDisable: unregisters and uninstalls all handlers in reverse order, then
// shuts down hooks, SpeechManager, and Logger.
void CyberSleuthAccessibility::onDisable()  (line 79)

// Returns plugin metadata (name, version). Note: version field says {0,8,0}
// but runtime log says v0.11.0 — version field has not been kept in sync.
const PluginInfo CyberSleuthAccessibility::getPluginInfo()  (line 108)

// DSCSModLoader DLL export — called by the mod loader to instantiate the plugin.
extern "C" __declspec(dllexport) CyberSleuthAccessibility* getPlugin(DSCSModLoader& modLoader)  (line 117)
