#ifndef RTDAQ_H
#define RTDAQ_H

#ifdef _WIN32
#include "rtdaqmxtask.h"
#elif __linux__
    #ifdef USE_COMEDI
    #include "RtComedi.h"
    #endif
#endif

#endif // RTDAQ_H
