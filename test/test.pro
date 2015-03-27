#-------------------------------------------------
#
# Project created by QtCreator 2014-12-30T00:58:51
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = test
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    timers.cpp

unix: SOURCES += comedi_test.cpp

include ( ../rtlab.pri )



win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../core/release/ -lRtCore
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../core/debug/ -lRtCore
else:unix:!macx: LIBS += -L$$OUT_PWD/../core/ -lRtCore

INCLUDEPATH += $$PWD/../core
DEPENDPATH += $$PWD/../core

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../core/release/libRtCore.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../core/debug/libRtCore.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../core/release/RtCore.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../core/debug/RtCore.lib
else:unix:!macx: PRE_TARGETDEPS += $$OUT_PWD/../core/libRtCore.a



win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../dev/release/ -lRtDev
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../dev/debug/ -lRtDev
else:unix: LIBS += -L$$OUT_PWD/../dev/ -lRtDev

INCLUDEPATH += $$PWD/../dev
DEPENDPATH += $$PWD/../dev

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../dev/release/libRtDev.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../dev/debug/libRtDev.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../dev/release/RtDev.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../dev/debug/RtDev.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../dev/libRtDev.a
