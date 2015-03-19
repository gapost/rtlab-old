#ifndef NI6221_H
#define NI6221_H

#include "comedi_qt.h"
//#include <stdexcept>

class ni6221 : public comedi::device
{
public:

    class ai : public comedi::subdevice
    {
        ai(const comedi::device& d, unsigned sub);
        ai();
        ai(const ai& other);
    public:
        unsigned read(unsigned ch, unsigned range, unsigned ref);

        friend class ni6221;

    };

    class ctr : public comedi::subdevice
    {
         unsigned pfi_source_;
         unsigned v_;

        ctr(const comedi::device& d, unsigned ctr_idx);
        ctr();
        ctr(const ctr& other);

    public:
        void setup_count_edges();
        void setup_input_filter(int f);
        void arm();
        unsigned read();

        friend class ni6221;
    };

    class dio : public comedi::subdevice
    {
        dio(const comedi::device& d, unsigned s);
        dio();
        dio(const dio& other);

    public:
        void setup(unsigned ch, bool out);
        unsigned read(unsigned ch);
        void write(unsigned ch, unsigned v);

        friend class ni6221;
    };

    ni6221(const char* filename);

    ctr* getCtr(int i) { return (ctr*)(subdev_[11+i]);  }
    dio* getPort(int i) { return (dio*)(i ? subdev_[7] : subdev_[2]); }
    ai* getAi() { return (ai*)(subdev_[0]); }
    comedi::subdevice* getSubDevice(int i) { return subdev_[i]; }


private:
    //ai* ai_;
    //ctr* ctr_[2];
    //dio* port_[2];
    comedi::subdevice* subdev_[14];



}; // ni6221

#endif // NI6221_H
