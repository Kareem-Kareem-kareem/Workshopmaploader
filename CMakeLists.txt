cmake_minimum_required(VERSION 3.20)
project(WorkshopMapLoader)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Automatically fetch the BakkesMod SDK via CMake during the GitHub runner session
include(FetchContent)
FetchContent_Declare(
    bakkesmodsdk
    GIT_REPOSITORY https://github.com/bakkesmodorg/BakkesModSDK.git
    GIT_TAG master
)
FetchContent_MakeAvailable(bakkesmodsdk)

# 2. Extract the bundled ImGui archive provided directly inside the official SDK
set(IMGUI_ZIP_PATH "${bakkesmodsdk_SOURCE_DIR}/imgui_bm.zip")
set(IMGUI_EXTRACT_DIR "${CMAKE_CURRENT_BINARY_DIR}/imgui_bm")

if(EXISTS "${IMGUI_ZIP_PATH}" AND NOT EXISTS "${IMGUI_EXTRACT_DIR}")
    file(ARCHIVE_EXTRACT INPUT "${IMGUI_ZIP_PATH}" DESTINATION "${IMGUI_EXTRACT_DIR}")
endif()

# 3. Explicitly pass the downloaded target SDK directory blocks to the include path
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${bakkesmodsdk_SOURCE_DIR}/include
    ${IMGUI_EXTRACT_DIR}
)

# 4. Map out the transient ImGui source paths generated inside the temporary build directory
set(IMGUI_SOURCES
    "${IMGUI_EXTRACT_DIR}/imgui/imgui.cpp"
    "${IMGUI_EXTRACT_DIR}/imgui/imgui_draw.cpp"
    "${IMGUI_EXTRACT_DIR}/imgui/imgui_widgets.cpp"
)

# 5. Build the library from your flat root workspace files + downloaded ImGui dependencies
add_library(${PROJECT_NAME} SHARED
    WorkshopMapLoader.cpp
    WorkshopMapLoader.h
    ${IMGUI_SOURCES}
)

# 6. Link against the precompiled core SDK library binary packaged with the repo fetch
target_link_libraries(${PROJECT_NAME} PRIVATE "${bakkesmodsdk_SOURCE_DIR}/lib/pluginsdk.lib")

if(MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE WIN32_LEAN_AND_MEAN _CRT_SECURE_NO_WARNINGS)
endif()
