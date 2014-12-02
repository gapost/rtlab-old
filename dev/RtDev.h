#ifndef _RTACQUISITION_H_
#define _RTACQUISITION_H_

#include "RtObject.h"

class RtInterface;
class RtDevice;
class RtDataChannel;
class RtKeithley2182;
class RtKeithley6220;
class RtPowerSupply;
class RtKepcoDps;
class RtKepcoBop;

class RtAcquisition : public RtObject
{
	Q_OBJECT
public:
	RtAcquisition(const QString& name, RtObject* parent);

public slots:

	void newInterface(const QString& name, const QString& type, uint addr = 0);
	void newDevice(const QString& name, RtInterface* ifc, int addr = 0, const QString& model = QString());
	void newTemperatureController(const QString& name, RtDataChannel* tc);
};

#endif

