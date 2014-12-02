#ifndef _RTPOWERSUPPLY_H_
#define _RTPOWERSUPPLY_H_

#include "RtDevice.h"

class RtPowerSupply : public RtDevice
{
	Q_OBJECT
	Q_PROPERTY(double currentLimit READ currentLimit WRITE setCurrentLimit)
	Q_PROPERTY(double voltageLimit READ voltageLimit WRITE setVoltageLimit)
	Q_PROPERTY(double powerLimit READ powerLimit WRITE setPowerLimit)
	Q_PROPERTY(double output READ output WRITE setOutput)
	Q_PROPERTY(Mode mode READ mode WRITE setMode)
	Q_PROPERTY(double dImax READ dImax WRITE setdImax)
	Q_PROPERTY(double load READ load WRITE setLoad)
	Q_ENUMS(Mode)

public:
	enum Mode { 
		ConstantVoltage = 0, 
		ConstantCurrent = 1,
		ConstantPower = 2
	};

protected:
	double cc_;
	double cv_;
	double cp_;
	double out_;
	Mode mode_;
	double dImax_;
	double load_;


	double out_set_;
	double i_;

	/// program input
	QPointer<RtDataChannel> program_in_;
	/// voltage sense channel
	QPointer<RtDataChannel> vs_;

	virtual bool update_cc_(double v) { cc_ = v; return true; }
	virtual bool update_cv_(double v) { cv_ = v; return true; }
	virtual bool update_cp_(double v) { cp_ = v; return true; }

	virtual void run();
	virtual bool setOnline_(bool on);

public:
	RtPowerSupply(const QString& name, const QString& desc, RtObject* parent, 
		RtInterface* aifc,  int addr);

	virtual void registerTypes(QScriptEngine* e);


	double currentLimit() const { return cc_; }
	bool setCurrentLimit(double v);
	double voltageLimit() const { return cv_; }
	bool setVoltageLimit(double v);
	double powerLimit() const { return cp_; }
	bool setPowerLimit(double v);
	double output() const { return out_; }
	bool setOutput(double v);
	Mode mode() const { return mode_; }
	virtual bool setMode(Mode m);
	double dImax() const { return dImax_; }
	void setdImax(double v) { dImax_ = v; }
	double load() const { return load_; }
	void setLoad(double v) { if (v>0) load_ = v; }


public:
	virtual bool setVoltage_(double& v) = 0;
	virtual bool setCurrent_(double& i) = 0;
	virtual bool setPower_(double& p);
public slots:
	void setVoltageSense(RtDataChannel* vs=0);
	void setProgramingInput(RtDataChannel* in=0);
};

class RtTTiDevice : public RtPowerSupply
{
	Q_OBJECT
	Q_PROPERTY(int outputNumber READ outputNumber WRITE setOutputNumber)
protected:
	int outno_;

	static RtInterface* cpx_otp_ifc_; // contains interface address for a CPX400 that did OTP error (for detecting in the other channel)

	virtual void run();
public:
	RtTTiDevice(const QString& name, RtObject* parent, 
		RtInterface* aifc,  int addr);

	// getters
	int outputNumber() const { return outno_; }
	// setters
	void setOutputNumber(int i);

protected:
	virtual bool update_cc_(double v) { cc_ = v; return true; }
	virtual bool update_cv_(double v) { cv_ = v; return true; }
public:
	virtual bool setVoltage_(double& v);
	virtual bool setCurrent_(double& v);
};

class RtKepcoBop : public RtPowerSupply
{
	Q_OBJECT
public:
	RtKepcoBop(const QString& name, RtObject* parent, 
		RtInterface* aifc,  int addr);
public:
	virtual bool setVoltage_(double& v);
	virtual bool setCurrent_(double& v);
};

class RtKepcoDps : public RtPowerSupply
{
	Q_OBJECT
	Q_PROPERTY(uint koibAddress READ koibAddress WRITE setKoibAddress)
	Q_PROPERTY(bool outputEnable READ outputEnable WRITE setOutputEnable)

protected:
	uint koibAddress_;
	bool output_enable_;

	bool send_comm(const char* str, int len);

public:
	RtKepcoDps(const QString& name, RtObject* parent, 
		RtInterface* aifc,  int addr);

	uint koibAddress() const { return koibAddress_; }
	bool outputEnable() const { return output_enable_; }

	void setKoibAddress(uint v);
	bool setOutputEnable(bool on);

	bool setOutputEnable_(bool on);

	virtual bool setVoltage_(double& v);
	virtual bool setCurrent_(double& i) { i=0; return false; }
protected:
	virtual bool setOnline_(bool on);
	virtual bool update_cc_(double v);
public:
	Q_SLOT float readCurrent();
	Q_SLOT float readVoltage();
};


#endif


