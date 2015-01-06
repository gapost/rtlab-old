TEMPLATE = subdirs
CONFIG += ordered


win32:SUBDIRS += \
    core \
    test \
    ni6602 \
    wago_mbt \
    dev \
    app

unix:SUBDIRS += \
    core \
    test \
    dev \
    app
