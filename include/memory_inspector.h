#pragma once

#include "hooks.h"

#include <cstdint>
#include <cstdio>
#include <string>
#include <mutex>
#include <unordered_map>

// Runtime memory inspector for discovering CUi* member offsets.
//
// Usage:
// 1. Handlers call SetActivePointer() from their hooked update functions
// 2. MemoryInspector polls F5 each frame via OnFrame()
// 3. On F5 press, dumps all active pointers to CyberSleuth_memdump.log
// 4. Compare dumps between cursor positions to find the offset that changes
class MemoryInspector : public IFrameHandler
{
public:
    static MemoryInspector* Get();

    // Register/clear a this pointer captured from a vtable hook.
    // className should match the CUi class name (e.g., "CUiTitle").
    void SetActivePointer(const std::string& className, void* ptr);
    void ClearPointer(const std::string& className);

    // IFrameHandler -- polls F5 each frame, dumps on keypress
    void OnFrame() override;

    void SetDumpSize(size_t bytes) { m_dumpSize = bytes; }

private:
    MemoryInspector() = default;

    void DumpAllPointers();
    void DumpMemory(FILE* fp, const std::string& label, void* ptr, size_t size);

    std::mutex m_mutex;
    std::unordered_map<std::string, void*> m_activePointers;
    size_t m_dumpSize = 512;
    bool m_f5WasDown = false;
};
