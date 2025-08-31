#-----------------------------------------------------------------------------------------------#
# OpenRGB Windows Dynamic Lighting Sync QMake Project                                                           #
#-----------------------------------------------------------------------------------------------#

#-----------------------------------------------------------------------------------------------#
# Qt Configuration                                                                              #
#-----------------------------------------------------------------------------------------------#
QT +=                                                                                           \
    core                                                                                        \
    gui                                                                                         \
    widgets                                                                                     \

DEFINES += WINDOWSDYNAMICLIGHTINGSYNC_LIBRARY
TEMPLATE = lib

#-----------------------------------------------------------------------------------------------#
# Build Configuration                                                                           #
#-----------------------------------------------------------------------------------------------#
CONFIG +=                                                                                       \
    plugin                                                                                      \
    silent                                                                                      \

#-----------------------------------------------------------------------------------------------#
# Application Configuration                                                                     #
#-----------------------------------------------------------------------------------------------#
MAJOR       = 0
MINOR       = 1
SUFFIX      = alpha

SHORTHASH   = $$system("git rev-parse --short=7 HEAD")
TARGET = OpenRGBWindowsDynamicLightingSyncPlugin_$${MAJOR}.$${MINOR}$${SUFFIX}_Windows_64_$${SHORTHASH}
LASTTAG     = "release_"$$MAJOR"."$$MINOR
COMMAND     = "git rev-list --count "$$LASTTAG"..HEAD"
COMMITS     = $$system($$COMMAND)

VERSION_NUM = $$MAJOR"."$$MINOR"."$$COMMITS
VERSION_STR = $$MAJOR"."$$MINOR

VERSION_WIX = $$VERSION_NUM".0"

equals(SUFFIX, "git") {
VERSION_STR = $$VERSION_STR"+ ("$$SUFFIX$$COMMITS")"
} else {
    !isEmpty(SUFFIX) {
VERSION_STR = $$VERSION_STR"+ ("$$SUFFIX")"
    }
}

message("VERSION_NUM: "$$VERSION_NUM)
message("VERSION_STR: "$$VERSION_STR)
message("VERSION_WIX: "$$VERSION_WIX)

#-----------------------------------------------------------------------------------------------#
# Automatically generated build information                                                     #
#-----------------------------------------------------------------------------------------------#
win32:BUILDDATE = $$system(date /t)
GIT_COMMIT_ID   = $$system(git --git-dir $$_PRO_FILE_PWD_/../.git --work-tree $$_PRO_FILE_PWD_/.. rev-parse HEAD)
GIT_COMMIT_DATE = $$system(git --git-dir $$_PRO_FILE_PWD_/../.git --work-tree $$_PRO_FILE_PWD_/.. show -s --format=%ci HEAD)
GIT_BRANCH      = $$system(git --git-dir $$_PRO_FILE_PWD_/../.git --work-tree $$_PRO_FILE_PWD_/.. rev-parse --abbrev-ref HEAD)

#-----------------------------------------------------------------------------------------------#
# Inject vars in defines                                                                        #
#-----------------------------------------------------------------------------------------------#
DEFINES +=                                                                                      \
    VERSION_STRING=\\"\"\"$$VERSION_STR\\"\"\"                                                  \
    BUILDDATE_STRING=\\"\"\"$$BUILDDATE\\"\"\"                                                  \
    GIT_COMMIT_ID=\\"\"\"$$GIT_COMMIT_ID\\"\"\"                                                 \
    GIT_COMMIT_DATE=\\"\"\"$$GIT_COMMIT_DATE\\"\"\"                                             \
    GIT_BRANCH=\\"\"\"$$GIT_BRANCH\\"\"\"                                                       \
    LATEST_BUILD_URL=\\"\"\"$$LATEST_BUILD_URL\\"\"\"                                           \

#-----------------------------------------------------------------------------------------------#
# Includes                                                                                      #
#-----------------------------------------------------------------------------------------------#
HEADERS +=                                                                                      \
    WindowsDynamicLightingSync.h                                                                \

SOURCES +=                                                                                      \
    WindowsDynamicLightingSync.cpp                                                              \

RESOURCES +=                                                                                    \
    resources.qrc

#-----------------------------------------------------------------------------------------------#
# OpenRGB Plugin SDK                                                                            #
#-----------------------------------------------------------------------------------------------#
INCLUDEPATH +=                                                                                  \
    ../dependencies/OpenRGBSamplePlugin/OpenRGB/                                                \
    ../dependencies/OpenRGBSamplePlugin/OpenRGB/i2c_smbus                                       \
    ../dependencies/OpenRGBSamplePlugin/OpenRGB/RGBController                                   \
    ../dependencies/OpenRGBSamplePlugin/OpenRGB/net_port                                        \
    ../dependencies/OpenRGBSamplePlugin/OpenRGB/dependencies/json                               \

HEADERS +=                                                                                      \
    ../dependencies/OpenRGBSamplePlugin/OpenRGB/OpenRGBPluginInterface.h                        \

#-----------------------------------------------------------------------------------------------#
# Windows-specific Configuration                                                                #
#-----------------------------------------------------------------------------------------------#
win32:CONFIG += QTPLUGIN c++17

win32:CONFIG(debug, debug|release) {
    win32:DESTDIR = ../build/debug
    win32:OBJECTS_DIR = ../build/_intermediate_debug/.obj
    win32:MOC_DIR     = ../build/_intermediate_debug/.moc
    win32:RCC_DIR     = ../build/_intermediate_debug/.qrc
    win32:UI_DIR      = ../build/_intermediate_debug/.ui
}

win32:CONFIG(release, debug|release) {
    win32:DESTDIR = ../release
    win32:OBJECTS_DIR = ../build/_intermediate_release/.obj
    win32:MOC_DIR     = ../build/_intermediate_release/.moc
    win32:RCC_DIR     = ../build/_intermediate_release/.qrc
    win32:UI_DIR      = ../build/_intermediate_release/.ui
}

win32:contains(QMAKE_TARGET.arch, x86_64) {
    LIBS +=                                                                                     \
        -lws2_32                                                                                \
        -lole32                                                                                 \
}

win32:contains(QMAKE_TARGET.arch, x86) {
    LIBS +=                                                                                     \
        -lws2_32                                                                                \
        -lole32                                                                                 \
}

win32:DEFINES +=                                                                                \
    _MBCS                                                                                       \
    WIN32                                                                                       \
    _CRT_SECURE_NO_WARNINGS                                                                     \
    _WINSOCK_DEPRECATED_NO_WARNINGS                                                             \
    WIN32_LEAN_AND_MEAN                                                                         \

#-----------------------------------------------------------------------------------------------#
# Note: This plugin is Windows-only due to Windows Dynamic Lighting API requirements          #
# Linux and macOS configurations have been removed as they are not applicable                 #
#-----------------------------------------------------------------------------------------------#
