############## 3rd Party Lib Paths ###############

GSL_PATH = ../../gsl-1.15
GSL_INCLUDE = $$GSL_PATH
GSL_LIB = ../../gsl-1.15/build.vc10/lib/Win32/Release/gsl.lib

DEFINES += QWT_DLL
QWT_INCLUDE = ../../qwt-6.1.0/src
CONFIG(debug, debug|release) {
    QWT_LIB = ../../qwt-6.1.0/lib/qwtd.lib
} else {
    QWT_LIB = ../../qwt-6.1.0/lib/qwt.lib
}

DEFINES += _HDF5USEDLL_ HDF5CPP_USEDLL
HDF5_INCLUDE = ../../hdf5-1.8.10/include
HDF5_LIB = ../../hdf5-1.8.10/lib/hdf5dll.lib ../../hdf5-1.8.10/lib/hdf5_cppdll.lib

MUPARSER_INCLUDE = ../../muparser_v2_2_3/include
CONFIG(debug, debug|release) {
    MUPARSER_LIB = ../../muparser_v2_2_3/lib/muparserd.lib
} else {
    MUPARSER_LIB = ../../muparser_v2_2_3/lib/muparser.lib
}

INCLUDEPATH += $$HDF5_INCLUDE $$MUPARSER_INCLUDE $$QWT_INCLUDE $$GSL_INCLUDE
LIBS += $$HDF5_LIB $$GSL_LIB $$MUPARSER_LIB $$QWT_LIB $$GSL_LIB
LIBS += winmm.lib

####################################################

CONFIG(debug, debug|release) {
    DESTDIR = ../debug
    LIBS += -L../debug
} else {
    DESTDIR = ../release
    LIBS += -L../release
}

DEFINES += _CRT_SECURE_NO_WARNINGS

