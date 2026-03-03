// File: CMakeLists.txt
// Purpose: Build configuration for the CyberSleuthAccessibility DLL plugin

cmake_minimum_required(VERSION 3.25)  (line 1)
project(CyberSleuthAccessibility LANGUAGES CXX C)  (line 2)
set(CMAKE_CXX_STANDARD 20)  (line 3)

// Boost auto-link disable to prevent vc143/vc145 mismatch
add_definitions(-DBOOST_ALL_NO_LIB)  (line 7)

// Dependencies via CPM:
CPMAddPackage DSCSModLoader @ git tag 33f18ca...  (line 11)
CPMAddPackage minhook v1.3.3 (DOWNLOAD_ONLY, manual static lib build)  (line 18)
CPMAddPackage SRAL @ main (static, UIA disabled, tests off)  (line 42)

// Output target
add_library(CyberSleuthAccessibility SHARED ...)  (line 48)
// Sources: main.cpp, logger.cpp, speech_manager.cpp, hooks.cpp,
//   memory_inspector.cpp, ui_probe.cpp, text_capture.cpp,
//   handlers/{main_menu,title,subtitle,yesno,scenario_select}_handler.cpp

target_compile_definitions(... SRAL_STATIC)  (line 64)
target_link_libraries(... DSCSModLoader minhook SRAL_static)  (line 68)
