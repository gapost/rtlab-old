#ifndef OS_UTILS_WIN32_H
#define OS_UTILS_WIN32_H

#include <sys/timeb.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

namespace os {

inline void beep()
{
    MessageBeep(0xFFFFFFFF);
}

// a win32 critical section

/**
 * @brief A critical section for thread syncronization
 *
 */
class critical_section
{
public:
        critical_section()  { InitializeCriticalSection(&section); }
        ~critical_section() { DeleteCriticalSection(&section); }
        void lock()         { EnterCriticalSection(&section); }
        void unlock()       { LeaveCriticalSection(&section); }

private:
        CRITICAL_SECTION section;
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


/**
 * @brief A high resolution stop-watch
 *
 * In Win32 implemented with the QueryPerformanceCounter function.
 *
 */
class stopwatch
{
private:
    typedef __int64 i64;

    bool running_;
    i64 start_time_;
    i64 t_;
    i64 total_;
    i64 counts_per_sec_;

    const i64& latch()
    {
        static i64 t;
        QueryPerformanceCounter((LARGE_INTEGER*)&t_);
        return t_;
    }

public:
    stopwatch() :  running_(false), start_time_(0), total_(0)
    {
        QueryPerformanceFrequency((LARGE_INTEGER*)&counts_per_sec_);
    }

    /**
        Start timing from 0.00.
    */
    void start()
    {
        start_time_ = latch();
        total_ = 0;
        running_ = true;
    }

    /**
        Stop timing
    */
    void stop()
    {
        if (running_)
        {
             total_ += (latch() - start_time_);
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
            start_time_ = latch();
            running_ = true;
        }
    }

    void reset() { total_ = 0; }

    /**
        Read current time (in seconds).
    */
    double sec()
    {
        return 1.*ticks()/counts_per_sec_;
    }

    /**
        Read current time (in base frequency periods).
    */
    const i64& ticks()
    {
        if (running_)
        {
            stop();
            resume();
        }
        return total_;
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
    __timeb32 t;
    _ftime32(&t);
    return RtTimeValue(t.time + 0.001*t.millitm);
}*/

template<class Functor>
class timer
{
private:
    unsigned int timerId;

    // windows timer callback
    static void CALLBACK _timerProc(UINT wTimerID, UINT msg, DWORD dwUser, DWORD dw1, DWORD dw2)
    {
        wTimerID = wTimerID;
        msg = msg;
        dw1 = dw1;
        dw2 = dw2;
        Functor* f = reinterpret_cast<Functor*>(dwUser);
        if (f) (*f)();
    }

public:

    timer() : timerId(0)
    {
        timeBeginPeriod(1U);
    }
    virtual ~timer()
    {
        stop();
        timeEndPeriod(1U);
    }
    bool start(Functor* f, unsigned int ms)
    {
        stop();
        timerId = timeSetEvent(ms,0,_timerProc,(DWORD)f,
            TIME_CALLBACK_FUNCTION | TIME_PERIODIC); // TIME_KILL_SYNCHRONOUS only winXP
        return timerId != 0;
    }
    void stop()
    {
        if (timerId)
        {
            timeKillEvent(timerId);
            timerId = 0; // wait for thread to die
        }
    }
    bool is_running() const { return timerId != 0; }
};

} // namespace os


#endif // WIN32UTILS_H
