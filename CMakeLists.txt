cmake_minimum_required(VERSION 3.16)
project(WorkshopMapLoader VERSION 1.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Suppress MSVC getenv deprecation warning ─────────────────────────────────
add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

# ── Fetch BakkesModSDK ────────────────────────────────────────────────────────
include(FetchContent)

FetchContent_Declare(
    BakkesModSDK
    GIT_REPOSITORY https://github.com/bakkesmodorg/BakkesModSDK.git
    GIT_TAG        master
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(BakkesModSDK)

# ── Extract imgui_bm.zip that ships inside the SDK ───────────────────────────
set(IMGUI_ZIP "${bakkesmodsdk_SOURCE_DIR}/imgui_bm.zip")
set(IMGUI_DIR "${CMAKE_BINARY_DIR}/imgui_bm")

if(NOT EXISTS "${IMGUI_DIR}/imgui/imgui.h")
    message(STATUS "Extracting imgui_bm.zip...")
    file(ARCHIVE_EXTRACT INPUT "${IMGUI_ZIP}" DESTINATION "${IMGUI_DIR}")
endif()

# ── Sources ───────────────────────────────────────────────────────────────────
add_library(WorkshopMapLoader SHARED
    WorkshopMapLoader.cpp
)

target_include_directories(WorkshopMapLoader PRIVATE
    ${bakkesmodsdk_SOURCE_DIR}/include
    ${IMGUI_DIR}           # gives us "imgui/imgui.h"
    ${IMGUI_DIR}/imgui     # gives us "imgui.h" directly
)

target_link_libraries(WorkshopMapLoader PRIVATE
    ${bakkesmodsdk_SOURCE_DIR}/lib/pluginsdk.lib
)

set_target_properties(WorkshopMapLoader PROPERTIES
    OUTPUT_NAME "WorkshopMapLoader"
    SUFFIX      ".dll"
)

if(MSVC)
    target_compile_options(WorkshopMapLoader PRIVATE /W3 /MP /EHsc)
    target_link_options(WorkshopMapLoader PRIVATE /EXPORT:CreatePlugin)
endif()

# ── Install ───────────────────────────────────────────────────────────────────
set(BAKKESMOD_PLUGINS_DIR "$ENV{APPDATA}/bakkesmod/bakkesmod/plugins"
    CACHE PATH "BakkesMod plugins directory")

install(TARGETS WorkshopMapLoader RUNTIME DESTINATION "${BAKKESMOD_PLUGINS_DIR}")
install(FILES WorkshopMapLoader.set DESTINATION "${BAKKESMOD_PLUGINS_DIR}")
