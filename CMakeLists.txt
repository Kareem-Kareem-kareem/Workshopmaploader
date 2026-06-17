cmake_minimum_required(VERSION 3.20)
project(WorkshopMapLoader)

# Enforce strict Modern C++ compilation standards
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "Configuring build target environment for Rocket League BakkesMod Plugin: ${PROJECT_NAME}")

#---------------------------------------------------------------------
# 1. AUTOMATED DEPENDENCY MANAGEMENT (BAKKESMOD SDK FETCH)
#---------------------------------------------------------------------
include(FetchContent)
FetchContent_Declare(
    bakkesmodsdk
    GIT_REPOSITORY https://github.com/bakkesmodorg/BakkesModSDK.git
    GIT_TAG master
)

message(STATUS "Synchronizing automated external content dependencies...")
FetchContent_MakeAvailable(bakkesmodsdk)
message(STATUS "BakkesMod SDK source directory located at: ${bakkesmodsdk_SOURCE_DIR}")

#---------------------------------------------------------------------
# 2. RUNTIME DEPENDENCY EXTRACTION (IMGUI SOURCE DEPLOYMENT)
#---------------------------------------------------------------------
set(IMGUI_ZIP_PATH "${bakkesmodsdk_SOURCE_DIR}/imgui_bm.zip")
set(IMGUI_EXTRACT_DIR "${CMAKE_CURRENT_BINARY_DIR}/imgui_bm")

if(EXISTS "${IMGUI_ZIP_PATH}" AND NOT EXISTS "${IMGUI_EXTRACT_DIR}")
    message(STATUS "Extracting embedded ImGui framework archives to transient build directory...")
    file(ARCHIVE_EXTRACT INPUT "${IMGUI_ZIP_PATH}" DESTINATION "${IMGUI_EXTRACT_DIR}")
else()
    message(STATUS "ImGui extraction target directory verified or archive extraction skipped.")
endif()

#---------------------------------------------------------------------
# 3. COMPILER INCLUDE DIRECTORY MAPPING
#---------------------------------------------------------------------
# This section completely resolves the undefined base class errors by forcing
# MSBuild to parse both the local repository root and the fetched SDK directories.
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${bakkesmodsdk_SOURCE_DIR}/include
    ${IMGUI_EXTRACT_DIR}
)

#---------------------------------------------------------------------
# 4. TRANSIENT SOURCE FILE TRACKING
#---------------------------------------------------------------------
set(IMGUI_SOURCES
    "${IMGUI_EXTRACT_DIR}/imgui/imgui.cpp"
    "${IMGUI_EXTRACT_DIR}/imgui/imgui_draw.cpp"
    "${IMGUI_EXTRACT_DIR}/imgui/imgui_widgets.cpp"
)

#---------------------------------------------------------------------
# 5. TARGET SHARED LIBRARY DEFINITION
#---------------------------------------------------------------------
add_library(${PROJECT_NAME} SHARED
    WorkshopMapLoader.cpp
    WorkshopMapLoader.h
    ${IMGUI_SOURCES}
)

#---------------------------------------------------------------------
# 6. LINKER AND OS SPECIFIC TARGET DEFINITIONS
#---------------------------------------------------------------------
# Explicitly link against the precompiled static library provided by the SDK download
target_link_libraries(${PROJECT_NAME} PRIVATE "${bakkesmodsdk_SOURCE_DIR}/lib/pluginsdk.lib")

if(MSVC)
    message(STATUS "Applying optimization and compatibility definitions for MSBuild/Windows environments.")
    target_compile_definitions(${PROJECT_NAME} PRIVATE 
        WIN32_LEAN_AND_MEAN 
        _CRT_SECURE_NO_WARNINGS
        _WINDLL
    )
endif()

message(STATUS "Configuration phase
