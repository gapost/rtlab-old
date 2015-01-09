#include <iostream>

#include "daqmx.h"

using namespace std;

int main()
{
    vector<string> v,ch;


    daqmx::sys::deviceNames(v);
    int nd = v.size();
    cout << "Found " << nd << " devices." << endl;
    for(int i=0; i<nd; ++i)
    {
        daqmx::dev D(v[i]);

        cout << i+1 << ". " << D.name().c_str() << endl;

        D.aiChannels(ch);
        cout << "  Analog Input Channels  : " << ch.size() << endl;
        D.aoChannels(ch);
        cout << "  Analog Output Channels : " << ch.size() << endl;
        D.ciChannels(ch);
        cout << "  Counter Input Channels  : " << ch.size() << endl;
        D.coChannels(ch);
        cout << "  Counter Output Channels : " << ch.size() << endl;
        D.diLines(ch);
        cout << "  Digital Input Channels  : " << ch.size() << endl;
        D.doLines(ch);
        cout << "  Digital Output Channels : " << ch.size() << endl;

    }
    cout << endl;



    return 0;
}


