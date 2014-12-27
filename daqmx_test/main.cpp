#include <iostream>

#include "daqmx.h"

using namespace std;

int main()
{
    vector<string> v;


    daqmx::sys::deviceNames(v);
    for(int i=0; i<(int)v.size(); ++i)
    {
        cout << v[i] << endl;
    }
    cout << endl;

    std::string devname(v[1]);

    v.clear();
    daqmx::dev::aiChannels(devname,v);
    for(int i=0; i<(int)v.size(); ++i)
    {
        cout << v[i] << endl;
    }
    cout << endl;

    v.clear();
    daqmx::dev::aoChannels(devname,v);
    for(int i=0; i<(int)v.size(); ++i)
    {
        cout << v[i] << endl;
    }
    cout << endl;

    v.clear();
    daqmx::dev::ciChannels(devname,v);
    for(int i=0; i<(int)v.size(); ++i)
    {
        cout << v[i] << endl;
    }
    cout << endl;

    v.clear();
    daqmx::dev::coChannels(devname,v);
    for(int i=0; i<(int)v.size(); ++i)
    {
        cout << v[i] << endl;
    }
    cout << endl;

    v.clear();
    daqmx::dev::diLines(devname,v);
    for(int i=0; i<(int)v.size(); ++i)
    {
        cout << v[i] << endl;
    }
    cout << endl;

    v.clear();
    daqmx::dev::doLines(devname,v);
    for(int i=0; i<(int)v.size(); ++i)
    {
        cout << v[i] << endl;
    }
    cout << endl;

    return 0;
}


