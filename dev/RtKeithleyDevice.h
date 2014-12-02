#ifndef _RTKEITHLEYDEVICE_H_
#define _RTKEITHLEYDEVICE_H_

#include <QVariant>
#include "RtTypes.h"
#include "RtDevice.h"

class RtKeithleyDevice : public RtDevice
{
	Q_OBJECT
	Q_PROPERTY(bool binaryDataTransfer READ binaryDataTransfer WRITE setBinaryDataTransfer)
	Q_PROPERTY(bool display READ display WRITE setDisplay)
	Q_PROPERTY(double delay READ delay WRITE setDelay)

protected:
	// on-off implementation
	virtual bool setOnline_(bool on);

	// job implementation
	virtual bool arm_();
	virtual void disarm_();
	virtual void run();

protected:
	//bool measUnit_;
	bool binary_, display_;
	double delay_;
	//int sampleCount_;
	bool debug_;
	bool bufferOrderReverse_;

	static QByteArray configDisplay(bool on);
	static QByteArray configFormat(bool bin);
	static QByteArray configSampleCount(int c);
	static QByteArray configMeasDelay(double d);

	virtual QList<QByteArray> configString_() = 0;

public:
	RtKeithleyDevice(const QString& name, const QString& desc, RtObject* parent, 
		RtInterface* aifc,  int addr, uint buffsz = 4096);
	virtual ~RtKeithleyDevice(void);

	bool binaryDataTransfer() const { return binary_; }
	bool display() const { return display_; }
	double delay() const { return delay_; }

	void setBinaryDataTransfer(bool b);
	void setDisplay(bool b);
	virtual void setDelay(double v);

	bool takeReading_();

	virtual void checkError(const char* msg, int len);


public:
	Q_SLOT QString configString();
	Q_SLOT void takeReading();
};

class RtKeithley2182 : public RtKeithleyDevice
{
	Q_OBJECT
	Q_PROPERTY(double range READ range WRITE setRange)
	Q_PROPERTY(double nplc READ nplc WRITE setNplc)
	Q_PROPERTY(bool autoZero READ autoZero WRITE setAutoZero)
	Q_PROPERTY(bool lineSync READ lineSync WRITE setLineSync)
	Q_PROPERTY(bool analogFilter READ analogFilter WRITE setAnalogFilter)
	Q_PROPERTY(bool dualChannel READ dualChannel WRITE setDualChannel)

protected:
	double range_, nplc_;
	bool autoZero_, lineSync_, filter_;
	bool dual_;

	RtDataChannel* ch2_;

	virtual QList<QByteArray> configString_();

	static QByteArray configNplc(double v);
	static QList<QByteArray> configRange(double v);
	static QByteArray configAZero(bool v);
	static QByteArray configLSync(bool v);
	static QList<QByteArray> configFilter(bool v);
	static QList<QByteArray> configDual(bool v);

public:
	RtKeithley2182(const QString& name, RtObject* parent, 
		RtInterface* aifc,  int addr, uint buffsz = 4096);
	virtual ~RtKeithley2182(void);

	// getters
	double range() const { return range_; }
	double nplc() const { return nplc_; }
	bool autoZero() const { return autoZero_; }
	bool lineSync() const { return lineSync_; }
	bool analogFilter() const { return filter_; }
	bool dualChannel() const { return dual_; }

	// setters
	void setRange(double v);
	void setNplc(double v);
	void setAutoZero(bool v);
	void setLineSync(bool v);
	void setAnalogFilter(bool v);
	void setDualChannel(bool v);
};

class RtKeithley2000 : public RtKeithleyDevice
{
	Q_OBJECT
	Q_PROPERTY(double range READ range WRITE setRange)
	Q_PROPERTY(double nplc READ nplc WRITE setNplc)
	Q_PROPERTY(MeasFunction func READ func WRITE setFunc)
	Q_PROPERTY(bool scan READ scan WRITE setScan)
	Q_PROPERTY(int scanChannel READ scanChannel WRITE setScanChannel)
	Q_PROPERTY(RtIntVector scanRange READ scanRange WRITE setScanRange)
	Q_ENUMS(MeasFunction)

public:
	enum MeasFunction { 
		VoltDC, 
		VoltAC, 
		CurrentDC, 
		CurrentAC, 
		Resistance2Wire, 
		Resistance4Wire,
		Period,
		Frequency,
		Temperature
	};

protected:
	double range_, nplc_;
	RtDataChannel* ch_[10];
	bool scan_;
	int scanChannel_;
	RtIntVector scanRange_;
	MeasFunction mfunc;

	virtual QList<QByteArray> configString_();

	static const char* measFuncName(MeasFunction f);
	static QByteArray configFunc(MeasFunction f);
	static QByteArray configNplc(MeasFunction f, double v);
	static QByteArray configRange(MeasFunction f, double v);
	static QList<QByteArray> configScanStr(int ch0, int ch1);

	void configScan_();

public:
	RtKeithley2000(const QString& name, RtObject* parent, 
		RtInterface* aifc,  int addr, uint buffsz = 4096);
	virtual ~RtKeithley2000(void);

	virtual void registerTypes(QScriptEngine* e);

	// getters
	double range() const { return range_; }
	double nplc() const { return nplc_; }
	MeasFunction func() const { return mfunc; }
	bool scan() const { return scan_; }
	int scanChannel() const { return scanChannel_; }
	const RtIntVector& scanRange() const { return scanRange_; }

	// setters
	void setRange(double v);
	void setNplc(double v);
	void setFunc(MeasFunction f);
	void setScan(bool v);
	void setScanChannel(int i);
	void setScanRange(const RtIntVector& v);
};

class RtKeithley6220 : public RtKeithleyDevice
{
	Q_OBJECT
	Q_PROPERTY(double current READ current WRITE setCurrent)
	Q_PROPERTY(bool output READ output WRITE setOutput)
	Q_PROPERTY(double range READ range WRITE setRange)
	Q_PROPERTY(double compliance READ compliance WRITE setCompliance)
	Q_PROPERTY(bool floating READ floating WRITE setFloating)
	Q_PROPERTY(bool sweep READ sweep WRITE setSweep)
	Q_PROPERTY(int sweepCount READ sweepCount WRITE setSweepCount)
	Q_PROPERTY(RtDoubleVector sweepList READ sweepList WRITE setSweepList)
	Q_PROPERTY(bool externalTrigger READ externalTrigger WRITE setExternalTrigger)
	Q_PROPERTY(int iline READ iline WRITE setIline)

protected:
	double curr_, range_, compliance_;
	bool out_, float_, sweep_, exttrig_;
	int sweepCount_, iline_;
	RtDoubleVector sweepList_;

	virtual QList<QByteArray> configString_();

	static QByteArray configCurrent(double v);
	static QByteArray configOutput(bool v);
	static QByteArray configRange(double v);
	static QByteArray configCompliance(double v);
	static QByteArray configFloating(bool v);
	static QByteArray configSourceDelay(double v);
	QList<QByteArray> configSweep();
	QList<QByteArray> configTrigger();

public:
	RtKeithley6220(const QString& name, RtObject* parent, 
		RtInterface* aifc,  int addr, uint buffsz = 4096);
	virtual ~RtKeithley6220(void);

	// job implementation
	virtual bool arm_();
	virtual void disarm_();


	// getters
	double current() const { return curr_; }
	bool output() const { return out_; }
	double range() const { return range_; }
	double compliance() const { return compliance_; }
	bool floating() const { return float_; }
	bool sweep() const { return sweep_; }
	int sweepCount() const { return sweepCount_; }
	RtDoubleVector sweepList() const { return sweepList_; }
	bool externalTrigger() const { return exttrig_; }
	int iline() const { return iline_; }

	// setters
	void setCurrent(double v);
	void setOutput(bool on);
	void setRange(double v);
	void setCompliance(double v);
	void setFloating(bool on);
	virtual void setDelay(double d);
	void setSweep(bool on);
	void setSweepCount(int c);
	void setSweepList(const RtDoubleVector& lst);
	void setExternalTrigger(bool on);
	void setIline(int i);
};

#endif


