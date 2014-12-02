#ifndef _RTNIGPIB_H_
#define _RTNIGPIB_H_

#include "RtInterface.h"
#include "RtTypes.h"

class RtNiGpib : public RtInterface
{
	Q_OBJECT

public:
	RtNiGpib(const QString& name, RtObject* parent, uint id = 0);
	virtual ~RtNiGpib();

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
};

#endif

