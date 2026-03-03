#pragma once

#include <string>

// Returns the directory containing our plugin DLL (with trailing backslash).
// Caches the result after first call.
std::string GetPluginDir();
