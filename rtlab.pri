############## 3rd Party Libs ###############

############## WAGO ModBus-TCP ##############

win32: LIBS += -L$$PWD/../wago_modbus_tcp/lib/ -lMBT

INCLUDEPATH += $$PWD/../wago_modbus_tcp/include
DEPENDPATH += $$PWD/../wago_modbus_tcp/include

############## NI-DAQ ##############

win32 {
    LIBS += -L$$PWD/../ni-daq/Lib/ -lnidaq32
    INCLUDEPATH += $$PWD/../ni-daq/Include
    DEPENDPATH += $$PWD/../ni-daq/Include
}

############## NI-488.2 ##############

win32 {
    LIBS += -L$$PWD/../ni4882-1.6 gpib-32.obj
    INCLUDEPATH += $$PWD/../ni4882-1.6
    DEPENDPATH += $$PWD/../ni4882-1.6
}

############## GSL ##############

win32 {
    LIBS += -L$$PWD/../gsl-1.15/build.vc10/lib/Win32/release/ -lgsl
    INCLUDEPATH += $$PWD/../gsl-1.15
    DEPENDPATH += $$PWD/../gsl-1.15/gsl
}

############## Qwt-5.2.1 ##############

DEFINES += QWT_DLL
win32 {
    CONFIG(release, debug|release) {
        LIBS += -L$$PWD/../qwt5_msvc2010_install/lib/ -lqwt5
    } else {
        LIBS += -L$$PWD/../qwt5_msvc2010_install/lib/ -lqwtd5
    }
    INCLUDEPATH += $$PWD/../qwt5_msvc2010_install/include
    DEPENDPATH += $$PWD/../qwt5_msvc2010_install/include
}

###################################################################

DEFINES += _HDF5USEDLL_ HDF5CPP_USEDLL
HDF5_INCLUDE = ../../hdf5-1.8.10/include
HDF5_LIB = ../../hdf5-1.8.10/lib/hdf5dll.lib ../../hdf5-1.8.10/lib/hdf5_cppdll.lib

MUPARSER_INCLUDE = ../../muparser_v2_2_3/include
CONFIG(debug, debug|release) {
    MUPARSER_LIB = ../../muparser_v2_2_3/lib/muparserd.lib
} else {
    MUPARSER_LIB = ../../muparser_v2_2_3/lib/muparser.lib
}

INCLUDEPATH += $$HDF5_INCLUDE $$MUPARSER_INCLUDE
LIBS += $$HDF5_LIB $$GSL_LIB $$MUPARSER_LIB
LIBS += winmm.lib user32.lib



####################################################

#CONFIG(debug, debug|release) {
#    DESTDIR = ../debug
#    LIBS += -L../debug
#} else {
#    DESTDIR = ../release
#    LIBS += -L../release
#}

DEFINES += _CRT_SECURE_NO_WARNINGS





