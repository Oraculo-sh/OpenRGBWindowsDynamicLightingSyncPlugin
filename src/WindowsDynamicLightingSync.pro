#-------------------------------------------------
# Windows Dynamic Lighting Sync Plugin for OpenRGB
# Project created by Code Architect AI
#-------------------------------------------------

QT += core gui widgets

TARGET = WindowsDynamicLightingSync
TEMPLATE = lib

DEFINES += WINDOWSDYNAMICLIGHTINGSYNC_LIBRARY
DEFINES += QT_DEPRECATED_WARNINGS

# Version information
VERSION = 1.0.0
QMAKE_TARGET_COMPANY = "OpenRGB Community"
QMAKE_TARGET_PRODUCT = "Windows Dynamic Lighting Sync Plugin"
QMAKE_TARGET_DESCRIPTION = "Synchronizes OpenRGB devices with Windows Dynamic Lighting"
QMAKE_TARGET_COPYRIGHT = "Copyright (c) 2024"

# C++ standard
CONFIG += c++17

# Build configuration
CONFIG += plugin
CONFIG += shared
CONFIG += warn_on

# Platform-specific configurations
win32 {
    CONFIG += skip_target_version_ext
    LIBS += -luser32 -ladvapi32 -lole32 -loleaut32
    
    # Windows-specific defines
    DEFINES += WIN32_LEAN_AND_MEAN
    DEFINES += NOMINMAX
    DEFINES += UNICODE
    DEFINES += _UNICODE
}

# OpenRGB include paths
INCLUDEPATH += $$PWD/../OpenRGB
INCLUDEPATH += $$PWD/../OpenRGB/RGBController
INCLUDEPATH += $$PWD/../OpenRGB/qt
INCLUDEPATH += $$PWD/../OpenRGB/dependencies/json

# Alternative include paths if OpenRGB is in different location
!exists($$PWD/../OpenRGB) {
    message("OpenRGB not found in ../OpenRGB, trying alternative paths...")
    
    # Try common OpenRGB installation paths
    exists("C:/Program Files/OpenRGB") {
        INCLUDEPATH += "C:/Program Files/OpenRGB"
        INCLUDEPATH += "C:/Program Files/OpenRGB/plugins"
    }
    
    exists("C:/OpenRGB") {
        INCLUDEPATH += "C:/OpenRGB"
        INCLUDEPATH += "C:/OpenRGB/plugins"
    }
    
    # Environment variable path
    !isEmpty(OPENRGB_PATH) {
        INCLUDEPATH += $$OPENRGB_PATH
        INCLUDEPATH += $$OPENRGB_PATH/plugins
    }
}

# Source files
SOURCES += \
    WindowsDynamicLightingSync.cpp \
    WindowsDynamicLightingSyncWidget.cpp

# Header files
HEADERS += \
    WindowsDynamicLightingSync.h \
    WindowsDynamicLightingSyncWidget.h

# OpenRGB interface headers (these should be available from OpenRGB)
HEADERS += \
    ../OpenRGB/OpenRGBPluginInterface.h \
    ../OpenRGB/ResourceManagerInterface.h \
    ../OpenRGB/RGBController/RGBController.h

# Output directory
DESTDIR = $$PWD/../release

# Intermediate directories
OBJECTS_DIR = $$PWD/../build/obj
MOC_DIR = $$PWD/../build/moc
RCC_DIR = $$PWD/../build/rcc
UI_DIR = $$PWD/../build/ui

# Create build directories
!exists($$OBJECTS_DIR): system(mkdir $$shell_quote($$shell_path($$OBJECTS_DIR)))
!exists($$MOC_DIR): system(mkdir $$shell_quote($$shell_path($$MOC_DIR)))
!exists($$RCC_DIR): system(mkdir $$shell_quote($$shell_path($$RCC_DIR)))
!exists($$UI_DIR): system(mkdir $$shell_quote($$shell_path($$UI_DIR)))
!exists($$DESTDIR): system(mkdir $$shell_quote($$shell_path($$DESTDIR)))

# Debug/Release specific settings
CONFIG(debug, debug|release) {
    TARGET = $$join(TARGET,,,_debug)
    DEFINES += DEBUG
    message("Building in DEBUG mode")
} else {
    DEFINES += NDEBUG
    DEFINES += QT_NO_DEBUG_OUTPUT
    message("Building in RELEASE mode")
}

# Compiler flags
msvc {
    QMAKE_CXXFLAGS += /std:c++17
    QMAKE_CXXFLAGS_WARN_ON += /W3
    QMAKE_CXXFLAGS_RELEASE += /O2
}

gcc|clang {
    QMAKE_CXXFLAGS += -std=c++17
    QMAKE_CXXFLAGS_WARN_ON += -Wall -Wextra
    QMAKE_CXXFLAGS_RELEASE += -O2
}

# Plugin export
DEFINES += QT_PLUGIN

# Installation
target.path = $$PWD/../release
INSTALLS += target

# Clean target
QMAKE_CLEAN += $$DESTDIR/$$TARGET.*

# Print configuration info
message("=== Windows Dynamic Lighting Sync Plugin Configuration ===")
message("Target: $$TARGET")
message("Template: $$TEMPLATE")
message("Qt modules: $$QT")
message("Include paths: $$INCLUDEPATH")
message("Output directory: $$DESTDIR")
message("Version: $$VERSION")
message("============================================================")

# Resource files (if any)
# RESOURCES += resources.qrc

# Translation files (if any)
# TRANSLATIONS += translations/plugin_en.ts \
#                translations/plugin_pt.ts

# Additional dependencies
# DEPENDPATH += $$INCLUDEPATH

# Post-build steps
win32 {
    # Copy to OpenRGB plugins directory if it exists
    OPENRGB_PLUGINS_DIR = "C:/Program Files/OpenRGB/plugins"
    exists($$OPENRGB_PLUGINS_DIR) {
        QMAKE_POST_LINK += $$quote(copy /Y $$shell_path($$DESTDIR/$$TARGET.dll) $$shell_path($$OPENRGB_PLUGINS_DIR/))
    }
}

# Manifest for Windows (if needed)
# win32:RC_FILE = plugin.rc