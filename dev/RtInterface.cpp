#include "RtInterface.h"
#include "RtDevice.h"
#include "RtEnumHelper.h"

RtInterface::RtInterface(const QString& name, const QString& descr, RtObject* parent, uint id) :
RtObject(name,descr,parent), addr_(id), timeout_(300), isOpen_(false)
{
}

RtInterface::~RtInterface(void)
{
}
void RtInterface::detach()
{
	close_();
	RtObject::detach();
}

void RtInterface::setAddress(uint v)
{
	if (throwIfOpen()) return;
	else
	{
		addr_ = v;
		emit propertiesChanged();
	}
}
void RtInterface::setTimeout(uint v)
{
	if (throwIfOpen()) return;
	else
	{
		setTimeout_(v);
		emit propertiesChanged();
	}
}

bool RtInterface::throwIfOpen()
{
	if (isOpen_) throwScriptError("Not possible while interface is open");
	return isOpen_;
}

bool RtInterface::open_port(uint i, RtDevice* dev)
{
	if (!isOpen_) return false;
	os::auto_lock L(comm_lock);
	if (!isValidPort(i) || ports_[i]!=0) return false;
	else { ports_[i] = dev; return true; }
}

void RtInterface::close_port(uint i)
{
	os::auto_lock L(comm_lock);
	if (isValidPort(i)) ports_[i]=0;
}

int RtInterface::read(uint port, char* buff, int len, int eos)
{
	port=port;
	buff=buff;
	len=len;
	eos=eos;
	return 0;
}

int RtInterface::write(uint port, const char* buff, int len, int eos)
{
	port=port=buff[0];
	len=len;
	eos=eos;
	return 0;
}

void RtInterface::close_()
{
	os::auto_lock L(comm_lock);
	if (!isOpen()) return;
	for(int i=0; i<ports_.size(); ++i)
	{
		RtDevice* dev = ports_[i];
		if (dev!=0) dev->forcedOffline(QString("Interface %1 closed").arg(fullName()));
		ports_[i] = 0;
	}
	isOpen_ = false;
	emit propertiesChanged();
}

//**********************************************************************//
Q_SCRIPT_ENUM(BaudRate, RtSerialInterface)
Q_SCRIPT_ENUM(Parity, RtSerialInterface)
Q_SCRIPT_ENUM(DataBits, RtSerialInterface)
Q_SCRIPT_ENUM(StopBits, RtSerialInterface)
Q_SCRIPT_ENUM(Handshake, RtSerialInterface)

RtSerialInterface::RtSerialInterface(const QString& name, const QString& descr, RtObject* parent, uint id) :
RtInterface(name, descr, parent, id), 
baud_(bps9600), parity_(None), databits_(DataBits8), 
stopbits_(StopBits1), handshake_(No)
{
}

RtSerialInterface::~RtSerialInterface(void)
{
}

void RtSerialInterface::registerTypes(QScriptEngine* e)
{
	qScriptRegisterBaudRate(e);
	qScriptRegisterParity(e);
	qScriptRegisterDataBits(e);
	qScriptRegisterStopBits(e);
	qScriptRegisterHandshake(e);
	RtInterface::registerTypes(e);
}

QString RtSerialInterface::name() const
{
	return QString("COM%1").arg(addr_);
}

void RtSerialInterface::setBaud(BaudRate v)
{
	if (throwIfOpen()) return;
	else
	{
		baud_ = v;
		emit propertiesChanged();
	}
}
void RtSerialInterface::setParity(Parity v)
{
	if (throwIfOpen()) return;
	else
	{
		parity_ = v;
		emit propertiesChanged();
	}
}
void RtSerialInterface::setDatabits(DataBits v)
{
	if (throwIfOpen()) return;
	else
	{
		databits_ = v;
		emit propertiesChanged();
	}
}
void RtSerialInterface::setStopbits(StopBits v)
{
	if (throwIfOpen()) return;
	else
	{
		stopbits_ = v;
		emit propertiesChanged();
	}
}
void RtSerialInterface::setHandshake(Handshake v)
{
	if (throwIfOpen()) return;
	else
	{
		handshake_ = v;
		emit propertiesChanged();
	}
}


int RtSerialInterface::read(uint port, char* buff, int len, int eos)
{
	os::auto_lock L(comm_lock);

	port=port;

	if (!port_.isopen()) return 0;

	int read = 0;
	char c;
	char eos_char = eos & 0xFF;
	bool ok;
	while (ok = port_.getch(c))
	{
		buff[read++] = c;
		if (eos_char && c==eos_char)
		{
			read--;
			break;
		}
		if (read==len) break;
	}
	if (!ok)
	{
		pushError("Read char failed", "possibly timed-out");
	}
	return ok ? read : 0;
}

int RtSerialInterface::write(uint port, const char* buff, int len, int eos)
{
	os::auto_lock L(comm_lock);

	port=port;

	if (!port_.isopen()) return 0;

	int written = 0;
	bool ok;
	while(written<len && (ok=port_.putch(buff[written])) ) written++;
	char eos_char = eos & 0xFF;
	if (eos_char && ok) ok=port_.putch(eos_char);
	if (!ok)
	{
		pushError("Write char failed");
	}
	return ok ? written : 0;
}

bool RtSerialInterface::open_()
{
	if (isOpen()) return true;
	os::auto_lock L(comm_lock);
	if (port_.open(name().toLatin1(),
		(WIOS::BaudRate)baud_,
		(WIOS::Parity)parity_,
		(WIOS::DataBits)databits_,
		(WIOS::StopBits)stopbits_,
		(WIOS::Handshake)handshake_,
		timeout_))
		RtInterface::open_();
	else pushError(QString("Open %1 failed").arg(name())); 
	emit propertiesChanged();
	return isOpen();
}

void RtSerialInterface::close_()
{
	os::auto_lock L(comm_lock);
	RtInterface::close_();
	port_.close();
}

void RtSerialInterface::clear_()
{
	os::auto_lock L(comm_lock);
	if (port_.isopen()) port_.purge();
}

RtRS232::RtRS232(const QString& name, RtObject* parent, uint id) : 
RtSerialInterface(name,"RS232 serial interface",parent, id)
{
	// only one port
	ports_.push_back((RtDevice*)0);
}
RtRS232::~RtRS232()
{
}

//**********************************************************************//

RtTcpip::RtTcpip(const QString& name, RtObject* parent, const QString& host, uint portn) :
RtInterface(name, "tcp/ip", parent, 0),
host_(host), port_(portn)
{
	// only one port
	ports_.push_back((RtDevice*)0);
}

RtTcpip::~RtTcpip(void)
{
}

void RtTcpip::setPort(uint p)
{
	if (throwIfOpen()) return;
	else
	{
		port_ = p;
		emit propertiesChanged();
	}
}
void RtTcpip::setHost(const QString& h)
{
	if (throwIfOpen()) return;
	else
	{
		host_.setAddress(h);
		emit propertiesChanged();
	}
}

int RtTcpip::read(uint port, char* buff, int len, int eos)
{
	os::auto_lock L(comm_lock);

	port=port;
	eos=eos;

	if (!socket_.isOpen()) return 0;

	if (socket_.bytesAvailable() || socket_.waitForReadyRead(timeout_))
		return socket_.read(buff,len);
	else
	{
		pushError("Read timed-out");
		return 0;
	}
}

int RtTcpip::write(uint port, const char* buff, int len, int eos)
{
	os::auto_lock L(comm_lock);

	port=port;
	eos=eos;

	if (!socket_.isOpen()) return 0;

	int w = socket_.write(buff,len);
	if (w && socket_.waitForBytesWritten(timeout_))
		return w;
	else
	{
		pushError("Write timed-out");
		return 0;
	}

}

bool RtTcpip::open_()
{
	if (isOpen() && socket_.isOpen()) return true;
	os::auto_lock L(comm_lock);

	socket_.connectToHost(host_,port_);
	if (!(isOpen_=socket_.waitForConnected(timeout_)))
	{
		socket_.close();
		pushError("Open tcpip timeout"); 
	
	}
	emit propertiesChanged();

	return isOpen();
}

void RtTcpip::close_()
{
	os::auto_lock L(comm_lock);
	RtInterface::close_();
	socket_.close();
}




