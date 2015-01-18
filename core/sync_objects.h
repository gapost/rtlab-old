#ifndef SYNC_OBJECTS_H
#define SYNC_OBJECTS_H

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
    int lock()
    {
        return pthread_mutex_lock(&cs_mutex);
    }
    int unlock()
    {
        return pthread_mutex_unlock(&cs_mutex);
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

class mutex
{
    pthread_mutex_t cs_mutex;
public:
    mutex()
    {
        pthread_mutexattr_t a;
        pthread_mutexattr_init(&a);
        pthread_mutexattr_settype(&a, PTHREAD_MUTEX_NORMAL);

        pthread_mutex_init( &cs_mutex, &a);

        pthread_mutexattr_destroy(&a);
    }
    ~mutex()
    {
        pthread_mutex_destroy(&cs_mutex);
    }
    int lock()
    {
        return pthread_mutex_lock(&cs_mutex);
    }
    int unlock()
    {
        return pthread_mutex_unlock(&cs_mutex);
    }
    pthread_mutex_t& handle()
    {
        return cs_mutex;
    }
};

// locks a mutex, and unlocks it automatically
// when the lock goes out of scope
class mutex_lock
{
public:
    mutex_lock(mutex& cs) : cs_(cs) { cs_.lock(); }
    ~mutex_lock() { cs_.unlock(); };

private:
    mutex& cs_;
};

class wait_condition
{
    pthread_cond_t cond;
public:
    wait_condition()
    {
        pthread_cond_init(&cond,NULL);
    }
    ~wait_condition()
    {
        pthread_cond_destroy(&cond);
    }
    int signal()
    {
        return pthread_cond_signal(&cond);
    }
    int wait(mutex& mtx)
    {
        return pthread_cond_wait(&cond,&(mtx.handle()));
    }
};

#endif // SYNC_OBJECTS_H
