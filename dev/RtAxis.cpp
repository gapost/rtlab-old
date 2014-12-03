#include "RtAxis.h"

#include "RtEnumHelper.h"
Q_SCRIPT_ENUM(AxisMode,RtAxis)


RtAxis::RtAxis(const QString& name, const QString& desc, RtObject* parent, 
		RtInterface* aifc,  int addr) :
RtDevice(name,desc,parent,aifc,addr), lup_(1), ldown_(0)
{
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

void RtAxis::rdstate_()
{
	//WORD addr = MODBUS0; // + id*sz;
	if (ifc && ifc->isOpen()) ifc->read(MODBUS0, (char*)(&ax_), sizeof(tAxis));
}

void RtAxis::home_()
{
	//WORD addr = MODBUS0; // + id*sz;
	if (ifc && ifc->isOpen())
	{
		ax_.x = 0;
		ax_.xset = 0;
		ifc->write(MODBUS0 + offsetof(tAxis,x)/2, (char*)(&ax_.x), 2*sizeof(int));
	}
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

