#ifndef THREAD_H
#define THREAD_H

template < class Functor >
class thread
{
    pthread_t  tid;
    Functor* F;

    volatile int running_;
    mutex mtx;
    wait_condition cond;

    void* thread_func_()
    {
        mtx.lock();
        running_ = 1;
        cond.signal();
        mtx.unlock();

        (*F)();

        running_ = 0;

        return NULL;
    }

    static void *thread_func(void *arg)
    {
        return ((thread*)arg)->thread_func_();
    }

public:
    thread() : tid(0), F(0), running_(0)
    {

    }
     bool start(Functor* f)
    {
        if (running_) return false;

        //store the func
        F = f;

        // lock mutex
        mtx.lock();

        // Create the timer thread
        if (pthread_create(&tid, (pthread_attr_t *)NULL, thread_func, this)!=0)
        {
            mtx.unlock();
            return false;
        }

        // wait for thread to start
        cond.wait(mtx);
        mtx.unlock();

        return true;
    }
    bool is_running() const
    {
        return running_;
    }
    void wait()
    {
        if (!running_) return;

        void* ret;
        pthread_join(tid,&ret);
    }
};


/** timer thread
  * A timer thread implemented using timer_create() + signal
  * + a thread that watches for the signal
  */
template<class Functor>
class timer
{
    typedef timer<Functor> self_t;

    struct myFunctor
    {
        timer* t;
        void operator()()
        {
            t->timer_func();
        }
    };

    thread<myFunctor> thread_;

    int timer_fd;
    unsigned long long wakeups_missed;
    unsigned int period; // ms
    myFunctor myF;
    Functor* F;

    volatile int continue_;

    // Arm/disarm the timer
    int arm (unsigned ms)
    {
        unsigned int ns;
        unsigned int sec;
        itimerspec itval;

        if (ms) {
            sec = period/1000;
            ns = (period - (sec * 1000)) * 1000000;
        }
        else sec=ns=0;
        itval.it_interval.tv_sec = sec;
        itval.it_interval.tv_nsec = ns;
        itval.it_value.tv_sec = sec;
        itval.it_value.tv_nsec = ns;
        return timerfd_settime (timer_fd, 0, &itval, NULL);
    }

    int wait_period ()
    {
        unsigned long long missed;
        int ret;

        /* Wait for the next timer event. If we have missed any the
           number is written to "missed" */
        ret = read (timer_fd, &missed, sizeof (missed));
        if (ret == -1)
        {
            //perror ("read timer");
            return ret;
        }

        wakeups_missed += missed;

        return ret;
    }

    void timer_func()
    {
        arm(period);
        wait_period();
        while (continue_)
        {
            F->operator()();
            wait_period();
        }
        arm(0);
    }

public:
    timer()
    {
        timer_fd = timerfd_create(CLOCK_MONOTONIC,0);
    }
    ~timer()
    {
        stop();
        if (timer_fd!=-1) close(timer_fd);
    }
    bool start(Functor* f, unsigned int ms)
    {
        stop();

        // copy options
        F = f;
        period = ms;
        myF.t = this;
        continue_ = true;

        return thread_.start(&myF);
    }
    bool is_running() const
    {
        return thread_.is_running();
    }
    void stop()
    {
        if (!thread_.is_running()) return;

        continue_ = false;
        thread_.wait();
    }
};

#endif // THREAD_H
