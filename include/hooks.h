#pragma once

// --- Frame handler system ---

// Handler interface: called once per frame from the SwapBuffers hook.
class IFrameHandler {
public:
    virtual ~IFrameHandler() = default;
    virtual void OnFrame() = 0;
};

void RegisterFrameHandler(IFrameHandler* handler);
void UnregisterFrameHandler(IFrameHandler* handler);

// --- Lifecycle ---

// Initialize hooks (MinHook + SwapBuffers frame callback).
// Call from onEnable() after SpeechManager is initialized.
bool hooks_init();

// Remove all hooks and clean up.
// Call from onDisable().
void hooks_shutdown();
