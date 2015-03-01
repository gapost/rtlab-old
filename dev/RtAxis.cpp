#include "RtAxis.h"

#include <RtDataChannel.h>

#include "RtEnumHelper.h"
Q_SCRIPT_ENUM(AxisMode,RtAxis)


RtAxis::RtAxis(const QString& name, const QString& desc, RtObject* parent, 
		RtInterface* aifc,  int addr) :
RtDevice(name,desc,parent,aifc,addr), lup_(1), ldown_(0)
{
    outputChannels_.push_back(new RtDataChannel("ch1","position channel",this));
}

RtAxis::~RtAxis(void)
{
}

void RtAxis::registerTypes(QScriptEngine* e)
{
	qScriptRegisterAxisMode(e);	
	RtDevice::registerTypes(e);
}

#define MODBUS0 0x3000

int RtAxis::rdstate_(tAxis &ax)
{
	//WORD addr = MODBUS0; // + id*sz;
    if (ifc && ifc->isOpen()) return ifc->read(MODBUS0, (char*)(&ax), sizeof(tAxis));
    else return 0;
}

void RtAxis::home_()
{
	//WORD addr = MODBUS0; // + id*sz;
	if (ifc && ifc->isOpen())
	{
        tAxis ax_;
		ax_.x = 0;
		ax_.xset = 0;
		ifc->write(MODBUS0 + offsetof(tAxis,x)/2, (char*)(&ax_.x), 2*sizeof(int));
	}
}

// arming
bool RtAxis::arm_()
{
    if (throwIfOffline()) return armed_ = false;
    else return RtJob::arm_();
}
//job run
void RtAxis::run()
{
    tAxis ax;
    if (rdstate_(ax)) outputChannels_[0]->push(ax.x);
}

// getters
int RtAxis::pos()
{
    tAxis ax;
    if (rdstate_(ax)) return ax.x;
    else return 0;
}
int RtAxis::setPos()
{
    tAxis ax;
    if (rdstate_(ax)) return ax.xset;
    else return 0;
}
RtAxis::AxisMode RtAxis::mode()
{
    tAxis ax;
    if (rdstate_(ax)) mode_ = (AxisMode)ax.command;
    return mode_;
}
int RtAxis::limitFlag()
{
    tAxis ax;
    if (rdstate_(ax)) lflag_ = ax.outFlags;
    return lflag_;
}

void RtAxis::setSetPos(int v)
{
	os::auto_lock L(comm_lock);
	if (v< lup_ && v>ldown_ && ifc && ifc->isOpen()) 
	{
		ifc->write(MODBUS0 + offsetof(tAxis,xset)/2, (char*)&v, sizeof(v));

		emit propertiesChanged();
		return;
	}
}

void RtAxis::setLimitUp(int v)
{
	os::auto_lock L(comm_lock);
	lup_ = v;
	emit propertiesChanged();
}
void RtAxis::setLimitDown(int v)
{
	os::auto_lock L(comm_lock);
	ldown_ = v;
	emit propertiesChanged();
}
void RtAxis::setMode(AxisMode v)
{
	os::auto_lock L(comm_lock);
	if (ifc->isOpen()) 
	{
		short md = (short) v;
		ifc->write(MODBUS0 + offsetof(tAxis,command)/2, (char*)(&md), sizeof(md));

		emit propertiesChanged();
		return;
	}
}

