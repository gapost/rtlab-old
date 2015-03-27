#-------------------------------------------------
#
# Project created by QtCreator 2014-12-02T16:17:03
#
#-------------------------------------------------

QT       += script network

TARGET = RtDev
TEMPLATE = lib
CONFIG += staticlib

include ( ../rtlab.pri )

include ($$PWD/../../3rdparty/qextserialport/src/qextserialport.pri)

SOURCES += RtDev.cpp \
    RtDevice.cpp \
    RtInterface.cpp \
    RtKeithleyDevice.cpp \
    RtPowerSupply.cpp \
    RtTemperatureController.cpp \
    RtResistanceController.cpp \
    RtAxis.cpp \
    rtgpib.cpp \
    rtmodbus.cpp \
    tcp_socket.cpp \
    rtserialinterface.cpp

HEADERS += RtDev.h \
    RtDevice.h \
    RtInterface.h \
    RtKeithleyDevice.h \
    RtPowerSupply.h \
    RtTemperatureController.h \
    RtResistanceController.h \
    isa_pid.h \
    relaytuner.h \
    RtAxis.h \
    rtgpib.h \
    rtmodbus.h \
    rtdaq.h \
    tcp_socket.h \
    rtserialinterface.h



nidaqmx {
HEADERS += rtdaqmxtask.h \
           daqmx.h
SOURCES += rtdaqmxtask.cpp \
           daqmx.cpp
}

comedi {
HEADERS += comedi_qt.h \
    ni6221.h \
    RtComedi.h
SOURCES += ni6221.cpp \
    RtComedi.cpp
}

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../core/release/ -lRtCore
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../core/debug/ -lRtCore
else:unix: LIBS += -L$$OUT_PWD/../core/ -lRtCore

INCLUDEPATH += $$PWD/../core
DEPENDPATH += $$PWD/../core

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../core/release/RtCore.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../core/debug/RtCore.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../core/libRtCore.a



