#-------------------------------------------------
#
# Project created by QtCreator 2014-12-16T09:29:15
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET    = qtdaqmx
CONFIG   += console
CONFIG   -= app_bundle

win32 {
DAQMX_PATH = "c:/Program Files (x86)/National Instruments/NI-DAQ/DAQmx ANSI C Dev"
INCLUDEPATH += $$DAQMX_PATH/include
LIBS += -L$$DAQMX_PATH/lib/msvc -lnidaqmx
}


TEMPLATE = app


SOURCES += main.cpp \
    qdaqmx.cpp

HEADERS += \
    qdaqmx.h
