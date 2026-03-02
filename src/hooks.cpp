// VTable hooking infrastructure and per-frame handler dispatch.
//
// SwapBuffers (OpenGL) is hooked via MinHook to get a reliable per-frame
// callback on the game thread. Frame handlers register to receive OnFrame()
// calls, where they can poll UI state and announce changes.
//
// VTable hooks use VirtualProtect to swap individual vtable entries,
// following the same pattern as the Skyrim accessibility mod.

#include "hooks.h"
#include "logger.h"

#include <windows.h>
#include <MinHook.h>

#include <modloader/utils.h>

#include <vector>
#include <mutex>
#include <algorithm>

// ============================================================
// VTable hook helpers
// ============================================================

void* HookVTableEntry(void* object, int index, void* newFunc)
{
    if (!object || !newFunc) return nullptr;

    // First qword of the object is the vtable pointer
    uintptr_t* vtable = *reinterpret_cast<uintptr_t**>(object);
    uintptr_t* entry = &vtable[index];

    // Save original
    void* original = reinterpret_cast<void*>(*entry);

    // Make writable, swap, restore
    DWORD oldProtect;
    VirtualProtect(entry, sizeof(uintptr_t), PAGE_EXECUTE_READWRITE, &oldProtect);
    *entry = reinterpret_cast<uintptr_t>(newFunc);
    VirtualProtect(entry, sizeof(uintptr_t), oldProtect, &oldProtect);

    Logger_Log("Hooks", "VTable hook: [%d] %p -> %p", index, original, newFunc);
    return original;
}

void* HookVTableByAddress(uintptr_t vtableAddr, int index, void* newFunc)
{
    if (!vtableAddr || !newFunc) return nullptr;

    uintptr_t* entry = reinterpret_cast<uintptr_t*>(vtableAddr + index * sizeof(uintptr_t));

    void* original = reinterpret_cast<void*>(*entry);

    DWORD oldProtect;
    VirtualProtect(entry, sizeof(uintptr_t), PAGE_EXECUTE_READWRITE, &oldProtect);
    *entry = reinterpret_cast<uintptr_t>(newFunc);
    VirtualProtect(entry, sizeof(uintptr_t), oldProtect, &oldProtect);

    Logger_Log("Hooks", "VTable hook @ 0x%llx[%d]: %p -> %p",
               (unsigned long long)vtableAddr, index, original, newFunc);
    return original;
}

// ============================================================
// Frame handler registry
// ============================================================

static std::mutex g_handlerMutex;
static std::vector<IFrameHandler*> g_handlers;

void RegisterFrameHandler(IFrameHandler* handler)
{
    if (!handler) return;
    std::lock_guard<std::mutex> lock(g_handlerMutex);
    g_handlers.push_back(handler);
    Logger_Log("Hooks", "Registered frame handler (total: %zu)", g_handlers.size());
}

void UnregisterFrameHandler(IFrameHandler* handler)
{
    if (!handler) return;
    std::lock_guard<std::mutex> lock(g_handlerMutex);
    g_handlers.erase(
        std::remove(g_handlers.begin(), g_handlers.end(), handler),
        g_handlers.end());
    Logger_Log("Hooks", "Unregistered frame handler (total: %zu)", g_handlers.size());
}

// ============================================================
// SwapBuffers hook — per-frame dispatch
// ============================================================

using SwapBuffersFunc = BOOL(WINAPI*)(HDC);
static SwapBuffersFunc g_origSwapBuffers = nullptr;
static void* g_addrSwapBuffers = nullptr;
static int g_frameCount = 0;

static BOOL WINAPI hooked_SwapBuffers(HDC hdc)
{
    g_frameCount++;

    // Log occasionally to confirm hook is alive
    if (g_frameCount == 60) {
        Logger_Log("Hooks", "SwapBuffers hook active (frame %d)", g_frameCount);
    }

    // Dispatch to all registered handlers
    {
        std::lock_guard<std::mutex> lock(g_handlerMutex);
        for (auto* handler : g_handlers) {
            handler->OnFrame();
        }
    }

    return g_origSwapBuffers(hdc);
}

// ============================================================
// Public API
// ============================================================

bool hooks_init()
{
    Logger_Log("Hooks", "Initializing hooks...");

    MH_STATUS status = MH_Initialize();
    if (status != MH_OK) {
        Logger_Log("Hooks", "MinHook init failed: %d", status);
        return false;
    }

    char* base = getBaseOffset();
    Logger_Log("Hooks", "Game module base: %p", base);

    // Hook SwapBuffers for per-frame callbacks
    HMODULE hGdi32 = GetModuleHandleA("gdi32.dll");
    if (hGdi32) {
        g_addrSwapBuffers = (void*)GetProcAddress(hGdi32, "SwapBuffers");
    }

    if (!g_addrSwapBuffers) {
        HMODULE hOgl = GetModuleHandleA("opengl32.dll");
        if (hOgl) {
            g_addrSwapBuffers = (void*)GetProcAddress(hOgl, "wglSwapBuffers");
        }
    }

    if (g_addrSwapBuffers) {
        status = MH_CreateHook(g_addrSwapBuffers, (void*)&hooked_SwapBuffers,
                               (void**)&g_origSwapBuffers);
        if (status != MH_OK) {
            Logger_Log("Hooks", "Failed to create SwapBuffers hook: %d", status);
            MH_Uninitialize();
            return false;
        }
        Logger_Log("Hooks", "SwapBuffers hook created at %p", g_addrSwapBuffers);
    } else {
        Logger_Log("Hooks", "ERROR: SwapBuffers not found!");
        MH_Uninitialize();
        return false;
    }

    // Enable hooks
    status = MH_EnableHook(MH_ALL_HOOKS);
    if (status != MH_OK) {
        Logger_Log("Hooks", "Failed to enable hooks: %d", status);
        MH_Uninitialize();
        return false;
    }

    Logger_Log("Hooks", "Hook infrastructure ready. SwapBuffers hooked for per-frame dispatch.");
    return true;
}

void hooks_shutdown()
{
    Logger_Log("Hooks", "Shutting down hooks...");

    // Clear handlers first
    {
        std::lock_guard<std::mutex> lock(g_handlerMutex);
        g_handlers.clear();
    }

    MH_DisableHook(MH_ALL_HOOKS);
    MH_Uninitialize();

    g_origSwapBuffers = nullptr;
    g_addrSwapBuffers = nullptr;
    g_frameCount = 0;

    Logger_Log("Hooks", "Hooks shut down.");
}
