// Runtime memory inspector -- dumps CUi* object memory on F5 hotkey.
//
// Designed for offset discovery: take two dumps with different cursor
// positions, diff them, and the offset that changed by 1 is the cursor index.

#include "memory_inspector.h"
#include "speech_manager.h"
#include "logger.h"

#include <windows.h>
#include <cstdio>
#include <algorithm>

MemoryInspector* MemoryInspector::Get()
{
    static MemoryInspector instance;
    return &instance;
}

void MemoryInspector::SetActivePointer(const std::string& className, void* ptr)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_activePointers[className] = ptr;
}

void MemoryInspector::ClearPointer(const std::string& className)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_activePointers.erase(className);
}

void MemoryInspector::OnFrame()
{
    bool f5Down = (GetAsyncKeyState(VK_F5) & 0x8000) != 0;

    if (f5Down && !m_f5WasDown) {
        DumpAllPointers();
    }

    m_f5WasDown = f5Down;
}

void MemoryInspector::DumpAllPointers()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_activePointers.empty()) {
        Logger_Log("MemInspect", "F5 pressed -- no active CUi pointers to dump");
        return;
    }

    // Build log path next to our DLL
    char dllPath[MAX_PATH] = {};
    HMODULE hSelf = nullptr;
    GetModuleHandleExA(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
        GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (LPCSTR)&MemoryInspector::Get, &hSelf);
    GetModuleFileNameA(hSelf, dllPath, MAX_PATH);

    std::string logPath(dllPath);
    auto pos = logPath.find_last_of("\\/");
    if (pos != std::string::npos)
        logPath = logPath.substr(0, pos + 1);
    logPath += "CyberSleuth_memdump.log";

    FILE* fp = fopen(logPath.c_str(), "a");
    if (!fp) {
        Logger_Log("MemInspect", "Cannot open %s", logPath.c_str());
        return;
    }

    // Timestamp header
    SYSTEMTIME st;
    GetLocalTime(&st);
    fprintf(fp, "\n===== DUMP %02d:%02d:%02d.%03d =====\n",
            st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

    for (auto& [name, ptr] : m_activePointers) {
        if (ptr) {
            DumpMemory(fp, name, ptr, m_dumpSize);
        }
    }

    fclose(fp);
    Logger_Log("MemInspect", "Dumped %zu pointer(s) to %s",
               m_activePointers.size(), logPath.c_str());

    // Speech confirmation
    SpeechManager::Get()->Speak("Dumped " + std::to_string(m_activePointers.size()) + " objects", true);
}

void MemoryInspector::DumpMemory(FILE* fp, const std::string& label,
                                  void* ptr, size_t size)
{
    fprintf(fp, "\n--- %s @ %p (%zu bytes) ---\n", label.c_str(), ptr, size);

    // Verify memory is readable
    MEMORY_BASIC_INFORMATION mbi = {};
    if (!VirtualQuery(ptr, &mbi, sizeof(mbi)) ||
        mbi.State != MEM_COMMIT ||
        (mbi.Protect & (PAGE_NOACCESS | PAGE_GUARD))) {
        fprintf(fp, "  <memory not readable>\n");
        return;
    }

    const uint8_t* bytes = static_cast<const uint8_t*>(ptr);

    // --- Hex dump with ASCII sidebar ---
    for (size_t offset = 0; offset < size; offset += 16) {
        fprintf(fp, "  +0x%04zx: ", offset);

        size_t lineLen = (std::min)((size_t)16, size - offset);

        // Hex bytes
        for (size_t i = 0; i < 16; i++) {
            if (i < lineLen)
                fprintf(fp, "%02x ", bytes[offset + i]);
            else
                fprintf(fp, "   ");
            if (i == 7) fprintf(fp, " ");
        }

        // ASCII
        fprintf(fp, " |");
        for (size_t i = 0; i < lineLen; i++) {
            uint8_t c = bytes[offset + i];
            fprintf(fp, "%c", (c >= 32 && c < 127) ? (char)c : '.');
        }
        fprintf(fp, "|\n");
    }

    // --- Int32 interpretation (first 256 bytes, most likely range for cursor) ---
    size_t intBytes = (std::min)(size, (size_t)256);
    size_t intCount = intBytes / 4;
    fprintf(fp, "\n  --- Int32 values (first %zu bytes) ---\n", intBytes);

    const int32_t* ints = reinterpret_cast<const int32_t*>(bytes);
    for (size_t i = 0; i < intCount; i++) {
        fprintf(fp, "  +0x%04zx: %d", i * 4, ints[i]);
        // Also show as hex if large
        if (ints[i] > 255 || ints[i] < -1)
            fprintf(fp, "  (0x%08x)", static_cast<uint32_t>(ints[i]));
        fprintf(fp, "\n");
    }

    // --- Pointer-sized values (first 256 bytes) ---
    size_t ptrBytes = (std::min)(size, (size_t)256);
    size_t ptrCount = ptrBytes / 8;
    fprintf(fp, "\n  --- Pointer values (first %zu bytes) ---\n", ptrBytes);

    const uint64_t* ptrs = reinterpret_cast<const uint64_t*>(bytes);
    for (size_t i = 0; i < ptrCount; i++) {
        fprintf(fp, "  +0x%04zx: 0x%016llx\n", i * 8,
                (unsigned long long)ptrs[i]);
    }
}
