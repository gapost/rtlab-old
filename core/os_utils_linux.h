#ifndef OS_UTILS_LINUX_H
#define OS_UTILS_LINUX_H

#include <time.h>
#include <pthread.h>
#include <sys/timerfd.h>
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


/** timer thread
  * A timer thread implemented using timer_create() + signal
  * + a thread that watches for the signal
  */
template<class Functor>
class timer
{
    typedef timer<Functor> self_t;
    pthread_t  tid;
    unsigned long long wakeups_missed;
    int timer_fd;
    unsigned int period;
    Functor* F;

    // Create a timer_fd object
    static int make_periodic (self_t *me)
    {
        int ret;
        unsigned int ns;
        unsigned int sec;
        itimerspec itval;

        /* Create the timer */
        me->timer_fd = timerfd_create (SW_CLOCK_ID, 0);
        me->wakeups_missed = 0;
        if (me->timer_fd == -1)
            return me->timer_fd;

        /* Make the timer periodic */
        sec = me->period/1000;
        ns = (me->period - (sec * 1000)) * 1000000;
        itval.it_interval.tv_sec = sec;
        itval.it_interval.tv_nsec = ns;
        itval.it_value.tv_sec = sec;
        itval.it_value.tv_nsec = ns;
        ret = timerfd_settime (me->timer_fd, 0, &itval, NULL);
        return ret;
    }

    static void wait_period (self_t *me)
    {
        unsigned long long missed;
        int ret;

        /* Wait for the next timer event. If we have missed any the
           number is written to "missed" */
        ret = read (me->timer_fd, &missed, sizeof (missed));
        if (ret == -1)
        {
            //perror ("read timer");
            return;
        }

        me->wakeups_missed += missed;
    }

    static void *thread_func(void *arg)
    {
        self_t* me = (self_t*)arg;

        make_periodic (me);
        while (1)
        {
            wait_period (me);
            me->F->operator()();
        }
        return NULL;
    }

public:
    timer() : tid(0), timer_fd(0)
    {

    }
    ~timer()
    {
        stop();
    }
    bool start(Functor* f, unsigned int ms)
    {
        stop();

        // copy options
        F = f;
        period = ms;

        // Create the timer thread
        int ret = pthread_create(&tid, (pthread_attr_t *)NULL, thread_func, this);
                               //  (void *(*)(void *))_thread_sigwait, &this);
        if (ret!=0) {
            //fatal_error(ret_val, "pthread_create()");
            return false;
        }



        return true;
    }
    bool is_running() const
    {
        return timer_fd != 0;
    }
    void stop()
    {
        if (!is_running()) return;
        pthread_cancel(tid);
        tid = 0;
    }
};


} // namespace os



#endif // OS_UTILS_LINUX_H
