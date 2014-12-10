#include <iostream>
#include <unistd.h>

#include "os_utils.h"

using namespace std;

void test_clock(const char* name, clockid_t id)
{
    timespec t;
    clock_getres(id, &t);
    cout << name << endl;
    cout << "  sec  " << t.tv_sec << endl;
    cout << "  nsec " << t.tv_nsec << endl << endl;
}

int test1();
int test2();

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

int main()
{
    os::ptimer<test_func> t;
    test_func f;

    f.init();
    t.start(&f,50);

    sleep(2);

    for(int i=0; i<f.n; i++)
        cout << f.d[i] << endl;

    return 0;
}

int test2()
{
    test_clock("CLOCK_REALTIME",CLOCK_REALTIME);
    test_clock("CLOCK_MONOTONIC",CLOCK_MONOTONIC);

    return 0;
}

int test1()
{
    int q = 0;
    os::stopwatch W;

    for(int k=0; k<10; k++)
    {
        W.start();
        for(int i=0; i<1000000; i++) q += i;
        cout << W.sec() << endl;
    }

    cout << q << endl;

    return 0;
}

