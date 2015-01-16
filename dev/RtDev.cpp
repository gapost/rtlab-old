#include "RtDev.h"
#include "RtInterface.h"

#include "RtDevice.h"
#include "RtKeithleyDevice.h"
#include "RtPowerSupply.h"
#include "RtTemperatureController.h"
#include "RtResistanceController.h"
#include "RtAxis.h"
#include "rtdaq.h"
#include "rtgpib.h"
#include "rtmodbus.h"


RtAcquisition::RtAcquisition(const QString& name, RtObject* parent) :
RtObject(name,"Acquisition devices folder",parent)
{
	canBeKilled_ = false;
}
void RtAcquisition::newInterface(const QString& name, const QString& type, uint addr)
{
	static const char* InvalidTypeMsg = 
		"Invalid interface type specification.\n"
		"Valid types are:\n"
        "  \"TCPIP\", Standard Tcp/Ip communications\n"
        "  \"GPIB\", IEEE488.2 communications\n"
        "  \"MODBUS-TCP\", modbus-tcp protocol comms\n";

	// check name
	if (!checkName(name)) return;

	// check the type
	int idx = -1;
    if (type=="TCPIP") idx=0;
    else if (type=="GPIB") idx=1;
    else if (type=="MODBUS-TCP") idx=2;
    else
	{
		throwScriptError(InvalidTypeMsg);
		return;
	}

	RtInterface* dev = 0;
	switch (idx)
	{
	case 0:
		dev = new RtTcpip(name,this);
        break;
    case 1:
        dev = new RtGpib(name,this,addr);
        break;
    case 2:
        dev = new RtModbus(name,this);
        break;
    }
	createScriptObject(dev);
}

void RtAcquisition::newDevice(const QString& name, RtInterface* ifc, int addr, const QString& model)
{
	static const char* InvalidModelMsg = 
		"Invalid model specification.\n"
		"Valid models are:\n"
		"  \"K2182\", Keithely 2182 nano-voltmeter\n"
		"  \"K2000\", Keithely 2000 digital multi-meter\n"
		"  \"K6220\", Keithely 6220 DC current source\n"
		"  \"TTi\", TTi power supply\n"
		"  \"KepcoBop\", Kepco BOP power supply\n"
		"  \"KepcoDps\", Kepco DPS power supply\n"		
		"  \"ResistanceController\", based on Kepco BOP power supply\n"
        "  \"Axis\", Axis controller\n"
        "  No model given: generic device";


	// check name
	if (!checkName(name)) return;

	// check address
	/*if (!ifc->isValidPort(addr))
	{
		throwScriptError("Address is not supported on this interface.");
		return;
	}*/

	// check the model
	int modelIdx = 0;
	if (!model.isNull())
	{
		if      (model=="K2182") modelIdx=1;
		else if (model=="K2000") modelIdx=2;
		else if (model=="K6220") modelIdx=3;
		else if (model=="TTi") modelIdx=4;
		else if (model=="KepcoBop") modelIdx=5;
		else if (model=="ResistanceController") modelIdx=6;
		else if (model=="KepcoDps") modelIdx=7;
        else if (model=="Axis") modelIdx=8;
        else
		{
			throwScriptError(InvalidModelMsg);
			return;
		}
	}

	RtDevice* dev = 0;
	switch (modelIdx)
	{
	case 0:
		dev = new RtDevice(name,"Generic device",this,ifc,addr);
		break;
	case 1:
		dev = new RtKeithley2182(name,this,ifc,addr);
		break;
	case 2:
		dev = new RtKeithley2000(name,this,ifc,addr);
		break;
	case 3:
		dev = new RtKeithley6220(name,this,ifc,addr);
		break;
	case 4:
		dev = new RtTTiDevice(name,this,ifc,addr);
		break;
	case 5:
		dev = new RtKepcoBop(name,this,ifc,addr);
		break;
	case 6:
		dev = new RtResistanceController(name,this,ifc,addr);
		break;
	case 7:
		dev = new RtKepcoDps(name,this,ifc,addr);
		break;	
    case 8:
        dev = new RtAxis(name,"Axis Controller",this,ifc,addr);
        break;
    }
	if (dev) createScriptObject(dev);
}

void RtAcquisition::newTemperatureController(const QString& name, RtDataChannel* tc)
{
	if (!checkName(name)) return;
	if (tc)
	{
		RtTemperatureController* dev = new RtTemperatureController(name,this,tc);
		createScriptObject(dev);
	}
	else throwScriptError("Invalid temperature channel.");
}

void RtAcquisition::newDAQmxTask(const QString &name)
{
#ifdef _WIN32
    if (!checkName(name)) return;
    RtDAQmxTask* task = new RtDAQmxTask(name,this);
    if (task) createScriptObject(task);
#elif __linux__
    throwScriptError("Not supported under Linux");
#endif

}



