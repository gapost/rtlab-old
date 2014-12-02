#ifndef _RT_RESISTANCE_CONTROLLER_H_
#define _RT_RESISTANCE_CONTROLLER_H_

#include "RtDevice.h"

#include "isa_pid.h"
#include "relaytuner.h"

class RtResistanceController : public RtDevice
{
	Q_OBJECT
	Q_PROPERTY(double samplingPeriod READ samplingPeriod WRITE setSamplingPeriod)
	Q_PROPERTY(double maxPower READ maxPower WRITE setMaxPower)
	Q_PROPERTY(double offsetCurrent READ offsetCurrent WRITE setOffsetCurrent)
	Q_PROPERTY(double power READ power WRITE setPower)
	Q_PROPERTY(bool autoMode READ autoMode WRITE setAutoMode)
	Q_PROPERTY(double setPoint READ setPoint WRITE setSetPoint)
	Q_PROPERTY(double gain READ gain WRITE setGain)
	Q_PROPERTY(double Ti READ Ti WRITE setTi)
	Q_PROPERTY(double Td READ Td WRITE setTd)
	Q_PROPERTY(double Tr READ Tr WRITE setTr)
	Q_PROPERTY(uint Nd READ Nd WRITE setNd)
	Q_PROPERTY(double beta READ beta WRITE setBeta)
	Q_PROPERTY(bool autoTune READ autoTune WRITE setAutoTune)
	Q_PROPERTY(double relayStep READ relayStep WRITE setRelayStep)
	Q_PROPERTY(double relayOffset READ relayOffset WRITE setRelayOffset)
	Q_PROPERTY(double relayThreshold READ relayThreshold WRITE setRelayThreshold)
	Q_PROPERTY(int relayIterations READ relayIterations WRITE setRelayIterations)
	Q_PROPERTY(double Kc READ Kc)
	Q_PROPERTY(double Tc READ Tc)

protected:
	bool auto_, autotune_;
	double Rs_, W_, R_, I_, Io;

	QPointer<RtDataChannel> vc_;

	virtual bool setOnline_(bool on);
	virtual void run();

	isa_pid<double> pid;
	autotuner<double> tuner;

	bool setCurrent_(double& i);

public:
	RtResistanceController(const QString& name, RtObject* parent, RtInterface* aifc,  int addr);
	virtual ~RtResistanceController(void);

	// getters
	bool autoMode() const { return auto_; }
	double maxPower() const { return pid.get_umax(); }
	double offsetCurrent() const { return Io; }
	double power() const { return W_; }
	double samplingPeriod() const { return pid.get_h(); }
	double setPoint() const { return Rs_; }
	double gain() const { return pid.get_k(); }
	double Ti() const { return pid.get_ti(); }
	double Td() const { return pid.get_td(); }
	double Tr() const { return pid.get_tr(); }
	uint Nd() const { return pid.get_N(); }
	double beta() const { return pid.get_b(); }
	double relayStep() const { return tuner.get_step(); }
	double relayOffset() const { return 0; }
	double relayThreshold() const { return tuner.get_dy(); }
	int relayIterations() const { return tuner.get_count(); }
	double Kc() const { return tuner.get_kc(); }
	double Tc() const { return tuner.get_tc()*pid.get_h(); }
	bool autoTune() const { return autotune_; }

	// setters
	void setAutoMode(bool on);
	void setMaxPower(double v);
	void setOffsetCurrent(double v);
	void setPower(double v);
	void setSamplingPeriod(double v);
	void setSetPoint(double v);
	void setGain(double v);
	void setTi(double v);
	void setTd(double v);
	void setTr(double v);
	void setNd(uint v);
	void setBeta(double v);
	void setRelayStep(double v);
	void setRelayOffset(double v);
	void setRelayThreshold(double v);
	void setRelayIterations(int v);
	void setAutoTune(bool on);

public slots:
	QString report() const;
	void setVoltageChannel(RtDataChannel* atc);
};

#endif


