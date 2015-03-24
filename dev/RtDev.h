#ifndef _RTACQUISITION_H_
#define _RTACQUISITION_H_

#include "RtObject.h"

class RtInterface;
class RtDataChannel;

class RtAcquisition : public RtObject
{
	Q_OBJECT
public:
	RtAcquisition(const QString& name, RtObject* parent);

public slots:

    RtObject* newInterface(const QString& name, const QString& type, uint addr = 0);
    RtObject* newDevice(const QString& name, RtInterface* ifc, int addr = 0, const QString& model = QString());
    RtObject* newTemperatureController(const QString& name, RtDataChannel* tc);

#if defined(_WIN32) && defined(USE_NIDAQMX)
    RtObject* newDAQmxTask(const QString& name);
#endif

#ifdef USE_COMEDI
    RtObject* newNI6221(const QString& name, const QString& devName);
#endif

};

#endif

