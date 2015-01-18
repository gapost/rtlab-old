#ifndef OS_UTILS_LINUX_H
#define OS_UTILS_LINUX_H

#include <time.h>
#include <pthread.h>
#include <sys/timerfd.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

namespace os {

static inline void beep()
{
    printf("\a");
}

#include "sync_objects.h"
#include "stopwatch.h"
#include "thread.h"






} // namespace os



#endif // OS_UTILS_LINUX_H
