TRIKCONTROL_BINDIR = ../trikRuntime/bin/release
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
           motor_controller.cpp \
           state.cpp \
           state_searching.cpp \
           state_tracking.cpp \
           state_squeezing.cpp \
           state_releasing.cpp \
           state_finished.cpp

HEADERS += \
           rover.h \
           log_fifo.h \
           cmd_fifo.h \
           motor_controller.h \
           enum_state.h \
           state.h \
           state_searching.h \
           state_tracking.h \
           state_squeezing.h \
           state_releasing.h \
           state_finished.h

