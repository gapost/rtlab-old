#include "RtResistanceController.h"
#include "RtDataChannel.h"

#include <math.h>


RtResistanceController::RtResistanceController(const QString& name, RtObject* parent, 
												 RtInterface* aifc,  int addr) :
RtDevice(name,"Temperature Controller",parent,aifc,addr), 
auto_(false), autotune_(false), Rs_(0), W_(0), Io(0.1), I_(0.)
{
	outputChannels_.push_back(new RtDataChannel("W","Power Output",this));
	outputChannels_.push_back(new RtDataChannel("I","Current Output",this));
	outputChannels_.push_back(new RtDataChannel("R","Resistance",this));
}

RtResistanceController::~RtResistanceController(void)
{
}

bool RtResistanceController::setOnline_(bool on)
{
	if (on==online_) return on;
	if (on)
	{
		if (vc_)
		{
			outputChannels_[0]->clear();
			outputChannels_[1]->clear();
			outputChannels_[2]->clear();
			return RtDevice::setOnline_(on);
		}
		else
		{
			throwScriptError("Voltage input is missing.");
			return RtDevice::setOnline_(false);
		}
	}
	else
	{
		auto_ = false;
		autotune_ = false;
		online_ = false;
		W_ = 0;
		outputChannels_[0]->push(0);
		outputChannels_[1]->push(0);
		outputChannels_[2]->push(0);
		I_ = 0.;
		setCurrent_(I_);
		return RtDevice::setOnline_(on);
	}
}

void RtResistanceController::run()
{
	// get T value
	if (!(vc_ && vc_->dataReady())) 
	{
		W_ = I_ = 0;
		outputChannels_[0]->push(W_);
		outputChannels_[1]->push(I_);
		outputChannels_[2]->push(0.);
		return;
	}

	double V = vc_->value();
	double R = I_>0. ? V/I_ : 0.;
	

	// go through autotuner
	//bool disableCtrl;
	bool ret = tuner(Rs_, R, W_, autotune_);

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
	pid(Rs_, R, W_, auto_ && !autotune_);
	I_ = sqrt(W_);
    if (Rs_==0. || I_ < fabs(Io)) I_ = Io;
	setCurrent_(I_);
	W_ = I_*I_;

	// send output power
	outputChannels_[0]->push(W_);
	outputChannels_[1]->push(I_);
	outputChannels_[2]->push(R);
}


// setters
void RtResistanceController::setAutoMode(bool on) 
{
	if (throwIfOffline()) return;
	os::auto_lock L(comm_lock);
	auto_ = on; 
	emit propertiesChanged();
}
void RtResistanceController::setAutoTune(bool on) 
{ 
	if (throwIfOffline()) return;
	os::auto_lock L(comm_lock);
	autotune_ = on; 
	emit propertiesChanged();
}
void RtResistanceController::setMaxPower(double v) 
{ 
	os::auto_lock L(comm_lock);
	pid.set_umax(v); 
	emit propertiesChanged();
}
void RtResistanceController::setOffsetCurrent(double v) 
{ 
	os::auto_lock L(comm_lock);
    Io=v;
	emit propertiesChanged();
}
void RtResistanceController::setPower(double v)
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
void RtResistanceController::setSamplingPeriod(double v) 
{ 
	os::auto_lock L(comm_lock);
	pid.set_h(v); 
	emit propertiesChanged();
}
void RtResistanceController::setSetPoint(double v) 
{ 
	os::auto_lock L(comm_lock);
	Rs_ = v; 
	emit propertiesChanged();
}
void RtResistanceController::setGain(double v) 
{ 
	os::auto_lock L(comm_lock);
	pid.set_k(v); 
	emit propertiesChanged();
}
void RtResistanceController::setTi(double v) 
{ 
	os::auto_lock L(comm_lock);
	pid.set_ti(v); 
	emit propertiesChanged();
}
void RtResistanceController::setTd(double v) 
{ 
	os::auto_lock L(comm_lock);
	pid.set_td(v); 
	emit propertiesChanged();
}
void RtResistanceController::setTr(double v) 
{ 
	os::auto_lock L(comm_lock);
	pid.set_tr(v); 
	emit propertiesChanged();
}
void RtResistanceController::setNd(uint v) 
{ 
	os::auto_lock L(comm_lock);
	pid.set_N(v); 
	emit propertiesChanged();
}
void RtResistanceController::setBeta(double v) 
{ 
	os::auto_lock L(comm_lock);
	pid.set_b(v); 
	emit propertiesChanged();
}
void RtResistanceController::setRelayStep(double v) 
{ 
	os::auto_lock L(comm_lock);
	tuner.set_step(v); 
	emit propertiesChanged();
}
void RtResistanceController::setRelayOffset(double v) 
{ 
	v=v;
}
void RtResistanceController::setRelayThreshold(double v) 
{ 
	os::auto_lock L(comm_lock);
	tuner.set_dy(v); 
	emit propertiesChanged();
}
void RtResistanceController::setRelayIterations(int v) 
{ 
	os::auto_lock L(comm_lock);
	tuner.set_count(v); 
	emit propertiesChanged();
}
void RtResistanceController::setVoltageChannel(RtDataChannel* avc)
{
	if (autoMode() || autoTune()) 
	{
		throwScriptError("Not possible when device is on automatic control.");
		return;
	}
	os::auto_lock L(comm_lock);
	vc_ = avc;
}

QString RtResistanceController::report() const
{
	//QString S("Biased Relay Feedback results:\n");
	QString S = "Input channel : ";
	if (vc_)
	{
		S += vc_->fullName();
		S += ", V = ";
		S += QString::number(vc_->value());
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

bool RtResistanceController::setCurrent_(double& i)
{
	int mode = i>=0 ? 4 : 5;
	i=fabs(i);
	if (i>10.) i=10.;
	int d1 = (int)(i/10.*4095);
	if (d1>4095) d1 = 4095;
	//d1 = n_corrected[d1];
	i = 10.*d1/4095;
	int d2 = 255;

    char mybuff[16];
    int len = sprintf(mybuff,"%1d%03X%02X",mode,d1,d2);
    bool ret =  write_(mybuff,len)==len;
	
	if (mode==5) i *= -1;
	return ret;
}
