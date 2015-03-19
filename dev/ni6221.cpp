#include "ni6221.h"

#include <sstream>

/* PCI 6221 (37-pin) Counter pins
Signal		Pin	Line
CTR 0 SRC	13	PFI 0
CTR 0 GATE	32	PFI 1
CTR 0 AUX	33	PFI 2
CTR 0 OUT	17	PFI 6
CTR 0 A		13	PFI 0
CTR 0 Z		32	PFI 1
CTR 0 B		33	PFI 2
CTR 1 SRC	15	PFI 3
CTR 1 GATE	34	PFI 4
CTR 1 AUX	35	PFI 5
CTR 1 OUT	36	PFI 7
CTR 1 A		15	PFI 3
CTR 1 Z		34	PFI 4
CTR 1 B		35	PFI 5
FREQ OUT	35	PFI 5
*/

ni6221::ni6221(const char* filename) : comedi::device(filename)
{
    for(int i=0; i<14; i++) subdev_[i] = 0;
    subdev_[0] = new ai(*this,0);
    subdev_[11]  = new ctr(*this,11);
    subdev_[12] = new ctr(*this,12);
    subdev_[2] = new dio(*this,2);
    subdev_[7] = new dio(*this,7); // PFI
}

ni6221::ctr::ctr(const device &d, unsigned i) : comedi::subdevice(d,i)
{
    // NI GPCT pfi pins for counters
    static const int pfi_[2][4] = {
   //    SRC, GATE, AUX, OUT
        { 0,    1,   2,   6  },
        { 3,    4,   5,   7  }
    };
    unsigned ctr_idx = i - 11; // subdevice 11 is the 1st counter
    pfi_source_ = NI_GPCT_PFI_CLOCK_SRC_BITS(pfi_[ctr_idx][0]);
}

void ni6221::ctr::setup_count_edges()
{
    v_ = 0;
    // choose correct clock source
    set_clock_source(0, pfi_source_, 0);
    // disable gate
    set_gate_source(0, 0, NI_GPCT_DISABLED_GATE_SELECT );

    lsampl_t counter_mode = NI_GPCT_COUNTING_MODE_NORMAL_BITS;
    // output pulse on terminal count (doesn't really matter for this application)
    counter_mode |= NI_GPCT_OUTPUT_TC_PULSE_BITS;
    /* Don't alternate the reload source between the load a and load b registers.
        Doesn't really matter here, since we aren't going to be reloading the counter.
    */
    counter_mode |= NI_GPCT_RELOAD_SOURCE_FIXED_BITS;
    // count up
    counter_mode |= NI_GPCT_COUNTING_DIRECTION_UP_BITS;
    // don't stop on terminal count
    counter_mode |= NI_GPCT_STOP_ON_GATE_BITS;
    // don't disarm on terminal count or gate signal
    counter_mode |= NI_GPCT_NO_HARDWARE_DISARM_BITS;
    set_counter_mode(0, counter_mode);

    /* set initial counter value by writing to channel 0.  The "load a" and "load b" registers can be
    set by writing to channels 1 and 2 respectively. */
    data_write(0, 0, 0, 0);
}

void ni6221::ctr::setup_input_filter(int f)
{
    unsigned fc;
    switch (f)
    {
    case 3:
        fc = NI_PFI_FILTER_2550us; break;
    case 2:
        fc = NI_PFI_FILTER_6425ns; break;
    case 1:
        fc = NI_PFI_FILTER_125ns; break;
    case 0:
    default:
        fc = NI_PFI_FILTER_OFF; break;
    }
    set_filter(pfi_source_, fc);
    // subdev_ may be must be no 7
}

void ni6221::ctr::arm()
{
    comedi::subdevice::arm(NI_GPCT_ARM_IMMEDIATE);
}

unsigned ni6221::ctr::read()
{
    unsigned d = (unsigned)data_read(0,0,0);
    unsigned r = d - v_;
    v_ = d;
    return r;
}

ni6221::dio::dio(const device &d, unsigned s) : comedi::subdevice(d,s)
{}

void ni6221::dio::setup(unsigned ch, bool out)
{
    dio_config(ch, out ? COMEDI_OUTPUT : COMEDI_INPUT);

    if (out && index()==7 ) // PFI port
        set_routing(ch,NI_PFI_OUTPUT_PFI_DO);
}

unsigned ni6221::dio::read(unsigned ch_)
{
    return dio_read(ch_);
}
void ni6221::dio::write(unsigned ch_,unsigned v)
{
    dio_write(ch_, v);
}

ni6221::ai::ai(const comedi::device &d, unsigned sub) : comedi::subdevice(d,sub)
{   }

unsigned ni6221::ai::read(unsigned ch, unsigned range, unsigned ref)
{
    return data_read(ch,range,ref);
}
