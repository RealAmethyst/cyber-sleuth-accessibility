#include "game_text.h"
#include "offsets.h"
#include "logger.h"

#include <modloader/utils.h>
#include <windows.h>

using GetTextTableManagerFunc = void* (__fastcall*)();
using LookupTextFunc = const char* (__fastcall*)(void* manager, const char* tableName,
                                                  int rowId, unsigned int language);

static GetTextTableManagerFunc s_getTextTableManager = nullptr;
static LookupTextFunc s_lookupText = nullptr;

void GameText_Init()
{
    uintptr_t base = reinterpret_cast<uintptr_t>(getBaseOffset());

    s_getTextTableManager = reinterpret_cast<GetTextTableManagerFunc>(
        base + Offsets::Text::FUNC_GetTextTableManager);
    s_lookupText = reinterpret_cast<LookupTextFunc>(
        base + Offsets::Text::FUNC_LookupText);

    Logger_Log("GameText", "Initialized: GetTextTableManager=%p, LookupText=%p",
               (void*)s_getTextTableManager, (void*)s_lookupText);
}

std::string GameText_Lookup(const char* tableName, int rowId)
{
    if (!s_getTextTableManager || !s_lookupText) {
        return "";
    }

    uintptr_t base = reinterpret_cast<uintptr_t>(getBaseOffset());

    // Get language index from the language settings singleton
    unsigned int language = 1; // Default to English
    uintptr_t langSettings = *reinterpret_cast<uintptr_t*>(
        base + Offsets::Text::DAT_LanguageSettings);
    if (langSettings != 0) {
        language = *reinterpret_cast<unsigned int*>(
            langSettings + Offsets::Text::LANGUAGE_INDEX_OFFSET);
    }

    void* manager = s_getTextTableManager();
    if (!manager) return "";

    const char* text = s_lookupText(manager, tableName, rowId, language);
    if (text && text[0] != '\0') {
        return std::string(text);
    }
    return "";
}
