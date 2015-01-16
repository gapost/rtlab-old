#include "rtgpib.h"

#ifdef _WIN32
// NI-GPIB
#include <decl-32.h>
#define LIB_NAME "NI488.2"
#elif __linux__
#include <gpib/ib.h>
#define LIB_NAME "linux-gpib"
#endif


// NI defs
    enum StatusCode {
        Dcas = 0x0001, // brd Device Clear State
        Dtas = 0x0002, // brd Device Trigger State
        Lacs = 0x0004, // brd Listener
        Tacs = 0x0008, // brd Talker
        Atn  = 0x0010, // brd Attention is asserted
        Cic  = 0x0020, // brd Controller-In-Charge
        Rem  = 0x0040, // brd Remote State
        Lok  = 0x0080, // brd Lockout State
        Cmpl = 0x0100, // dev, brd I/O completed
        Rqs  = 0x0800, // dev Device requesting service
        Srqi = 0x1000, // brd SRQ interrupt received
        End  = 0x2000, // dev, brd END or EOS detected
        Timo = 0x4000, // dev, brd Time limit exceeded
        Err  = 0x8000  // dev, brd NI-488.2 error
    };

    enum ErrorCode {
        Edvr = 0 , // 0  EDVR System error
        Ecic = 1 , // 1  ECIC Function requires GPIB interface to be CIC
        Enol = 2 , // 2  ENOL No Listeners on the GPIB
        Eadr = 3 , // 3  EADR GPIB interface not addressed correctly
        Earg = 4 , // 4  EARG Invalid argument to function call
        Esac = 5 , // 5  ESAC GPIB interface not System Controller as required
        Eabo = 6 , // 6  EABO I/O operation aborted (timeout)
        Eneb = 7 , // 7  ENEB Nonexistent GPIB interface
        Edma = 8 , // 8  EDMA DMA error
        Eoip = 10, // 10 EOIP Asynchronous I/O in progress
        Ecap = 11, // 11 ECAP No capability for operation
        Efso = 12, // 12 EFSO File system error
        Ebus = 14, // 14 EBUS GPIB bus error
        Estb = 15, // 15 ESTB Serial poll status byte queue overflow
        Esrq = 16, // 16 ESRQ SRQ stuck in ON position
        Etab = 20  // 20 ETAB Table problem
    };

    enum TimeoutCode
    {
        tNONE   = 0,   /* Infinite timeout (disabled)        */
        t10us   = 1,   /* Timeout of 10 us (ideal)           */
        t30us   = 2,   /* Timeout of 30 us (ideal)           */
        t100us  = 3,   /* Timeout of 100 us (ideal)          */
        t300us  = 4,   /* Timeout of 300 us (ideal)          */
        t1ms    = 5,   /* Timeout of 1 ms (ideal)            */
        t3ms    = 6,   /* Timeout of 3 ms (ideal)            */
        t10ms   = 7,   /* Timeout of 10 ms (ideal)           */
        t30ms   = 8,   /* Timeout of 30 ms (ideal)           */
        t100ms  = 9,   /* Timeout of 100 ms (ideal)          */
        t300ms  =10,   /* Timeout of 300 ms (ideal)          */
        t1s     =11,   /* Timeout of 1 s (ideal)             */
        t3s     =12,   /* Timeout of 3 s (ideal)             */
        t10s    =13,   /* Timeout of 10 s (ideal)            */
        t30s    =14,   /* Timeout of 30 s (ideal)            */
        t100s   =15,   /* Timeout of 100 s (ideal)           */
        t300s   =16,   /* Timeout of 300 s (ideal)           */
        t1000s  =17    /* Timeout of 1000 s (ideal)          */
    };

RtGpib::RtGpib(const QString& name, RtObject* parent, uint id) :
RtInterface(name,LIB_NAME,parent,id)
{
    // 32 empty places
    ports_.fill(0, 32);
}
RtGpib::~RtGpib()
{
}

void RtGpib::pushGpibError(int code, const QString& comm)
{
    pushError(errorCode(code),comm);
}

const char* RtGpib::errorCode(int idx)
{
    static const char* code[] = {
        "EDVR: System error",
        "ECIC: Function requires GPIB interface to be CIC",
        "ENOL: No Listeners on the GPIB",
        "EADR: GPIB interface not addressed correctly",
        "EARG: Invalid argument to function call",
        "ESAC: GPIB interface not System Controller as required",
        "EABO: I/O operation aborted (timeout)",
        "ENEB: Nonexistent GPIB interface",
        "EDMA: DMA error",
        "Unknown Error",
        "EOIP: Asynchronous I/O in progress",
        "ECAP: No capability for operation",
        "EFSO: File system error",
        "Unknown Error",
        "EBUS: GPIB bus error",
        "ESTB: Serial poll status byte queue overflow",
        "ESRQ: SRQ stuck in ON position",
        "Unknown Error",
        "Unknown Error",
        "Unknown Error",
        "ETAB: Table problem",
        "Unknown Error"
    };
    const int N_ERROR_CODES = 21;

    if (idx>=0 && idx<N_ERROR_CODES) return code[idx];
    else return code[N_ERROR_CODES];
}

bool RtGpib::open_()
{
    static const uint TimeoutValues[] = { // in ms
        0, // inf (disabled)
        0, 0, 0, 0, // less than ms
        1, 3,
        10, 30,
        100, 300,
        1000,    3000,
        10000,   30000,
        100000,  300000, 1000000
    };

    if (isOpen()) return true;

    os::auto_lock L(comm_lock);

    SendIFC(addr_);

    if (ibsta & Err)
    {
        pushGpibError(
            iberr,
            QString("SendIFC(%1)").arg(addr_)
            );
    }
    else RtInterface::open_();

    if (isOpen())
    {
        int c; //(timeout code)
        if (timeout_==0) c = tNONE;
        else
        {
            c = 5;
            while(c<17 && timeout_>TimeoutValues[c]) c++;
        }
        ibconfig(addr_,IbcTMO,c);
    }

    emit propertiesChanged();
    return isOpen();
}

bool RtGpib::open_port(uint id, RtDevice* dev)
{
    os::auto_lock L(comm_lock);
    if (RtInterface::open_port(id,dev))
    {
        Addr4882_t dd[2] = {id, NOADDR};
        EnableRemote(addr_, dd);
        if (ibsta & Err)
        {
            pushGpibError(
                iberr,
                QString("EnableRemote(%1,%2)").arg(addr_).arg(id)
            );
            RtInterface::close_port(id);
            return false;
        }
        else return true;
    }
    else return false;
}

void RtGpib::close_port(uint id)
{
    os::auto_lock L(comm_lock);
    Addr4882_t dd[2] = {id, NOADDR};
    EnableLocal(addr_, dd);
    if (ibsta & Err)
    {
        pushGpibError(
            iberr,
            QString("EnableLocal(%1,%2)").arg(addr_).arg(id)
        );
    }
    RtInterface::close_port(id);
}

void RtGpib::clear_port(uint id)
{
    os::auto_lock L(comm_lock);
    Addr4882_t dd = (Addr4882_t)id;
    DevClear(addr_,dd);
    if (ibsta & Err)
    {
        pushGpibError(
            iberr,
            QString("DevClear(%1,%2)").arg(addr_).arg(id)
            );
    }
}

int RtGpib::read(uint port, char* data, int len, int eoi)
{
    os::auto_lock L(comm_lock);
    Receive(addr_, (Addr4882_t) port, data, len, eoi);
    if (ibsta & Err)
    {
        pushGpibError(
            iberr,
            QString("Recieve(%1,%2,0x%3,%4,%5)").arg(addr_).arg(port).arg((uint)data,0,8).arg(len).arg(eoi)
            );
        return 0;
    }
    else return ibcntl;
}

int RtGpib::readStatusByte(uint port)
{
    os::auto_lock L(comm_lock);
    short result;
    ReadStatusByte (addr_, (Addr4882_t) port, &result);
    if (ibsta & Err)
    {
        pushGpibError(
            iberr,
            QString("ReadStatusByte(%1,%2,x)").arg(addr_).arg(port)
            );
        return 0;
    }
    else return result;
}

int RtGpib::write(uint port, const char* buff, int len, int e)
{
    os::auto_lock L(comm_lock);
    int eot =  (e & 0x0000FF00) >> 8;
    Send(addr_, (Addr4882_t) port, const_cast<char*>(buff), len, eot);
    if (ibsta & Err)
    {
        pushGpibError(
            iberr,
            QString("Send(%1,%2,%3,%4,%5)").arg(addr_).arg(port)
                .arg(QString(QByteArray(buff,len))).arg(len).arg(eot)
            );
        return 0;
    }
    else return len; // ibcntl; linux-gpib sends +1 byte
}

void RtGpib::clear_()
{
    os::auto_lock L(comm_lock);
    SendIFC(addr_);
    if (ibsta & Err)
    {
        pushGpibError(
            iberr,
            QString("SendIFC(%1)").arg(addr_)
            );
    }
}

RtIntVector RtGpib::findListeners()
{
    os::auto_lock L(comm_lock);

    Addr4882_t addrlist[32], results[32];
    int limit = 31;

    RtIntVector Listeners;

    for(int i=0; i<limit; i++) addrlist[i] = i+1;
    addrlist[31] = NOADDR;

    FindLstn(addr_, addrlist, results, limit);

    if (ibsta & Err)
    {
        pushGpibError(
            iberr,
            QString("FindLstn(%1,array,array,30)").arg(addr_)
            );
    }
    else
    {
        for(int i=0; i<ibcntl; ++i) Listeners.push_back(results[i]);
    }
    return Listeners;
}


