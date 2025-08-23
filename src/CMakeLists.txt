cmake_minimum_required(VERSION 3.16)

# Project configuration
project(WindowsDynamicLightingSync VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui)

# Enable Qt MOC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Plugin configuration
set(PLUGIN_NAME "WindowsDynamicLightingSync")
set(PLUGIN_VERSION "1.0.0")
set(PLUGIN_DESCRIPTION "Synchronizes OpenRGB devices with Windows Dynamic Lighting")

# Compiler definitions
add_definitions(-DWINDOWSDYNAMICLIGHTINGSYNC_LIBRARY)
add_definitions(-DQT_DEPRECATED_WARNINGS)

# Windows-specific definitions
if(WIN32)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    add_definitions(-DNOMINMAX)
    add_definitions(-DUNICODE)
    add_definitions(-D_UNICODE)
endif()

# Debug/Release definitions
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEBUG)
else()
    add_definitions(-DNDEBUG)
    add_definitions(-DQT_NO_DEBUG_OUTPUT)
endif()

# OpenRGB include directories
set(OPENRGB_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../OpenRGB")

# Check if OpenRGB exists in expected location
if(EXISTS "${OPENRGB_ROOT_DIR}")
    message(STATUS "Found OpenRGB at: ${OPENRGB_ROOT_DIR}")
    set(OPENRGB_INCLUDE_DIRS
        "${OPENRGB_ROOT_DIR}"
        "${OPENRGB_ROOT_DIR}/RGBController"
        "${OPENRGB_ROOT_DIR}/qt"
        "${OPENRGB_ROOT_DIR}/dependencies/json"
        "${OPENRGB_ROOT_DIR}/i2c_smbus"
    )
else()
    message(WARNING "OpenRGB not found at ${OPENRGB_ROOT_DIR}")
    message(STATUS "Trying alternative paths...")
    
    # Try common installation paths
    set(ALTERNATIVE_PATHS
        "C:/Program Files/OpenRGB"
        "C:/OpenRGB"
        "$ENV{OPENRGB_PATH}"
    )
    
    foreach(PATH ${ALTERNATIVE_PATHS})
        if(EXISTS "${PATH}")
            message(STATUS "Found OpenRGB at alternative path: ${PATH}")
            set(OPENRGB_INCLUDE_DIRS
                "${PATH}"
                "${PATH}/plugins"
            )
            break()
        endif()
    endforeach()
    
    if(NOT OPENRGB_INCLUDE_DIRS)
        message(FATAL_ERROR "OpenRGB not found. Please set OPENRGB_PATH environment variable or place OpenRGB in ../OpenRGB")
    endif()
endif()

# Source files
set(PLUGIN_SOURCES
    WindowsDynamicLightingSync.cpp
    WindowsDynamicLightingSyncWidget.cpp
)

# Header files
set(PLUGIN_HEADERS
    WindowsDynamicLightingSync.h
    WindowsDynamicLightingSyncWidget.h
)

# OpenRGB interface headers
set(OPENRGB_HEADERS
    "${OPENRGB_ROOT_DIR}/OpenRGBPluginInterface.h"
    "${OPENRGB_ROOT_DIR}/ResourceManagerInterface.h"
    "${OPENRGB_ROOT_DIR}/RGBController/RGBController.h"
)

# Create the plugin library
add_library(${PLUGIN_NAME} SHARED
    ${PLUGIN_SOURCES}
    ${PLUGIN_HEADERS}
)

# Set target properties
set_target_properties(${PLUGIN_NAME} PROPERTIES
    VERSION ${PLUGIN_VERSION}
    SOVERSION 1
    PREFIX ""
    SUFFIX ".dll"
    OUTPUT_NAME ${PLUGIN_NAME}
)

# Include directories
target_include_directories(${PLUGIN_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OPENRGB_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(${PROJECT_NAME}
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    windowsapp
)

# Windows-specific libraries
if(WIN32)
    target_link_libraries(${PLUGIN_NAME}
        user32
        advapi32
        ole32
        oleaut32
    )
endif()

# Compiler-specific flags
if(MSVC)
    target_compile_options(${PLUGIN_NAME} PRIVATE
        /W3
        /std:c++17
    )
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(${PLUGIN_NAME} PRIVATE /O2)
    endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${PLUGIN_NAME} PRIVATE
        -Wall
        -Wextra
        -std=c++17
    )
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(${PLUGIN_NAME} PRIVATE -O2)
    endif()
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../release")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../release")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../release")

# Create output directory if it doesn't exist
file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../release")

# Installation
install(TARGETS ${PLUGIN_NAME}
    RUNTIME DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/../release"
    LIBRARY DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/../release"
)

# Post-build steps for Windows
if(WIN32)
    # Try to copy to OpenRGB plugins directory
    set(OPENRGB_PLUGINS_DIRS
        "C:/Program Files/OpenRGB/plugins"
        "C:/OpenRGB/plugins"
        "$ENV{OPENRGB_PATH}/plugins"
    )
    
    foreach(PLUGINS_DIR ${OPENRGB_PLUGINS_DIRS})
        if(EXISTS "${PLUGINS_DIR}")
            message(STATUS "Will copy plugin to: ${PLUGINS_DIR}")
            add_custom_command(TARGET ${PLUGIN_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    $<TARGET_FILE:${PLUGIN_NAME}>
                    "${PLUGINS_DIR}/"
                COMMENT "Copying plugin to OpenRGB plugins directory"
            )
            break()
        endif()
    endforeach()
endif()

# Print configuration information
message(STATUS "=== Windows Dynamic Lighting Sync Plugin Configuration ===")
message(STATUS "Plugin Name: ${PLUGIN_NAME}")
message(STATUS "Version: ${PLUGIN_VERSION}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Qt Version: ${Qt6_VERSION}")
message(STATUS "OpenRGB Include Dirs: ${OPENRGB_INCLUDE_DIRS}")
message(STATUS "Output Directory: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "============================================================")

# Optional: Add custom target for cleaning
add_custom_target(clean-plugin
    COMMAND ${CMAKE_COMMAND} -E remove -f
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PLUGIN_NAME}.dll"
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PLUGIN_NAME}_debug.dll"
    COMMENT "Cleaning plugin files"
)

# Optional: Add custom target for packaging
add_custom_target(package-plugin
    DEPENDS ${PLUGIN_NAME}
    COMMAND ${CMAKE_COMMAND} -E tar czf
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PLUGIN_NAME}-${PLUGIN_VERSION}.zip"
        "$<TARGET_FILE:${PLUGIN_NAME}>"
    COMMENT "Creating plugin package"
)

# Development helpers
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    # Add debug symbols and disable optimizations
    target_compile_definitions(${PLUGIN_NAME} PRIVATE DEBUG_BUILD)
    
    if(MSVC)
        target_compile_options(${PLUGIN_NAME} PRIVATE /Zi /Od)
        target_link_options(${PLUGIN_NAME} PRIVATE /DEBUG)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${PLUGIN_NAME} PRIVATE -g -O0)
    endif()
endif()