#-------------------------------------------------
#
# Project created by QtCreator 2014-12-02T23:10:11
#
#-------------------------------------------------

TARGET = RtLab
TEMPLATE = app
QT       += core gui script network
CONFIG   += uitools

RESOURCES     = rtlab.qrc
win32:RC_FILE = rtlab.rc

include ( ../rtlab.pri )

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

SOURCES += main.cpp \
    QConsoleWidget.cpp \
    QConsoleHistory.cpp \
    MainWindow.cpp \
    ScriptEditor.cpp \
    ScriptConsole.cpp

HEADERS  += \
    QConsoleWidget.h \
    QConsoleHistory.h \
    MainWindow.h \
    ScriptEditor.h \
    ScriptConsole.h



win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../core/release/ -lRtCore
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../core/debug/ -lRtCore
else:unix: LIBS += -L$$OUT_PWD/../core/ -lRtCore

INCLUDEPATH += $$PWD/../core
DEPENDPATH += $$PWD/../core

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../core/release/RtCore.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../core/debug/RtCore.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../core/libRtCore.a



win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../dev/release/ -lRtDev
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../dev/debug/ -lRtDev
else:unix: LIBS += -L$$OUT_PWD/../dev/ -lRtDev

INCLUDEPATH += $$PWD/../dev
DEPENDPATH += $$PWD/../dev

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../dev/release/RtDev.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../dev/debug/RtDev.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../dev/libRtDev.a