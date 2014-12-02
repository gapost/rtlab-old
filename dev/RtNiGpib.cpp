#include "RtNiGpib.h"

// NI-GPIB
#include <decl-32.h>

RtNiGpib::RtNiGpib(const QString& name, RtObject* parent, uint id) :
RtInterface(name,"NI-GPIB",parent,id)
{
	// 32 empty places
	ports_.fill(0, 32);
}
RtNiGpib::~RtNiGpib()
{
}

void RtNiGpib::pushGpibError(int code, const QString& comm)
{
	pushError(errorCode(code),comm);
}

const char* RtNiGpib::errorCode(int idx)
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

bool RtNiGpib::open_()
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

bool RtNiGpib::open_port(uint id, RtDevice* dev)
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

void RtNiGpib::close_port(uint id)
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

void RtNiGpib::clear_port(uint id)
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

int RtNiGpib::read(uint port, char* data, int len, int eoi)
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

int RtNiGpib::readStatusByte(uint port)
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

int RtNiGpib::write(uint port, const char* buff, int len, int e)
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
	else return ibcntl;
}

void RtNiGpib::clear_()
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

RtIntVector RtNiGpib::findListeners()
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

