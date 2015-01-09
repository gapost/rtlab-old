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








