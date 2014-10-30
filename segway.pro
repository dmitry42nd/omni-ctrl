TRIKCONTROL_BINDIR = ../trikRuntime/bin/arm-release
TRIKCONTROL_DIR = ../trikRuntime/trikControl

INCLUDE_DIR = include
SRC_DIR     = src

MOC_DIR     = .moc
OBJECTS_DIR = .obj

QT       += core
QT       += network
QT       += gui

TARGET = segway
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

INCLUDEPATH = \
        $$INCLUDE_DIR \
        $$PWD \
        $$BOOST_DIR \
        $$TRIKCONTROL_DIR/include \

LIBS += -L$$TRIKCONTROL_BINDIR -ltrikControl -ltrikKernel

!macx {
        QMAKE_LFLAGS += -Wl,-O1,-rpath,.
        QMAKE_CXXFLAGS += -std=c++11
}

SOURCES += src/*  

HEADERS += include/*
