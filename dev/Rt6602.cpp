#include "Rt6602.h"
#include "RtDataChannel.h"

#include "ni6602.h"

Rt6602::Rt6602(const QString& name, RtObject* parent, uint addr) :
RtInterface(name,"Ni 6602 counter pci board",parent,addr)
{
	dev_ = new pci6602::device(addr);

	// 16 empty places: 8 counters + 8 DIO
	ports_.fill(0, 16);
}
Rt6602::~Rt6602(void)
{
	delete dev_;
}
void Rt6602::clear_()
{
	dev_->reset();
}
bool Rt6602::open_()
{
	isOpen_ = (dev_->reset()==0);
	if (!isOpen_)
	{
		pushError("Device Open/Reset failed", dev_->getErrorMessage());
	}
	return isOpen_;
}
void Rt6602::newCounter(const QString& name, int no)
{
	// check name
	if (!checkName(name)) return;

	createScriptObject(new Rt6602Counter(name,this,no));
}
void Rt6602::newPulseGenerator(const QString& name, int no)
{
	// check name
	if (!checkName(name)) return;

	createScriptObject(new Rt6602PulseGenerator(name,this,no));
}
void Rt6602::newDio(const QString& name, int no)
{
	// check name
	if (!checkName(name)) return;

	createScriptObject(new Rt6602Dio(name,this,no));
}
//********************************************************************//
Rt6602CounterBase::Rt6602CounterBase(const QString& name, Rt6602* parent, uint no) :
RtDevice(name,"NI PCI-6602 Counter",parent,parent,no,32), ctr_(0)
{
}
Rt6602CounterBase::~Rt6602CounterBase(void)
{
}
bool Rt6602CounterBase::setOnline_(bool on)
{
	if (on==online_) return online_;
	if (on)
	{
		bool ret = RtDevice::setOnline_(on) && 
			(ctr_ = dev()->getCounter(addr_)) &&
			(ctr_->Reset()==0) &&
			configCtr_();
		if (!ret)
		{
			if (ctr_)
			{
				pushError("Error initializing counter",ctr_->getErrorMessage());
				dev()->releaseCounter(ctr_);
				ctr_ = 0;			
			}
			RtDevice::setOnline_(false);
		}
	}
	else
	{
		RtDevice::setOnline_(false);
		if (ctr_) {
			dev()->releaseCounter(ctr_);
			ctr_ = 0;
		}
	}
	return online_;
}
bool Rt6602CounterBase::arm_()
{
	JobLocker L(this);
	disarm_();
	RtDevice::arm_();
	//count0 = 0;
	armed_ = ctr_ && (ctr_->Prepare()==0) && (ctr_->Arm()==0);
	return armed_;
}
void Rt6602CounterBase::disarm_()
{
	JobLocker L(this);
	if (armed_ && ctr_) ctr_->Disarm();
	RtDevice::disarm_();

}
//********************************************************************//
Rt6602Counter::Rt6602Counter(const QString& name, Rt6602* parent, uint no) :
Rt6602CounterBase(name,parent,no)
{
	inputChannels_.push_back(new RtDataChannel("ch1","Counts",this));
}
Rt6602Counter::~Rt6602Counter(void)
{
}
bool Rt6602Counter::configCtr_()
{
	count0 = 0;
	return ctr_ && (ctr_->ConfEventCount()==0);
}
void Rt6602Counter::takeReading_()
{
	u32 c;
	if (ctr_ && (ctr_->GetCount(c)==0))
	{
		inputChannels_[0]->push(c-count0);
		count0 = c;
	}
}
void Rt6602Counter::run()
{
	takeReading_();
}
void Rt6602Counter::takeReading()
{
	if (throwIfArmed()) return;
	if (throwIfOffline()) return;
	takeReading_();
	inputChannels_[0]->forceProcces();
}
bool Rt6602Counter::filterSource(int fid)
{
    os::auto_lock L(comm_lock);
	bool ret=false;
	if (ctr_ && dev() && fid>=0 && fid<6)
	{
		int sta =  dev()->filterLine(pci6602::SourceID[ctr_->getCounterNo()],(pci6602::LineFilter)fid);
		ret = sta==0;
		if (!ret)
			pushError("Setting source filter",dev()->getErrorMessage());
	}
	return ret;
}
//********************************************************************//
Rt6602PulseGenerator::Rt6602PulseGenerator(const QString& name, Rt6602* parent, uint no) :
Rt6602CounterBase(name,parent,no), tlow_(25), thigh_(25)
{
}
Rt6602PulseGenerator::~Rt6602PulseGenerator(void)
{
}
bool Rt6602PulseGenerator::configCtr_()
{
	/*
	multiply by 20000 to convert ms to the base period of 6602
	20MHz -> 50ns
	thus 1ms -> 20000 periods
	*/
	return ctr_ && (ctr_->ConfSquareGenerator(tlow_*20000,thigh_*20000)==0);
}
void Rt6602PulseGenerator::setTlow(uint t)
{
	if (throwIfArmed()) return;
	if (t<1) t=1;
	if (t>800) t=800;
	tlow_ = t;
	if (online())
	{
		ctr_->Reset();
		configCtr_();
	}
	emit propertiesChanged();
}
void Rt6602PulseGenerator::setThigh(uint t)
{
	if (throwIfArmed()) return;
	if (t<1) t=1;
	if (t>800) t=800;
	thigh_ = t;
	if (online())
	{
		ctr_->Reset();
		configCtr_();
	}
	emit propertiesChanged();
}
//********************************************************************//
Rt6602Dio::Rt6602Dio(const QString& name, Rt6602* parent, uint no) :
RtDevice(name,"NI PCI-6602 Dig I/O line",parent,parent,no,32), dio_(0), input_(true)
{
}
Rt6602Dio::~Rt6602Dio(void)
{
}
bool Rt6602Dio::setOnline_(bool on)
{
	if (on==online_) return online_;
	addr_ += 8;
	if (on)
	{
		bool ret = RtDevice::setOnline_(on) && 
			(dio_ = dev()->getIoPort(addr_ - 8)) &&
			(dio_->setDirection(input_ ? pci6602::digital_io::In : pci6602::digital_io::Out) == 0);
		if (!ret)
		{
			if (dio_)
			{
				pushError("Error initializing DIO",dio_->getErrorMessage());
				dev()->releaseIoPort(dio_);
				dio_ = 0;			
			}
			RtDevice::setOnline_(false);
		}
	}
	else
	{
		RtDevice::setOnline_(false);
		if (dio_) {
			dev()->releaseIoPort(dio_);
			dio_ = 0;
		}
	}
	
	addr_ -= 8;

	return online_;
}
bool Rt6602Dio::state() const
{
	i16 i;
	bool ret;
	if (dio_ && (dio_->getState(i)==0)) ret = i;
	return ret;
}
void Rt6602Dio::setInput(bool v)
{
	input_ = v;
	if (dio_) dio_->setDirection(input_ ? pci6602::digital_io::In : pci6602::digital_io::Out);
	emit propertiesChanged();
}
void Rt6602Dio::setState(bool v)
{
	if (dio_ && dio_->getDirection()==pci6602::digital_io::Out)
		dio_->setState(v);
	emit propertiesChanged();
}
