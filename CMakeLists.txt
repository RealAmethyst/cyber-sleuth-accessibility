cmake_minimum_required(VERSION 3.25)
project(CyberSleuthAccessibility LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 20)

# Disable Boost auto-linking — CMake handles linking already.
# Prevents vc143/vc145 mismatch on newer MSVC toolchains.
add_definitions(-DBOOST_ALL_NO_LIB)

include(cmake/CPM.cmake)

CPMAddPackage(
  NAME DSCSModLoader
  GITHUB_REPOSITORY "SydMontague/DSCSModLoader"
  GIT_TAG "33f18ca2e3112648ffa31633517c5b1da4d8501c"
)

# MinHook — no CMakeLists.txt, so we download source and build manually
CPMAddPackage(
  NAME minhook
  GITHUB_REPOSITORY "TsudaKageyu/minhook"
  GIT_TAG "v1.3.3"
  DOWNLOAD_ONLY YES
)

if(minhook_ADDED)
  add_library(minhook STATIC
    ${minhook_SOURCE_DIR}/src/buffer.c
    ${minhook_SOURCE_DIR}/src/hook.c
    ${minhook_SOURCE_DIR}/src/trampoline.c
    ${minhook_SOURCE_DIR}/src/hde/hde64.c
  )
  target_include_directories(minhook PUBLIC ${minhook_SOURCE_DIR}/include)
  target_include_directories(minhook PRIVATE ${minhook_SOURCE_DIR}/src)
endif()

# SRAL (Screen Reader Abstraction Layer) — replaces Tolk
# Configure SRAL options BEFORE CPM processes the subdirectory
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(BUILD_SRAL_TEST OFF CACHE BOOL "" FORCE)
set(SRAL_DISABLE_UIA ON CACHE BOOL "" FORCE)

CPMAddPackage(
  NAME SRAL
  GITHUB_REPOSITORY "m1maker/SRAL"
  GIT_TAG "main"
)

add_library(CyberSleuthAccessibility SHARED
  src/main.cpp
  src/logger.cpp
  src/speech_manager.cpp
  src/hooks.cpp
  src/plugin_util.cpp
  src/game_text.cpp
  src/memory_inspector.cpp
  src/ui_probe.cpp
  src/text_capture.cpp
  src/handlers/main_menu_handler.cpp
  src/handlers/title_handler.cpp
  src/handlers/subtitle_handler.cpp
  src/handlers/yesno_handler.cpp
  src/handlers/scenario_select_handler.cpp
)

target_include_directories(CyberSleuthAccessibility PRIVATE include)
target_compile_definitions(CyberSleuthAccessibility PRIVATE SRAL_STATIC)

# Link SRAL — try SRAL_static first (from SRAL's CMakeLists), fall back to SRAL
if(TARGET SRAL_static)
  target_link_libraries(CyberSleuthAccessibility DSCSModLoader minhook SRAL_static)
elseif(TARGET SRAL)
  target_link_libraries(CyberSleuthAccessibility DSCSModLoader minhook SRAL)
else()
  message(FATAL_ERROR "SRAL target not found — check CPM fetch")
endif()

# SRAL include path
if(SRAL_SOURCE_DIR)
  target_include_directories(CyberSleuthAccessibility PRIVATE ${SRAL_SOURCE_DIR}/Include)
elseif(sral_SOURCE_DIR)
  target_include_directories(CyberSleuthAccessibility PRIVATE ${sral_SOURCE_DIR}/Include)
endif()
