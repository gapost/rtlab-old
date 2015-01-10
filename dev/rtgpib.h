#ifndef RTGPIB_H
#define RTGPIB_H

#include "RtInterface.h"
#include "RtTypes.h"

class RtGpib : public RtInterface
{
    Q_OBJECT

public:
    RtGpib(const QString& name, RtObject* parent, uint id = 0);
    virtual ~RtGpib();

    // getters

    // setters

    // io
    virtual bool open_port(uint i, RtDevice*);
    virtual void close_port(uint i);
    virtual void clear_port(uint i);
    virtual int read(uint port, char* buff, int len, int eos = 0);
    virtual int write(uint port, const char* buff, int len, int eos = 0);

    // diagnostics
    virtual int readStatusByte(uint port);

    virtual bool isValidPort(uint i) { return i>0 && i<32; }

protected:
    virtual bool open_();
    virtual void clear_();

public slots:
    RtIntVector findListeners();

private:
    // error reporting
    void pushGpibError(int code, const QString& comm);
    static const char* errorCode(int idx);

};

#endif // RTGPIB_H
