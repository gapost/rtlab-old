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

SOURCES += RtDev.cpp \
    RtDevice.cpp \
    RtInterface.cpp \
    RtKeithleyDevice.cpp \
    RtPowerSupply.cpp \
    RtTemperatureController.cpp \
    RtResistanceController.cpp \
    RtAxis.cpp \
    rtdaqmxtask.cpp \
    rtgpib.cpp \
    rtmodbus.cpp

daqmxbase {
    SOURCES += daqmx_base.cpp
} else {
    SOURCES += daqmx.cpp
}

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
    rtdaqmxtask.h \
    daqmx.h \
    rtgpib.h \
    rtmodbus.h

win32 {
SOURCES += $$LIBMODBUS_PATH/modbus.c \
        $$LIBMODBUS_PATH/modbus-data.c \
        $$LIBMODBUS_PATH/modbus-rtu.c \
        $$LIBMODBUS_PATH/modbus-tcp.c

HEADERS += $$LIBMODBUS_PATH/modbus.h \
        $$LIBMODBUS_PATH/config.h
}

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../core/release/ -lRtCore
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../core/debug/ -lRtCore
else:unix: LIBS += -L$$OUT_PWD/../core/ -lRtCore

INCLUDEPATH += $$PWD/../core
DEPENDPATH += $$PWD/../core

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../core/release/RtCore.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../core/debug/RtCore.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../core/libRtCore.a

#win32 {
#    CONFIG(release, debug|release) {
#        LIBS += -L$$OUT_PWD/../wago_mbt/release/ -lwago_mbt
#        PRE_TARGETDEPS += $$OUT_PWD/../wago_mbt/release/wago_mbt.lib
#    } else {
#        LIBS += -L$$OUT_PWD/../wago_mbt/debug/ -lwago_mbt
#        PRE_TARGETDEPS += $$OUT_PWD/../wago_mbt/debug/wago_mbt.lib
#    }
#    INCLUDEPATH += $$PWD/../wago_mbt
#    DEPENDPATH += $$PWD/../wago_mbt

#}

