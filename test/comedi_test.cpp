#include <iostream>

#include <ni6221.h>


#include <unistd.h>

using namespace std;

int comedi_test()
{
    const char* fname = "/dev/comedi0";

    try
    {
        /*ni6221 dev(fname);

        ni6221::dio* dio = dev.getLine(0,0);
        //dio->reset();
        dio->setup(true);


        ni6221::ctr* ctr = dev.getCtr(0);

        ctr->reset();
        ctr->setup_count_edges();
        ctr->arm();

        unsigned v = 0;
        for(int i=0; i<10; i++)
        {
            v = !v;
            dio->write(v);
            cout << ctr->read() << '\t' << v << endl;
            sleep(1);
        }*/

        ni6221 dev(fname);
        cout << "Board name = " << dev.board_name() << endl;
        cout << "Driver name = " << dev.driver_name() << endl;

        unsigned nd = dev.n_subdevices();
        cout << "Subdevices = " << nd << endl;

        for(unsigned is=0; is<nd; is++) {
            comedi::subdevice sd(dev, is);
            comedi_subdevice_type tp = sd.subdevice_type();
            unsigned nch = sd.n_channels();
            cout << "  " << is << ". " << comedi::subdevice::type_name(tp) << ", channels: " << nch << endl;
        }

        ni6221::ai* ai = dev.getAi();
        unsigned nr = ai->n_ranges();
        cout << "ai Ranges = " << nr << endl;
        for(unsigned ir=0; ir<nr; ir++) {
            const comedi_range *rng = ai->range(0,ir);
            cout << "  " << ir << ". [" << rng->min << ", " << rng->max << "]" << endl;
        }

        ni6221::dio* dio = dev.getPort(0);
        //dio->reset();
        dio->setup(0,true);


        ni6221::ctr* ctr = dev.getCtr(0);

        ctr->reset();
        ctr->setup_count_edges();
        ctr->arm();


        comedi::to_physical converter(ai->hardcal_converter(0,0,COMEDI_TO_PHYSICAL));

        unsigned v = 0;
        while(1)
        {
            v = !v;
            dio->write(0,v);
            double v0 = converter(ai->read(0,0,AREF_COMMON));
            cout << v0 << '\t' << ctr->read() << '\t' << dio->read(0) << endl;
            sleep(1);
        }

    }
    catch(std::runtime_error& e)
    {
        cerr << e.what() << endl;
    }

    return 0;
}


