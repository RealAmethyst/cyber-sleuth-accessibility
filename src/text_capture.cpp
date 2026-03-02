// TextCapture — hooks the game's core text lookup function to capture
// every text string fetched from MBE tables.
//
// Hook target: FUN_1401b9260 (RVA 0x1b9260)
// Signature: const char* LookupText(void* manager, const char* tableName,
//                                    int rowId, unsigned int language)
//
// Every time the game looks up text (menu items, dialog, UI labels),
// we record {tableName, rowId, text}. At end of frame (OnFrame via
// SwapBuffers), we diff against the previous frame and log NEW entries.

#include "text_capture.h"
#include "offsets.h"
#include "logger.h"

#include <modloader/utils.h>
#include <MinHook.h>
#include <windows.h>

// ============================================================
// Singleton
// ============================================================

TextCapture* TextCapture::Get()
{
    static TextCapture instance;
    return &instance;
}

// ============================================================
// Hook installation
// ============================================================

void TextCapture::Install()
{
    if (m_installed) return;

    uintptr_t base = reinterpret_cast<uintptr_t>(getBaseOffset());

    s_hookTarget = reinterpret_cast<void*>(base + Offsets::Text::FUNC_LookupText);

    MH_STATUS status = MH_CreateHook(
        s_hookTarget,
        reinterpret_cast<void*>(&HookedLookupText),
        reinterpret_cast<void**>(&s_originalLookupText));

    if (status != MH_OK) {
        Logger_Log("TextCapture", "MH_CreateHook failed for LookupText @ 0x%llx: %d",
                   (unsigned long long)(base + Offsets::Text::FUNC_LookupText), status);
        return;
    }

    status = MH_EnableHook(s_hookTarget);
    if (status != MH_OK) {
        Logger_Log("TextCapture", "MH_EnableHook failed: %d", status);
        MH_RemoveHook(s_hookTarget);
        return;
    }

    m_installed = true;
    Logger_Log("TextCapture", "Hooked LookupText (RVA 0x%llx) — capturing all text lookups",
               (unsigned long long)Offsets::Text::FUNC_LookupText);
}

void TextCapture::Uninstall()
{
    if (!m_installed) return;

    if (s_hookTarget) {
        MH_DisableHook(s_hookTarget);
        MH_RemoveHook(s_hookTarget);
    }

    s_originalLookupText = nullptr;
    s_hookTarget = nullptr;
    m_installed = false;

    {
        std::lock_guard<std::mutex> lock(s_captureMutex);
        s_currentFrameCaptures.clear();
    }
    m_previousFrame.clear();

    Logger_Log("TextCapture", "Uninstalled");
}

// ============================================================
// Hooked LookupText — called by the game every time it fetches text
// ============================================================

const char* __fastcall TextCapture::HookedLookupText(
    void* manager, const char* tableName, int rowId, unsigned int language)
{
    // Call original first — we need the return value
    const char* result = nullptr;
    if (s_originalLookupText) {
        result = s_originalLookupText(manager, tableName, rowId, language);
    }

    // Record the lookup (skip nulls and empty table names)
    if (tableName && tableName[0] != '\0') {
        TextEntry entry;
        entry.tableName = tableName;
        entry.rowId = rowId;
        entry.text = (result && result[0] != '\0') ? result : "";

        // Capture subtitle_text entries in order for SubtitleHandler
        if (entry.tableName == "subtitle_text" && !entry.text.empty()) {
            auto* self = TextCapture::Get();
            std::lock_guard<std::mutex> lock(self->m_subtitleMutex);
            // Only add if not already present (avoid duplicates from re-lookups)
            bool found = false;
            for (auto& existing : self->m_subtitleTexts) {
                if (existing.rowId == rowId) { found = true; break; }
            }
            if (!found) {
                self->m_subtitleTexts.push_back({rowId, entry.text});
            }
        }

        std::lock_guard<std::mutex> lock(s_captureMutex);
        s_currentFrameCaptures.push_back(std::move(entry));
    }

    return result;
}

// ============================================================
// Subtitle text access
// ============================================================

std::vector<TextCapture::SubtitleTextEntry> TextCapture::GetSubtitleTexts() const
{
    std::lock_guard<std::mutex> lock(m_subtitleMutex);
    return m_subtitleTexts;
}

void TextCapture::ClearSubtitleTexts()
{
    std::lock_guard<std::mutex> lock(m_subtitleMutex);
    m_subtitleTexts.clear();
}

// ============================================================
// Per-frame diffing — called from SwapBuffers hook
// ============================================================

void TextCapture::OnFrame()
{
    // Swap out the capture buffer
    std::vector<TextEntry> captures;
    {
        std::lock_guard<std::mutex> lock(s_captureMutex);
        captures.swap(s_currentFrameCaptures);
    }

    if (captures.empty() && m_previousFrame.empty()) {
        return; // Nothing happening
    }

    // Build current frame set
    std::unordered_set<TextEntry, TextEntryHash> currentFrame;
    for (auto& entry : captures) {
        currentFrame.insert(std::move(entry));
    }

    // On first frame with data, just log everything and establish baseline
    if (m_firstFrame && !currentFrame.empty()) {
        m_firstFrame = false;
        Logger_Log("TextCapture", "First frame: %zu unique text lookups", currentFrame.size());
        for (auto& entry : currentFrame) {
            Logger_Log("TextCapture", "  [%s:%d] \"%s\"",
                       entry.tableName.c_str(), entry.rowId,
                       entry.text.c_str());
        }
        m_previousFrame = std::move(currentFrame);
        return;
    }

    // Find NEW entries (in current but not in previous)
    for (auto& entry : currentFrame) {
        if (m_previousFrame.find(entry) == m_previousFrame.end()) {
            Logger_Log("TextCapture", "NEW: [%s:%d] \"%s\"",
                       entry.tableName.c_str(), entry.rowId,
                       entry.text.c_str());
        }
    }

    // Find REMOVED entries (in previous but not in current)
    for (auto& entry : m_previousFrame) {
        if (currentFrame.find(entry) == currentFrame.end()) {
            Logger_Log("TextCapture", "GONE: [%s:%d] \"%s\"",
                       entry.tableName.c_str(), entry.rowId,
                       entry.text.c_str());
        }
    }

    m_previousFrame = std::move(currentFrame);
}
