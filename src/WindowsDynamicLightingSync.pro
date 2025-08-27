#-----------------------------------------------------------------------------------------------#
# OpenRGB Windows Dynamic Lighting Sync Plugin QMake Project                                   #
#-----------------------------------------------------------------------------------------------#

#-----------------------------------------------------------------------------------------------#
# Qt Configuration                                                                              #
#-----------------------------------------------------------------------------------------------#
QT +=                                                                                           \
    core                                                                                        \
    gui                                                                                         \
    widgets                                                                                     \
    network                                                                                     \

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
MAJOR       = 1
MINOR       = 0
SUFFIX      = 

VERSION_NUM = $$MAJOR"."$$MINOR".0"
VERSION_STR = $$MAJOR"."$$MINOR

#-----------------------------------------------------------------------------------------------#
# Inject vars in defines                                                                        #
#-----------------------------------------------------------------------------------------------#
DEFINES +=                                                                                      \
    VERSION_STRING=\\\"$$VERSION_STR\\\"                                                  \

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
    ../dependencies/OpenRGBSamplePlugin/OpenRGB/                                               \
    ../dependencies/OpenRGBSamplePlugin/OpenRGB/i2c_smbus                                      \
    ../dependencies/OpenRGBSamplePlugin/OpenRGB/RGBController                                   \
    ../dependencies/OpenRGBSamplePlugin/OpenRGB/net_port                                        \
    ../dependencies/OpenRGBSamplePlugin/OpenRGB/dependencies/json                               \

HEADERS +=                                                                                      \
    ../dependencies/OpenRGBSamplePlugin/OpenRGB/OpenRGBPluginInterface.h                       \

#-----------------------------------------------------------------------------------------------#
# Windows-specific Configuration                                                                #
#-----------------------------------------------------------------------------------------------#
win32:CONFIG += QTPLUGIN c++17

win32:CONFIG(debug, debug|release) {
    win32:DESTDIR = debug
}

win32:CONFIG(release, debug|release) {
    win32:DESTDIR = release
}

win32:OBJECTS_DIR = _intermediate_$$DESTDIR/.obj
win32:MOC_DIR     = _intermediate_$$DESTDIR/.moc
win32:RCC_DIR     = _intermediate_$$DESTDIR/.qrc
win32:UI_DIR      = _intermediate_$$DESTDIR/.ui

win32:contains(QMAKE_TARGET.arch, x86_64) {
    LIBS +=                                                                                     \
        -lws2_32                                                                                \
        -lole32                                                                                 \
        -lwindowsapp                                                                            \
        -loleaut32                                                                              \
        -lruntimeobject                                                                         \
}

win32:contains(QMAKE_TARGET.arch, x86) {
    LIBS +=                                                                                     \
        -lws2_32                                                                                \
        -lole32                                                                                 \
        -lwindowsapp                                                                            \
        -loleaut32                                                                              \
        -lruntimeobject                                                                         \
}

win32:DEFINES +=                                                                                \
    _MBCS                                                                                       \
    WIN32                                                                                       \
    _CRT_SECURE_NO_WARNINGS                                                                     \
    _WINSOCK_DEPRECATED_NO_WARNINGS                                                             \
    WIN32_LEAN_AND_MEAN                                                                         \

#-----------------------------------------------------------------------------------------------#
# Linux-specific Configuration                                                                  #
#-----------------------------------------------------------------------------------------------#
unix:!macx {
    QMAKE_CXXFLAGS += -std=c++17
    target.path=$$PREFIX/lib/openrgb/plugins/
    INSTALLS += target
}

#-----------------------------------------------------------------------------------------------#
# MacOS-specific Configuration                                                                  #
#-----------------------------------------------------------------------------------------------#
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.15

macx: {
    CONFIG += c++17
}