include(rtlab_config.pri)

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets uitools
} else {
    CONFIG += uitools
}

############## 3rd Party Libs for win32 ###############

win32 {

############## NI-DAQmx ##############
nidaqmx {
DAQMX_PATH = "c:/Program Files (x86)/National Instruments/NI-DAQ/DAQmx ANSI C Dev"
#    DAQMX_PATH = "c:/Program Files/National Instruments/NI-DAQ/DAQmx ANSI C Dev"
LIBS += -L$$DAQMX_PATH//Lib/msvc -lnidaqmx
INCLUDEPATH += $$DAQMX_PATH/include/
DEFINES += USE_NIDAQMX
}

    ############## NI-488.2 ##############
    LIBS += -L$$PWD/../3rdparty/ni4882-1.6 gpib-32.obj
    INCLUDEPATH += $$PWD/../3rdparty/ni4882-1.6
    DEPENDPATH += $$PWD/../3rdparty/ni4882-1.6

    ############## GSL ##############
    LIBS += -L$$PWD/../3rdparty/gsl-1.15/build.vc10/lib/Win32/release/ -lgsl
    INCLUDEPATH += $$PWD/../3rdparty/gsl-1.15
    DEPENDPATH += $$PWD/../3rdparty/gsl-1.15/gsl

greaterThan(QT_MAJOR_VERSION, 4) {
# Qwt-6
DEFINES += QWT_DLL
QWT_PATH = $$PWD/../3rdparty/Qwt6-Qt5-msvc2010
LIBS += -L$$QWT_PATH/lib/
CONFIG(release, debug|release) {
    LIBS += -lqwt
} else {
    LIBS += -lqwtd
}
INCLUDEPATH += $$QWT_PATH/include
DEPENDPATH += $$QWT_PATH/include
} else {
    ############## Qwt-5.2.1 ##############
    DEFINES += QWT_DLL
    CONFIG(release, debug|release) {
        LIBS += -L$$PWD/../3rdparty/qwt5_msvc2010_install/lib/ -lqwt5
    } else {
        LIBS += -L$$PWD/../3rdparty/qwt5_msvc2010_install/lib/ -lqwtd5
    }
    INCLUDEPATH += $$PWD/../3rdparty/qwt5_msvc2010_install/include
    DEPENDPATH += $$PWD/../3rdparty/qwt5_msvc2010_install/include
}

    ############## HDF5 ##############
    DEFINES += _HDF5USEDLL_ HDF5CPP_USEDLL
    INCLUDEPATH += $$PWD/../3rdparty/hdf5-1.8.13/include
    LIBS += -L$$PWD/../3rdparty/hdf5-1.8.13/lib -lhdf5 -lhdf5_cpp

    ############## muParser ##############
    INCLUDEPATH += $$PWD/../3rdparty/muparser_v2_2_3/include
    LIBS += -L$$PWD/../3rdparty/muparser_v2_2_3/lib/
    CONFIG(debug, debug|release) {
        LIBS += -lmuparserd
    } else {
        LIBS += -lmuparser
    }

############# libmodbus ##############
LIBMODBUS_PATH = $$PWD/../3rdparty/libmodbus/src
INCLUDEPATH += $$LIBMODBUS_PATH
LIBS += -L$$LIBMODBUS_PATH/win32
CONFIG(debug, debug|release) {
    LIBS += -lmodbusd
} else {
    LIBS += -lmodbus
}
#LIBS += -lsetupapi -lwsock32 ws2_32.lib advapi32.lib


    ######## Win32 Libs ##########
    LIBS += winmm.lib user32.lib ws2_32.lib

    DEFINES += _CRT_SECURE_NO_WARNINGS

    DEFINES += finite=_finite
}

############# linux ##############
unix {
    ########## system libs ##########
    LIBS += -lrt -lpthread

    ############## GSL ##############
    LIBS += -lgsl -lcblas -latlas -lm
    # INCLUDEPATH += /usr/include/gsl

greaterThan(QT_MAJOR_VERSION, 4) {
# Qwt-6 compiled
INCLUDEPATH += /usr/local/qwt-6.1.2/include
LIBS += -L/usr/local/qwt-6.1.2/lib/ -lqwt
} else {
# Qwt5 installed from repo package
INCLUDEPATH += /usr/include/qwt
LIBS += -lqwt
}


    ############## HDF5 ##############
    LIBS += -lhdf5 -lhdf5_cpp

    ############## muParser ##############
    LIBS += -lmuparser

######### linux-gpib ############
LIBS += -lgpib

######### libmodbus #############
LIBS += -lmodbus
INCLUDEPATH += /usr/include/modbus

}

OTHER_FILES += \
    ../linux_readme.txt

