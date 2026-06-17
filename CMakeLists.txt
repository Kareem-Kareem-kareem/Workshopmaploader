cmake_minimum_required(VERSION 3.20)
project(WorkshopMapLoader)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Fetch the official BakkesMod SDK automatically in the GitHub Actions runner environment
include(FetchContent)
FetchContent_Declare(
    bakkesmodsdk
    GIT_REPOSITORY https://github.com/bakkesmodorg/BakkesModSDK.git
    GIT_TAG master
)
FetchContent_MakeAvailable(bakkesmodsdk)

# 2. Extract the SDK's internal ImGui source files into the runner's build folder
set(IMGUI_ZIP_PATH "${bakkesmodsdk_SOURCE_DIR}/imgui_bm.zip")
set(IMGUI_EXTRACT_DIR "${CMAKE_CURRENT_BINARY_DIR}/imgui_bm")

if(EXISTS "${IMGUI_ZIP_PATH}" AND NOT EXISTS "${IMGUI_EXTRACT_DIR}")
    file(ARCHIVE_EXTRACT INPUT "${IMGUI_ZIP_PATH}" DESTINATION "${IMGUI_EXTRACT_DIR}")
endif()

# 3. Explicitly tell the compiler where to map include files
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${bakkesmodsdk_SOURCE_DIR}/include
    ${IMGUI_EXTRACT_DIR}
)

# 4. Map out the transient ImGui source files extracted into the build path
set(IMGUI_SOURCES
    "${IMGUI_EXTRACT_DIR}/imgui/imgui.cpp"
    "${IMGUI_EXTRACT_DIR}/imgui/imgui_draw.cpp"
    "${IMGUI_EXTRACT_DIR}/imgui/imgui_widgets.cpp"
)

# 5. Compile the DLL from your flat root files and the transient ImGui sources
add_library(${PROJECT_NAME} SHARED
    WorkshopMapLoader.cpp
    WorkshopMapLoader.h
    ${IMGUI_SOURCES}
)

# 6. Link against the precompiled core SDK library binary
target_link_libraries(${PROJECT_NAME} PRIVATE "${bakkesmodsdk_SOURCE_DIR}/lib/pluginsdk.lib")

if(MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE WIN32_LEAN_AND_MEAN _CRT_SECURE_NO_WARNINGS)
endif()
