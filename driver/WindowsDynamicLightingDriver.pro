#-----------------------------------------------------------------------------------------------#
# Windows Dynamic Lighting Driver (Console) - QMake Project                                     #
#-----------------------------------------------------------------------------------------------#

QT += core network
CONFIG += console c++17
CONFIG -= app_bundle
TEMPLATE = app

TARGET = WindowsDynamicLightingDriver

SOURCES += \
    src/main.cpp \
    src/WDLDriverServer.cpp

HEADERS += \
    src/WDLDriverServer.h \
    common/DriverProtocol.h

INCLUDEPATH += \
    . \
    common \
    src

win32:DEFINES += \
    _CRT_SECURE_NO_WARNINGS \
    WIN32_LEAN_AND_MEAN

QMAKE_CXXFLAGS += -Wall -Wextra

# Output dirs
win32:CONFIG(debug, debug|release) {
    DESTDIR = ../build/driver/debug
    OBJECTS_DIR = ../build/driver/_intermediate_debug/.obj
    MOC_DIR     = ../build/driver/_intermediate_debug/.moc
}

win32:CONFIG(release, debug|release) {
    DESTDIR = ../release
    OBJECTS_DIR = ../build/driver/_intermediate_release/.obj
    MOC_DIR     = ../build/driver/_intermediate_release/.moc
}
