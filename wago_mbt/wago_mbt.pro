TEMPLATE = lib
CONFIG += staticlib
CONFIG	 -= qt

SOURCES += \
    ModBus.cpp

HEADERS += \
    ModBus.h

include ( ../rtlab.pri )

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../core/release/ -lRtCore
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../core/debug/ -lRtCore
else:unix: LIBS += -L$$OUT_PWD/../core/ -lRtCore

INCLUDEPATH += $$PWD/../core
DEPENDPATH += $$PWD/../core

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../core/release/RtCore.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../core/debug/RtCore.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../core/libRtCore.a
