// File: src/memory_inspector.cpp
// Purpose: Runtime memory inspector — dumps CUi* object memory to
//          CyberSleuth_memdump.log on F5 keypress (detected via GetAsyncKeyState
//          in OnFrame). Designed for offset discovery: take two dumps with
//          different cursor positions, diff them, and the changed int32 offset
//          is the cursor index.

// Singleton accessor.
MemoryInspector* MemoryInspector::Get()  (line 14)

// Registers (or updates) a named pointer for F5 dump. Thread-safe.
void MemoryInspector::SetActivePointer(const std::string& className, void* ptr)  (line 20)

// Removes a named pointer from the active set. Thread-safe.
void MemoryInspector::ClearPointer(const std::string& className)  (line 26)

// IFrameHandler::OnFrame — polls F5 key state each frame; calls DumpAllPointers()
// on leading edge (key just pressed).
void MemoryInspector::OnFrame()  (line 32)

// Opens CyberSleuth_memdump.log in append mode, writes a timestamped header,
// then calls DumpMemory() for each registered pointer. Speaks a count confirmation.
void MemoryInspector::DumpAllPointers()  (line 43)

// Writes a hex+ASCII dump and int32/pointer interpretations of [size] bytes at [ptr]
// to [fp]. Checks VirtualQuery before reading to avoid faulting on bad pointers.
// Outputs three sections: hex dump, int32 values (first 256 bytes), pointer values
// (first 256 bytes).
void MemoryInspector::DumpMemory(FILE* fp, const std::string& label,
                                  void* ptr, size_t size)  (line 93)
