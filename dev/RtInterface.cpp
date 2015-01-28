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
        if (host_.setAddress(h))
            emit propertiesChanged();
        else throwScriptError("Invalid IP address.");
	}
}

int RtTcpip::read(uint port, char* buff, int len, int eos)
{
	os::auto_lock L(comm_lock);

	port=port;
	eos=eos;

    if (!socket_.is_connected()) return 0;

    int rc = socket_.receive(buff,len);
    if (rc==-1){
        pushError("Socket recieve error.",tcp_socket::lastErrorStr());
        return 0;
    }

    return rc;
}

int RtTcpip::write(uint port, const char* buff, int len, int eos)
{
	os::auto_lock L(comm_lock);

	port=port;
	eos=eos;

    if (!socket_.is_connected()) return 0;

    int w = socket_.send(buff,len);
    if (w==-1)
	{
        pushError("Socket send error.",tcp_socket::lastErrorStr());
		return 0;
	}

    return w;
}

bool RtTcpip::open_()
{
    if (isOpen() && socket_.is_connected()) return true;

	os::auto_lock L(comm_lock);

    isOpen_ = socket_.connect(host_.toString().toLatin1().constData(), port_)!=-1;
    if (!isOpen_)
        pushError("Socket connect failed",tcp_socket::lastErrorStr());

	emit propertiesChanged();

	return isOpen();
}

void RtTcpip::close_()
{
	os::auto_lock L(comm_lock);
	RtInterface::close_();
    socket_.disconnect();
}








