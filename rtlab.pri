
############## 3rd Party Libs for win32 ###############

win32 {
    ############## WAGO ModBus-TCP ##############
    CONFIG += wago_mbt
    wago_mbt {
        LIBS += -L$$PWD/../3rdparty/wago_modbus_tcp/lib/ -lMBT
        INCLUDEPATH += $$PWD/../3rdparty/wago_modbus_tcp/include
        DEPENDPATH += $$PWD/../3rdparty/wago_modbus_tcp/include
        DEFINES += USE_WAGO_MBT
    }

    ############## NI-DAQ ##############
    LIBS += -L$$PWD/../3rdparty/ni-daq-8.6/Lib/ -lnidaq32
    INCLUDEPATH += $$PWD/../3rdparty/ni-daq-8.6/Include
    DEPENDPATH += $$PWD/../3rdparty/ni-daq-8.6/Include

    ############## NI-DAQmx ##############
    DAQMX_PATH = "c:/Program Files (x86)/National Instruments/NI-DAQ/DAQmx ANSI C Dev"
    LIBS += -L$$DAQMX_PATH//Lib/msvc -lnidaqmx
    INCLUDEPATH += $$DAQMX_PATH/include/

    ############## NI-488.2 ##############
    LIBS += -L$$PWD/../3rdparty/ni4882-1.6 gpib-32.obj
    INCLUDEPATH += $$PWD/../3rdparty/ni4882-1.6
    DEPENDPATH += $$PWD/../3rdparty/ni4882-1.6

    ############## GSL ##############
    LIBS += -L$$PWD/../3rdparty/gsl-1.15/build.vc10/lib/Win32/release/ -lgsl
    INCLUDEPATH += $$PWD/../3rdparty/gsl-1.15
    DEPENDPATH += $$PWD/../3rdparty/gsl-1.15/gsl

    ############## Qwt-5.2.1 ##############
    DEFINES += QWT_DLL
    CONFIG(release, debug|release) {
        LIBS += -L$$PWD/../3rdparty/qwt5_msvc2010_install/lib/ -lqwt5
    } else {
        LIBS += -L$$PWD/../3rdparty/qwt5_msvc2010_install/lib/ -lqwtd5
    }
    INCLUDEPATH += $$PWD/../3rdparty/qwt5_msvc2010_install/include
    DEPENDPATH += $$PWD/../3rdparty/qwt5_msvc2010_install/include


    ############## HDF5 ##############
    DEFINES += _HDF5USEDLL_ HDF5CPP_USEDLL
    INCLUDEPATH += $$PWD/../3rdparty/hdf5-1.8.10/include
    LIBS += -L$$PWD/../3rdparty/hdf5-1.8.10/lib -lhdf5dll -lhdf5_cppdll

    ############## muParser ##############
    INCLUDEPATH += $$PWD/../3rdparty/muparser_v2_2_3/include
    LIBS += -L$$PWD/../3rdparty/muparser_v2_2_3/lib/
    CONFIG(debug, debug|release) {
        LIBS += -lmuparserd
    } else {
        LIBS += -lmuparser
    }

    ############# libmodbus ##############
    LIBMODBUS_PATH = $$PWD/../3rdparty/libmodbus-3.1.1/src
    INCLUDEPATH += $$LIBMODBUS_PATH
    LIBS += -lsetupapi -lwsock32 ws2_32.lib advapi32.lib


    ######## Win32 Libs ##########
    LIBS += winmm.lib user32.lib

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

    ############## Qwt-5.2.1 ##############
    LIBS += -lqwt
    INCLUDEPATH += /usr/include/qwt

    ############## HDF5 ##############
    #INCLUDEPATH += /usr/include
    LIBS += -lhdf5 -lhdf5_cpp

    ############## muParser ##############
    #INCLUDEPATH += /usr/include
    LIBS += -lmuparser

    ############## NI DAQmx Base #########
    CONFIG += daqmxbase
    LIBS += -lnidaqmxbase
    DEFINES += USE_DAQMX_BASE
}

OTHER_FILES += \
    ../linux_readme.txt

