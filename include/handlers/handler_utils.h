#pragma once

#include <cstdint>
#include <string>

// Shared utilities for UI handlers.

namespace HandlerUtils {

// Type-safe memory read from a game object at a given offset.
template<typename T>
inline T ReadMemory(void* thisPtr, uintptr_t offset)
{
    auto* ptr = reinterpret_cast<uint8_t*>(thisPtr);
    return *reinterpret_cast<T*>(ptr + offset);
}

// Format a standard announcement: "itemName, N of M"
// cursorIndex is 0-based, displayed as 1-based.
inline std::string FormatAnnouncement(const std::string& itemName,
                                       int cursorIndex,
                                       int totalItems)
{
    return itemName + ", " +
        std::to_string(cursorIndex + 1) + " of " + std::to_string(totalItems);
}

// Format with an optional description: "itemName, description, N of M"
inline std::string FormatAnnouncementWithDesc(const std::string& itemName,
                                               const std::string& description,
                                               int cursorIndex,
                                               int totalItems)
{
    std::string result = itemName;
    if (!description.empty()) {
        result += ", " + description;
    }
    result += ", " + std::to_string(cursorIndex + 1) + " of " + std::to_string(totalItems);
    return result;
}

}  // namespace HandlerUtils
