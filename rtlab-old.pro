TEMPLATE = subdirs
CONFIG += ordered


win32:SUBDIRS += \
    core \
    test \
    wago_mbt \
    dev \
    app

unix:SUBDIRS += \
    core \
    test \
    dev \
    app
