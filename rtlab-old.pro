TEMPLATE = subdirs
CONFIG += ordered


win32:SUBDIRS += \
    core \
    test \
    ni6602 \
    wago_mbt \
    dev \
    app \
    daqmx_test \
    qtdaqmx

unix:SUBDIRS += \
    core \
    test \
    dev \
    app
