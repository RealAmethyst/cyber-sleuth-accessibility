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
static uintptr_t s_base = 0;

void GameText_Init()
{
    s_base = reinterpret_cast<uintptr_t>(getBaseOffset());

    s_getTextTableManager = reinterpret_cast<GetTextTableManagerFunc>(
        s_base + Offsets::Text::FUNC_GetTextTableManager);
    s_lookupText = reinterpret_cast<LookupTextFunc>(
        s_base + Offsets::Text::FUNC_LookupText);

    Logger_Log("GameText", "Initialized: GetTextTableManager=%p, LookupText=%p",
               (void*)s_getTextTableManager, (void*)s_lookupText);
}

// Read current language index live — player may change it in settings.
int GameText_GetLanguage()
{
    uintptr_t langSettings = *reinterpret_cast<uintptr_t*>(
        s_base + Offsets::Text::DAT_LanguageSettings);
    if (langSettings != 0) {
        return static_cast<int>(*reinterpret_cast<unsigned int*>(
            langSettings + Offsets::Text::LANGUAGE_INDEX_OFFSET));
    }
    return 1;  // Default to English
}

static unsigned int GetLanguage()
{
    uintptr_t langSettings = *reinterpret_cast<uintptr_t*>(
        s_base + Offsets::Text::DAT_LanguageSettings);
    if (langSettings != 0) {
        return *reinterpret_cast<unsigned int*>(
            langSettings + Offsets::Text::LANGUAGE_INDEX_OFFSET);
    }
    return 1;  // Default to English
}

std::string GameText_Lookup(const char* tableName, int rowId)
{
    if (!s_getTextTableManager || !s_lookupText) {
        return "";
    }

    void* manager = s_getTextTableManager();
    if (!manager) return "";

    const char* text = s_lookupText(manager, tableName, rowId, GetLanguage());
    if (text && text[0] != '\0') {
        return std::string(text);
    }
    return "";
}
