#-------------------------------------------------
#
# Project created by QtCreator 2013-02-04T17:24:24
#
#-------------------------------------------------

include ( ../rtlab.pri )

include(../../qtpropertybrowser/src/qtpropertybrowser.pri)


TARGET = RtCore
TEMPLATE = lib
CONFIG += staticlib uitools

QT       += script

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
    RtPlotWidget.cpp \
    RtInterpChannel.cpp \
    RtTableWidget.cpp \
    RtChannelWidget.cpp \
    RtSession.cpp \
    RtScriptJob.cpp \
    RtDelegates.cpp \
    objectcontroller.cpp \
    RtRootSerialize.cpp

FORMS = ChannelWidget.ui

