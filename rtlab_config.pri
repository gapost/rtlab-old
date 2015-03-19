# Set Configuration options for RtLab


# Options for win32 platform
win32 {
# Uncomment next line to include NI-DAQmx support
CONFIG += nidaqmx
}

# Options for linux platform
unix {
# Uncomment next line to include comedi driver support
CONFIG += comedi
}
