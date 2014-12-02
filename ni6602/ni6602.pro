#-------------------------------------------------
#
# Project created by QtCreator 2014-12-02T08:58:42
#
#-------------------------------------------------

# QT       -= core gui

TARGET = ni6602
TEMPLATE = lib
CONFIG += staticlib
CONFIG	 -= qt

include ( ../rtlab.pri )


SOURCES += \
    DigIO.cpp \
    device.cpp \
    Counter.cpp \
    constants.cpp

HEADERS += ni6602.h \
    DigIO.h \
    device.h \
    Counter.h


