TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    daqmx.cpp

win32 {
DAQMX_PATH = "c:/Program Files (x86)/National Instruments/NI-DAQ/DAQmx ANSI C Dev"
INCLUDEPATH += $$DAQMX_PATH/include
LIBS += -L$$DAQMX_PATH/lib/msvc -lnidaqmx
}

HEADERS += \
    daqmx.h

