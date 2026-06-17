cmake_minimum_required(VERSION 3.16)
project(WorkshopMapLoader VERSION 1.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Auto-fetch BakkesModSDK (no manual download needed) ───────────────────────
include(FetchContent)

FetchContent_Declare(
    BakkesModSDK
    GIT_REPOSITORY https://github.com/bakkesmodorg/BakkesModSDK.git
    GIT_TAG        master
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(BakkesModSDK)

# ── Sources ───────────────────────────────────────────────────────────────────
set(SOURCES
    src/WorkshopMapLoader.cpp
)

add_library(WorkshopMapLoader SHARED ${SOURCES})

target_include_directories(WorkshopMapLoader PRIVATE
    ${bakkesmodsdk_SOURCE_DIR}/include
    src
)

target_link_libraries(WorkshopMapLoader PRIVATE
    ${bakkesmodsdk_SOURCE_DIR}/lib/BakkesMod.lib
)

set_target_properties(WorkshopMapLoader PROPERTIES
    OUTPUT_NAME "WorkshopMapLoader"
    SUFFIX      ".dll"
)

if(MSVC)
    target_compile_options(WorkshopMapLoader PRIVATE /W3 /MP /EHsc)
    target_link_options(WorkshopMapLoader PRIVATE /EXPORT:CreatePlugin)
endif()

# ── Install to BakkesMod plugins folder ──────────────────────────────────────
set(BAKKESMOD_PLUGINS_DIR "$ENV{APPDATA}/bakkesmod/bakkesmod/plugins"
    CACHE PATH "BakkesMod plugins directory")

install(TARGETS WorkshopMapLoader
        RUNTIME DESTINATION "${BAKKESMOD_PLUGINS_DIR}")
install(FILES WorkshopMapLoader.set
        DESTINATION "${BAKKESMOD_PLUGINS_DIR}")
