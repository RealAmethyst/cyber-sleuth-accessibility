#include "plugin_util.h"
#include <windows.h>

std::string GetPluginDir()
{
    static std::string cachedDir;
    if (!cachedDir.empty()) return cachedDir;

    HMODULE hSelf = nullptr;
    GetModuleHandleExA(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (LPCSTR)&GetPluginDir,
        &hSelf);

    char dllPath[MAX_PATH] = {};
    if (hSelf) {
        GetModuleFileNameA(hSelf, dllPath, MAX_PATH);
    }

    if (dllPath[0]) {
        char* lastSlash = strrchr(dllPath, '\\');
        if (lastSlash) {
            *(lastSlash + 1) = '\0';
            cachedDir = dllPath;
            return cachedDir;
        }
    }

    // Fallback: current directory
    cachedDir = ".\\";
    return cachedDir;
}
