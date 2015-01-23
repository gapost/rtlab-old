#include "RtDevice.h"

RtDevice::RtDevice(const QString& name, const QString& desc, RtObject* parent, 
		RtInterface* aifc,  int addr, uint buffsz) : 
RtJob(name,desc,parent), online_(false), addr_(addr),
buff_(buffsz,char(0)), buff_sz_(buffsz),
eos_(0x100), 
ifc(aifc)
{
}
RtDevice::~RtDevice()
{
}
void RtDevice::detach()
{
	RtJob::detach();
	setOnline_(false);
}
// on-off
void RtDevice::setOnline(bool on)
{
	if (throwIfArmed()) return;
    os::auto_lock L(comm_lock);
	if (on!=online_)
	{
		setOnline_(on);
		emit propertiesChanged();
	}
}
bool RtDevice::on()
{
	setOnline(true);
	return online_; 
}
void RtDevice::off()
{
	setOnline(false);
} 
bool RtDevice::setOnline_(bool on)
{
	if (on==online_) return online_;
	if (on && ifc) 
	{
		// set this device to on
		online_ = ifc->open_port(addr_,this);
		if (online_) ifc->clear_port(addr_);
		// switch-on also the sub-devices
		if (online_)
		{
			bool ok = true;
			int k = 0;
			foreach(RtJob* j, subjobs)
			{
				RtDevice* dev = qobject_cast<RtDevice*>(j);
				if (dev)
				{
					ok = dev->setOnline_(true);
					if (!ok) break;
					k++;
				}
			}
			if (!ok) // switch off all
			{
				online_ = false;
				ifc->close_port(addr_);
				foreach(RtJob* j, subjobs)
				{
					RtDevice* dev = qobject_cast<RtDevice*>(j);
					if (dev)
					{
						ok = dev->setOnline_(false);
						k--;
						if (k==0) break;
					}
				}
			}
		}
		return online_;
	}
	else {  
		if (ifc) ifc->close_port(addr_); // ifc should excist
		return online_ = false;
	}
}
void RtDevice::forcedOffline(const QString& reason)
{
    os::auto_lock L(comm_lock);
	if (armed_) disarm_();// forcedDisarm(reason);
	if (online_) 
	{
		setOnline_(false);
		pushError("forced offline", reason);
	}
}
// setters
void RtDevice::setBufferSize(unsigned int sz)
{
	if (throwIfArmed()) return;
    os::auto_lock L(comm_lock);
    buff_.resize(sz);
    buff_sz_ = sz;
	emit propertiesChanged();
}
void RtDevice::setAddress(int a)
{
	if (throwIfOnline()) return;
    os::auto_lock L(comm_lock);
	addr_ = a;
	emit propertiesChanged();
}
void RtDevice::setEot(int e)
{
	if (throwIfArmed()) return;
    os::auto_lock L(comm_lock);
	eot_ = e;
}
void RtDevice::setEos(int e)
{
	if (throwIfArmed()) return;
    os::auto_lock L(comm_lock);
	eos_ = e;
}
void RtDevice::setInterface(RtInterface* i)
{
	if (i==ifc) return;
	if (armed_) disarm_();
	if (online_) setOnline_(false);
	ifc = i;
	emit propertiesChanged();
}
// states / messages
bool RtDevice::throwIfOffline()
{
	bool ret = !online_;
	if (ret) throwScriptError("Not possible when device is offline.");
	return ret;
}
bool RtDevice::throwIfOnline()
{
	if (online_) throwScriptError("Not possible when device is online.");
	return online_;
}
// arming
bool RtDevice::arm_()
{
	if (throwIfOffline()) return armed_ = false;
	else return RtJob::arm_();
}
// io
int RtDevice::write_(const char* msg, int len)
{
    os::auto_lock L(comm_lock);
	int ret = ifc->write(addr_, msg, len, eot_);
	//if (!ret && armed_) forcedDisarm(QString("Write to device %1 failed").arg(fullName()));
	checkError(msg,len);
	return ret;
}
int RtDevice::write_(const char* msg)
{
	return write_(msg, strlen(msg));
}
int RtDevice::write_(const QByteArray& msg)
{
	return write_(msg.constData(),msg.size());
}
bool RtDevice::write_(const QList<QByteArray>& msglist)
{
    os::auto_lock L(comm_lock);
	foreach(const QByteArray& msg, msglist)
	{
		if (msg.size() != write_(msg)) return false;
	}
	return true;
}
int RtDevice::write(const QString& msg)
{
	//if (throwIfArmed()) return 0;
	if (throwIfOffline()) return 0;
	return write_(msg.toLatin1());
}
QByteArray RtDevice::read_()
{
    os::auto_lock L(comm_lock);
    buff_.resize(buff_sz_);
    char* mem = buff_.data();
    int cnt =  ifc->read(addr_,mem,buff_sz_,eos_);
    buff_.resize(cnt);
	//if (!buff_cnt_ && armed_) forcedDisarm(QString("Read from device %1 failed").arg(fullName())); 
    return buff_;
}
QString RtDevice::read() 
{
	//if (throwIfArmed()) return QString();
	if (throwIfOffline()) return QString();
	return QString(read_());
}
QString RtDevice::query(const QString& msg) 
{
	//if (throwIfArmed()) return QString();
	if (throwIfOffline()) return QString();
    os::auto_lock L(comm_lock);
	write_(msg.toLatin1());
	return QString(read_());
}
int RtDevice::statusByte_()
{
	return ifc->readStatusByte(addr_);
}
int RtDevice::statusByte() 
{
	if (throwIfArmed()) return 0;
	if (throwIfOffline()) return 0;
	return statusByte_();
}
void RtDevice::clear()
{
	if (throwIfArmed()) return;
	if (throwIfOffline()) return;
	ifc->clear_port(addr_);
}
