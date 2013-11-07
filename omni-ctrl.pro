TRIKCONTROL_BINDIR = ../trikRuntime/bin/release
TRIKCONTROL_DIR = ../trikRuntime/trikControl
BOOST_DIR = ../../Downloads/boost_1_54_0

QT       += core
QT       += network
QT       += gui
TARGET = omni-ctrl
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

INCLUDEPATH = \
        $$PWD \
        $$BOOST_DIR \
        $$TRIKCONTROL_DIR/include \

LIBS += -L$$TRIKCONTROL_BINDIR -ltrikControl

!macx {
        QMAKE_LFLAGS += -Wl,-O1,-rpath,.
}

SOURCES += main.cpp \
    omnirobot.cpp

HEADERS += \
    omnirobot.h
