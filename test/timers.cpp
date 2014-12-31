#include <iostream>
#include <cstdlib>

#include <RtTimeValue.h>

#ifdef __linux__

#include <unistd.h>

inline void sleep_for_secs(int s)
{
    sleep(s);
}

#else //win32

inline void sleep_for_secs(int s)
{
    Sleep(s*1000);
}

#endif

#include <os_utils.h>

using namespace std;

struct test_func
{
    os::stopwatch w;
    double d[2000];
    int n;
    void init()
    {
        w.start();
        n=0;
    }
    void operator()()
    {
        //double t = w.sec();
        w.stop();
        //cout << w.sec()*1000 << endl;
        d[n++] = w.sec()*1000;
        w.start();
    }
};

int timers_test(int argc, char* argv[])
{
    char* usage =
            "Usage:\n"
            "  timetest p t\n"
            "  p  :  period in ms\n"
            "  t  :  time in s\n";

    if (argc!=3)
    {
        cout << usage << endl;
        return -1;
    }

    int period = atoi(argv[1]); // ms
    int ts = atoi(argv[2]); // s

    os::timer<test_func> t;
    test_func f;

    f.init();
    bool ret = t.start(&f,period);
    // cout << ret << endl;

    sleep_for_secs(ts);

    t.stop();

    for(int i=0; i<f.n; i++)
        cout << f.d[i] << endl;



    return 0;
}

int test_timevalue()
{
    RtTimeValue tv[10];
    for(int i=0; i<10; i++)
    {
        tv[i] = RtTimeValue::now();
        cout << tv[i] << '\t' << tv[i]-tv[0] << '\t' << tv[i].toString().toLatin1().constData() << endl;
        sleep_for_secs(1);
    }

    return 0;
}

