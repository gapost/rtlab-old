#ifndef STOPWATCH_H
#define STOPWATCH_H

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

#endif // STOPWATCH_H
