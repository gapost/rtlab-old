#ifndef RTMODBUS_H
#define RTMODBUS_H

#include "RtInterface.h"


class RtModbus : public RtTcpip
{
    Q_OBJECT

protected:
    //modbus_t* ctx;
    void* ctx_;

public:
    RtModbus(const QString& name, RtObject* parent, const QString& ahost = QString(), uint portno = 0);
    virtual ~RtModbus();

    // io
    virtual int read(uint port, char* buff, int len, int eos = 0);
    virtual int write(uint port, const char* buff, int len, int eos = 0);

protected:
    virtual bool open_();
    virtual void close_();
};

#endif // RTMODBUS_H
