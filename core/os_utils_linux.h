#ifndef OS_UTILS_LINUX_H
#define OS_UTILS_LINUX_H

#include <time.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>

namespace os {

static inline void beep()
{
    printf("\a");
}


class critical_section
{
    pthread_mutex_t cs_mutex;
public:
    critical_section()
    {
        pthread_mutexattr_t a;
        pthread_mutexattr_init(&a);
        pthread_mutexattr_settype(&a, PTHREAD_MUTEX_RECURSIVE);

        pthread_mutex_init( &cs_mutex, &a);

        pthread_mutexattr_destroy(&a);
    }
    ~critical_section()
    {
        pthread_mutex_destroy(&cs_mutex);
    }
    void lock()
    {
        pthread_mutex_lock(&cs_mutex);
    }
    void unlock()
    {
        pthread_mutex_unlock(&cs_mutex);
    }
};

// locks a critical section, and unlocks it automatically
// when the lock goes out of scope
class auto_lock
{
public:
    auto_lock(critical_section& cs) : cs_(cs) { cs_.lock(); }
    ~auto_lock() { cs_.unlock(); };

private:
    critical_section& cs_;
};


#define SW_CLOCK_ID CLOCK_MONOTONIC

class stopwatch
{
    bool running_;
    timespec start_time_;
    timespec resol_;
    timespec total_;

    int latch(timespec& t_)
    {
        return clock_gettime(SW_CLOCK_ID,&t_);

    }

public:
    stopwatch() :  running_(false)
    {
        clock_getres(SW_CLOCK_ID,&resol_);
    }

    /**
        Start timing from 0.00.
    */
    void start()
    {
        reset();
        latch(start_time_);
        running_ = true;
    }

    /**
        Stop timing
    */
    void stop()
    {
        if (running_)
        {
            timespec t;
            latch(t);
            total_.tv_sec += t.tv_sec - start_time_.tv_sec;
            total_.tv_nsec += t.tv_nsec - start_time_.tv_nsec;
            running_ = false;
        }
    }

    /**
        Resume timing, if currently stopped.  Operation
        has no effect if Stopwatch is already running_.
    */
    void resume()
    {
        if (!running_)
        {
            latch(start_time_);
            running_ = true;
        }
    }

    void reset() { total_.tv_sec = total_.tv_nsec = 0; }

    /**
        Read current time (in seconds).
    */
    double sec()
    {
        if (running_)
        {
            stop();
            resume();
        }
        return 1.e-9*total_.tv_nsec + total_.tv_sec;
    }


    /**
        Check if clock is running.
    */
    bool is_running() const
    {
        return running_;
    }
};

/**
  Returns time in seconds since the Epoch
  */
/*int system_time(double& t)
{
    timespec ts_;
    int ret = clock_gettime(CLOCK_REALTIME, &ts_);
    t = 1.e-9*ts_.tv_nsec + ts_.tv_sec;
    return ret;
}*/

#define SIG SIGRTMIN
/** timer thread
  * A timer thread implemented using timer_create() + signal
  * + a thread that watches for the signal
  */
template<class Functor>
class timer
{
    pthread_t  tid;
    timer_t timerid;
    int signo;

    struct _thread_arg_t
    {
        int signo;
        Functor* F;
    };

    static void _thread_sigwait(void* arg)
    {
        _thread_arg_t* ta = (_thread_arg_t*)arg;
        int signo = ta->signo;
        Functor* F = ta->F;

        // Wait for the timer signal
        sigset_t set;
        sigemptyset(&set);
        sigaddset(&set, signo);

        while (1) {
            //printf("Waiting for a signal\n");
            int sig;
            int ret_val = sigwait(&set, &sig);
            if (ret_val == 0) {
                //printf("Signal number %d received\n",sig);

                // Signal processing code here
                (*F)();


            } else {
                //fatal_error(ret_val, "sigwait()");
                break;
            }
        };
    }
public:
    timer() : tid(0), timerid(0)
    {

    }
    ~timer()
    {
        stop();
    }
    bool start(Functor* f, unsigned int ms)
    {
        stop();

        // select signal
        signo = SIG;

        // Block from being delivered to calling thread
        sigset_t set;
        sigemptyset(&set);
        sigaddset(&set, signo);
        pthread_sigmask(SIG_SETMASK, &set, (sigset_t *)NULL);

        // Create the signal handling thread
        _thread_arg_t arg;
        arg.signo = signo;
        arg.F = f;
        int ret_val = pthread_create(&tid, (pthread_attr_t *)NULL,
                                 (void *(*)(void *))_thread_sigwait, &arg);
        if (ret_val!=0) {
            //fatal_error(ret_val, "pthread_create()");
            return false;
        }

        // Create timer
        struct sigevent timer_event;
        memset(&timer_event, 0, sizeof(struct sigevent));
        timer_event.sigev_notify = SIGEV_SIGNAL;
        timer_event.sigev_signo = signo;
        ret_val = timer_create(SW_CLOCK_ID, &timer_event, &timerid);
        if (ret_val!=0) {
            //fatal_error(ret_val, "timer_create()");
            timerid = 0;
            return false;
        }

        // start timer
        struct itimerspec tvalue;
        tvalue.it_value.tv_sec = ms / 1000;
        tvalue.it_value.tv_nsec = (ms % 1000) * 1000000;
        tvalue.it_interval.tv_sec = tvalue.it_value.tv_sec;
        tvalue.it_interval.tv_nsec = tvalue.it_value.tv_nsec;

        ret_val = timer_settime(timerid, 0, &tvalue, NULL);
        if (ret_val!=0) {
            //fatal_error(ret_val, "timer_settime()");
            return false;
        }

        return true;
    }
    bool is_running() const
    {
        return timerid != 0;
    }
    void stop()
    {
        if (!is_running()) return;
        timer_delete(timerid);
        pthread_cancel(tid);
        timerid = 0;
        tid = 0;
    }
};


} // namespace os



#endif // OS_UTILS_LINUX_H
