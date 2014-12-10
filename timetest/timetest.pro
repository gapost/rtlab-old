TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp

HEADERS += \
    os_utils.h \
    os_utils_win32.h \
    os_utils_linux.h

unix: LIBS += -lrt -lpthread


