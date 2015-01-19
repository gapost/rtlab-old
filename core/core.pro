#-------------------------------------------------
#
# Project created by QtCreator 2013-02-04T17:24:24
#
#-------------------------------------------------

TARGET = RtCore
TEMPLATE = lib
CONFIG += staticlib
QT       += script

include ( ../rtlab.pri )

include(../../3rdparty/qt-solutions/qtpropertybrowser/src/qtpropertybrowser.pri)

# INCLUDEPATH += $$PWD

HEADERS += \
    os_utils.h \ 
    RtObject.h \
    RtEvent.h \
    RtRoot.h \
    RtJob.h \
    RtVector.h \
    RtTypes.h \
    RtDataBuffer.h \
    ivector.h \
    RtTimerLoop.h \
    RtDataChannel.h \
    filters.h \
    PacketManager.h \
    Packet.h \
    RtFigure.h \
    RtBinaryChannelOp.h \
    RtMainWindow.h \
    RtEnumHelper.h \
    RtObjectInspector.h \
    linefit.h \
    RtPlotWidget.h \
    RtInterpChannel.h \
    RtTableWidget.h \
    RtChannelWidget.h \
    RtTimeValue.h \
    RtSession.h \
    RtScriptJob.h \
    RtDelegates.h \
    objectcontroller.h

win32: HEADERS += os_util_win32.h
unix:  HEADERS += os_util_linux.h

SOURCES += \
    RtObject.cpp \
    RtEvent.cpp \
    RtRoot.cpp \
    RtJob.cpp \
    RtVector.cpp \
    RtTypes.cpp \
    RtDataBuffer.cpp \
    RtTimerLoop.cpp \
    RtDataChannel.cpp \
    Packet.cpp \
    RtFigure.cpp \
    RtBinaryChannelOp.cpp \
    RtMainWindow.cpp \
    RtObjectInspector.cpp \
    RtInterpChannel.cpp \
    RtTableWidget.cpp \
    RtChannelWidget.cpp \
    RtSession.cpp \
    RtScriptJob.cpp \
    RtDelegates.cpp \
    objectcontroller.cpp \
    RtRootSerialize.cpp

greaterThan(QT_MAJOR_VERSION, 4): SOURCES += rtplotwidgetqwt6.cpp
else: SOURCES += rtplotwidgetqwt5.cpp

FORMS = ChannelWidget.ui

