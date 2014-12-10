TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp

HEADERS += \
    ../core/os_utils.h \
    ../core/os_utils_win32.h \
    ../core/os_utils_linux.h

unix: LIBS += -lrt -lpthread


