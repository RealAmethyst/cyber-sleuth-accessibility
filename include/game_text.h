#pragma once

#include <string>

// Shared text lookup utility — wraps the game's LookupText API.
// Resolves the text table manager, language settings, and performs the lookup.
// Must be initialized once (call GameText_Init after hooks_init).

void GameText_Init();

// Returns the current language index (0=JP, 1=EN, 2=CN, 3=EN_Censor, 4=KR, 5=DE).
int GameText_GetLanguage();

// Look up text from any MBE table by table name and row ID.
// Returns the localized text string, or empty string on failure.
std::string GameText_Lookup(const char* tableName, int rowId);
