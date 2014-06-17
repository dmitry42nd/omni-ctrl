TRIKCONTROL_BINDIR = ../trikRuntime/bin/arm-release
TRIKCONTROL_DIR = ../trikRuntime/trikControl

QT       += core
QT       += network
QT       += gui
TARGET = rover-ctrl
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
           rover.cpp \
           log_fifo.cpp \
           cmd_fifo.cpp \
           rover_engine.cpp \
           rover_manual.cpp \
           rover_logic.cpp

HEADERS += \
           rover.h \
           log_fifo.h \
           cmd_fifo.h \
           rover_engine.h \
           rover_manual.h \
           rover_logic.h
