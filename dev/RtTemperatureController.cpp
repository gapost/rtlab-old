#include "RtTemperatureController.h"
#include "RtDataChannel.h"

#include <math.h>



RtTemperatureController::RtTemperatureController(const QString& name, RtObject* parent, 
												 RtDataChannel* atc) :
RtDevice(name,"Temperature Controller",parent,0,0,0), tc(atc), 
auto_(false), autotune_(false), Ts_(0), W_(0)
{
	outputChannels_.push_back(new RtDataChannel("W","Power Output",this));
}

RtTemperatureController::~RtTemperatureController(void)
{
}

bool RtTemperatureController::setOnline_(bool on)
{
	if (on==online_) return on;
	if (on)
	{
		if (tc)
		{
			outputChannels_[0]->clear();
			return online_  = true;
		}
		else
		{
			throwScriptError("Temperature input is missing.");
			return false;
		}
	}
	else
	{
		auto_ = false;
		autotune_ = false;
		online_ = false;
		W_ = 0;
		outputChannels_[0]->push(0);
		return true;
	}
}

void RtTemperatureController::run()
{
	// get T value
	if (!(tc && tc->dataReady())) 
	{
		W_ = 0;
		outputChannels_[0]->push(W_);
		return;
	}

	double T = tc->value();

	// go through autotuner
	//bool disableCtrl;
	bool ret = tuner(Ts_, T, W_, autotune_);

	if (ret) emit propertiesChanged();
/*	{
		double k, ti(0.), td(0.), tr(0.);
		if (Td()>0.0)
		{
			tuner.get_par(pid.get_h(),k,ti,td);
			tr = sqrt(ti*td);
		}
		else if (Ti()>0.0)
		{
			tuner.get_par(pid.get_h(),k,ti);
			tr=ti;
		}
		else
		{
			tuner.get_par(pid.get_h(),k);
		}
		pid.set_k(k);
		pid.set_ti(ti);
		pid.set_td(td);
		pid.set_tr(tr);
	}*/

	// go through pid
	pid(Ts_, T, W_, auto_ && !autotune_);

	// send output power
	outputChannels_[0]->push(W_);
}


// setters
void RtTemperatureController::setAutoMode(bool on) 
{
	if (throwIfOffline()) return;
	os::auto_lock L(comm_lock);
	auto_ = on; 
	emit propertiesChanged();
}
void RtTemperatureController::setAutoTune(bool on) 
{ 
	if (throwIfOffline()) return;
	os::auto_lock L(comm_lock);
	autotune_ = on; 
	emit propertiesChanged();
}
void RtTemperatureController::setMaxPower(double v) 
{ 
	os::auto_lock L(comm_lock);
	pid.set_umax(v); 
	emit propertiesChanged();
}
void RtTemperatureController::setPower(double v)
{
	if (throwIfOffline()) return;
	if (!auto_)
	{
		os::auto_lock L(comm_lock);
		W_ = v;
		outputChannels_[0]->push(W_);
		emit propertiesChanged();
	}
}
void RtTemperatureController::setSamplingPeriod(double v) 
{ 
	os::auto_lock L(comm_lock);
	pid.set_h(v); 
	emit propertiesChanged();
}
void RtTemperatureController::setSetPoint(double v) 
{ 
	os::auto_lock L(comm_lock);
	Ts_ = v; 
	emit propertiesChanged();
}
void RtTemperatureController::setGain(double v) 
{ 
	os::auto_lock L(comm_lock);
	pid.set_k(v); 
	emit propertiesChanged();
}
void RtTemperatureController::setTi(double v) 
{ 
	os::auto_lock L(comm_lock);
	pid.set_ti(v); 
	emit propertiesChanged();
}
void RtTemperatureController::setTd(double v) 
{ 
	os::auto_lock L(comm_lock);
	pid.set_td(v); 
	emit propertiesChanged();
}
void RtTemperatureController::setTr(double v) 
{ 
	os::auto_lock L(comm_lock);
	pid.set_tr(v); 
	emit propertiesChanged();
}
void RtTemperatureController::setNd(uint v) 
{ 
	os::auto_lock L(comm_lock);
	pid.set_N(v); 
	emit propertiesChanged();
}
void RtTemperatureController::setBeta(double v) 
{ 
	os::auto_lock L(comm_lock);
	pid.set_b(v); 
	emit propertiesChanged();
}
void RtTemperatureController::setRelayStep(double v) 
{ 
	os::auto_lock L(comm_lock);
	tuner.set_step(v); 
	emit propertiesChanged();
}
void RtTemperatureController::setRelayOffset(double v) 
{ 
	v=v;
}
void RtTemperatureController::setRelayThreshold(double v) 
{ 
	os::auto_lock L(comm_lock);
	tuner.set_dy(v); 
	emit propertiesChanged();
}
void RtTemperatureController::setRelayIterations(int v) 
{ 
	os::auto_lock L(comm_lock);
	tuner.set_count(v); 
	emit propertiesChanged();
}
void RtTemperatureController::setTemperatureChannel(RtDataChannel* atc)
{
	if (autoMode() || autoTune()) 
	{
		throwScriptError("Not possible when device is on automatic control.");
		return;
	}
	os::auto_lock L(comm_lock);
	tc = atc;
}

QString RtTemperatureController::autoTunerReport() const
{
	//QString S("Biased Relay Feedback results:\n");
	QString S = "Input channel : ";
	if (tc)
	{
		S += tc->fullName();
		S += ", T = ";
		S += QString::number(tc->value());
	}
	else S += "Undefined";

	/*double mo = tuner.step;
	double m = tuner.offset;
	double e = tuner.hyst;

	double Tu1 = tuner.Tup*pid.get_h();
	double Tu2 = tuner.Tdown*pid.get_h();
	double Au = tuner.Aup;
	double Ad = tuner.Adown;
	double K = fabs(tuner.ysum/tuner.usum);
	double theta1 = log(
		((mo+m)*K-e) / ((mo+m)*K-Au)
		);
	double theta2 = log(
		((mo-m)*K-e) / ((mo-m)*K+Ad)
		);
	double theta = (theta1+theta2)/2;
	double T1 = Tu1/log(
		(2*m*K*exp(theta)+mo*K-m*K+e) / (m*K+mo*K-e)
		);
	double T2 = Tu2/log(
		(2*m*K*exp(theta)-mo*K-m*K+e) / (m*K-mo*K-e)
		);


	S += QString("  step  = %1 W\n").arg(mo);
	S += QString("  offset  = %1 W\n").arg(m);
	S += QString("  hyst.  = %1 K\n").arg(e);
	S += QString("  A+    = %1 K\n").arg(Au);
	S += QString("  A-    = %1 K\n").arg(Ad);
	S += QString("  Tu1   = %1 s\n").arg(Tu1);
	S += QString("  Tu2 = %1 s\n").arg(Tu2);
	S += QString("  ysum  = %1 K-samples\n").arg(tuner.ysum);
	S += QString("  usum  = %1 W-samples\n").arg(tuner.usum);
	S += QString("  Keff  = %1 K/W\n").arg(K);
	S += QString("  theta1  = %1 \n").arg(theta1);
	S += QString("  theta2  = %1 \n").arg(theta2);
	S += QString("  theta  = %1 \n").arg(theta);
	S += QString("  T1  = %1 \n").arg(T1);
	S += QString("  T2  = %1 \n").arg(T2);
	S += QString("  L  = %1 or %2 s\n").arg(T1*theta).arg(T2*theta);*/

	return S;
}
