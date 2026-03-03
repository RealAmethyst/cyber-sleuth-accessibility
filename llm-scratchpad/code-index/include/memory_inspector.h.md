// File: include/memory_inspector.h
// Purpose: Development/research tool for discovering CUi* member offsets at runtime.
//          Handlers call SetActivePointer() from their hooked update functions;
//          pressing F5 dumps raw memory for all registered pointers to
//          CyberSleuth_memdump.log so offsets can be compared between cursor positions.

// Implements IFrameHandler — polls F5 each frame and triggers dumps on keypress.
class MemoryInspector : public IFrameHandler  // (line 18)
{
public:
    static MemoryInspector* Get();  // (line 21) — singleton accessor

    // Register a this pointer captured from a vtable hook under a class name label.
    // className should match the CUi class name, e.g. "CUiTitle".
    void SetActivePointer(const std::string& className, void* ptr);  // (line 25)

    // Remove a previously registered pointer (e.g. when a screen closes).
    void ClearPointer(const std::string& className);  // (line 26)

    // IFrameHandler: polls GetAsyncKeyState(VK_F5) each frame; dumps on press.
    void OnFrame() override;  // (line 29)

    // Set how many bytes to hex-dump per pointer (default 512).
    void SetDumpSize(size_t bytes);  // (line 31)

private:
    MemoryInspector() = default;  // (line 34)

    // Dump all registered pointers to file.
    void DumpAllPointers();  // (line 36)

    // Write a hex dump of `size` bytes at `ptr` to the file, labeled with `label`.
    void DumpMemory(FILE* fp, const std::string& label, void* ptr, size_t size);  // (line 37)

    std::mutex m_mutex;                                        // (line 39)
    std::unordered_map<std::string, void*> m_activePointers;  // (line 40)
    size_t m_dumpSize = 512;                                   // (line 41)
    bool m_f5WasDown = false;                                  // (line 42) — edge-detect to fire once per press
}
