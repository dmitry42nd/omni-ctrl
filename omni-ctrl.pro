TRIKCONTROL_BINDIR = ../trikRuntime/bin/release
TRIKCONTROL_DIR = ../trikRuntime/trikControl

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
           omni.cpp \
           log_fifo.cpp \
           cmd_fifo.cpp \
           motor_controller.cpp \

HEADERS += \
           omni.h \
           log_fifo.h \
           cmd_fifo.h \
           motor_controller.h 

