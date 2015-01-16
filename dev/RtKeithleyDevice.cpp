#include "RtKeithleyDevice.h"
#include "RtDataChannel.h"
#include <math.h>

RtKeithleyDevice::RtKeithleyDevice(const QString& name, const QString& desc, RtObject* parent, 
		RtInterface* aifc,  int addr, uint buffsz) :
RtDevice(name,desc,parent,aifc,addr,buffsz), binary_(false), display_(true), delay_(0),//, sampleCount_(1), measUnit_(false)
debug_(false), bufferOrderReverse_(false)
{
}

RtKeithleyDevice::~RtKeithleyDevice(void)
{
}

bool RtKeithleyDevice::setOnline_(bool on)
{
	if (on==online_) return on;
	if (on)
	{
        if (RtDevice::setOnline_(true))
        {
            bool ok = write_("*cls")==4 &&
			write_("*rst")==4 && 
			write_(configString_());
            if (!ok) setOnline_(false);
        }
        return online_;
	}
	else
	{
		write_(configDisplay(true)); 
		//if (inputChannels_.size())
		{
			write("rout:scan:lsel none");
			write("*wai");
		}
		write_("syst:pres");
		write_("*rst");
		return RtDevice::setOnline_(false);
	}
}

bool RtKeithleyDevice::arm_()
{
	JobLocker L(this);
	disarm_();
	RtDevice::arm_();
	if (armed_ && inputChannels_.size()) armed_ =  write_(":init")==5;//write_("read?")==5;
	return armed_;
}
void RtKeithleyDevice::disarm_()
{
	JobLocker L(this);
	//if (armed_ && inputChannels_.size()) takeReading_();
	RtDevice::disarm_();
}
void RtKeithleyDevice::run()
{
	if (inputChannels_.size() && takeReading_()) write_(":init");//write_("read?");
}
// config commands
QByteArray RtKeithleyDevice::configDisplay(bool on)
{
	char msg[64];
	int len = sprintf(msg,"disp:enab %c", on ? '1' : '0');
	return QByteArray(msg,len);
}
QByteArray RtKeithleyDevice::configFormat(bool bin)
{
	static const char* code[] = { "dreal", "ascii" };
	char msg[64];
	int len = sprintf(msg,"form:data %s", bin ? code[0] : code[1]);
	return QByteArray(msg,len);
}
QByteArray RtKeithleyDevice::configSampleCount(int c)
{
	char msg[64];
	int len = sprintf(msg,"samp:coun %d",c);
	return QByteArray(msg,len);
}
QByteArray RtKeithleyDevice::configMeasDelay(double v)
{
	char msg[64];
	int len = sprintf(msg,"trig:del %g", v);
	return QByteArray(msg,len);
}

QString RtKeithleyDevice::configString()
{
	QList<QByteArray> lst = configString_();
	int i=0;
	QString S;
	foreach(const QByteArray& msg, lst)
	{
		if (i) S += '\n';
		i++;
		S += msg;
	}
	return S;
}

void RtKeithleyDevice::checkError(const char* msg, int len)
{
	if (!debug_) return;
	QString S;
	int i = 0;
	debug_ = false;
	while ( statusByte_() && 0x04 ) 
	{
		 write_("syst:err?");
		if (read__())
		{
			QByteArray s(buff_,buff_cnt_);
			if (i) S += ", ";
			S += s;
		}
		i++;
	}
	if (i) pushError(QString("Error after sending %1").arg(QString(QByteArray(msg,len))),S);
	debug_ = true;
}

bool RtKeithleyDevice::takeReading_()
{
	// get measurement from instrument
	bool ret = (write("fetch?")==6) && read__();
	if (!ret) return false;

	if (binary_)
	{
		const double* d = (const double*)(buff_ + 2);
		if (bufferOrderReverse_)
		{
			for(int i= (int)(inputChannels_.size()-1); i>=0; --i)
				inputChannels_[i]->push(*d++);
		}
		else
		{
			for(int i=0; i<(int)(inputChannels_.size()); ++i)
				inputChannels_[i]->push(*d++);
		}
		return true;
	}
	else
	{
		QByteArray bytes = QByteArray(buff_,buff_cnt_).trimmed();
		if (inputChannels_.size()>1)
		{
			QList<QByteArray> S = bytes.split(',');
			if (S.size()==inputChannels_.size())
			{
				bool ok = true;
				int i=0;
				QList<QByteArray>::iterator Si = bufferOrderReverse_ ? S.end() : S.begin();
				while(ok && i<inputChannels_.size())
				{
					if (bufferOrderReverse_) --Si;
					double v = Si->toDouble(&ok);
					inputChannels_[i++]->push(v);
					if (!bufferOrderReverse_) ++Si;
				}
				return ok;
			}
			else return false;
		}
		else
		{
			bool ok;
			double v = bytes.toDouble(&ok);
			if (ok) inputChannels_[0]->push(v);
			return ok;
		}
	}
}
void RtKeithleyDevice::takeReading()
{
	if (throwIfArmed()) return;
	if (throwIfOffline()) return;

	if (write_(":init") && takeReading_())
	{
		for(int i=0; i<inputChannels_.size(); ++i)
			inputChannels_[i]->forceProcces();
	}

}
//setters
void RtKeithleyDevice::setBinaryDataTransfer(bool b)
{
	if (throwIfArmed()) return;
	binary_ = b;
	if (online_) write_(configFormat(b));
	emit propertiesChanged();
}
void RtKeithleyDevice::setDisplay(bool b)
{
	if (throwIfArmed()) return;
	display_ = b;
	if (online_) write_(configDisplay(b));
	emit propertiesChanged();
}
void RtKeithleyDevice::setDelay(double v)
{
	static const double vmin = 0;
	static const double vmax = 1000.;
	if (throwIfArmed()) return;
	else
	{
		if (v<vmin) v=vmin;
		if (v>vmax) v=vmax;
		delay_ = v;
		if (online_) write_(configMeasDelay(v));
		emit propertiesChanged();
	}
}

//******************************************************************************************//

RtKeithley2182::RtKeithley2182(const QString& name, RtObject* parent, 
		RtInterface* aifc,  int addr, uint buffsz) :
RtKeithleyDevice(name,"Keithley 2182 Nanovoltmeter",parent,aifc,addr,buffsz),
range_(0.0), nplc_(1.), autoZero_(false), lineSync_(false), dual_(false), filter_(false)
{
	inputChannels_.push_back(new RtDataChannel("ch1","Channel1",this));
	ch2_ = new RtDataChannel("ch2","Channel2",this);
	bufferOrderReverse_ = true;
}
RtKeithley2182::~RtKeithley2182(void)
{
}
void RtKeithley2182::setRange(double v)
{
	static const double ranges[] = { 0.01, 0.1, 1., 10. }; 

	if (throwIfArmed()) return;
	if (v==0.0) range_ = v;
	else
	{
		int i=0;
		while(i<3 && v>ranges[i]) i++;
		range_ = ranges[i];
	}
	if (online_) write_(configRange(range_));
	emit propertiesChanged();
}
void RtKeithley2182::setNplc(double v)
{
	if (throwIfArmed()) return;
	v = (v>50.) ? 50. : v;
	v = (v<0.01) ? 0.01 : v;
	nplc_ = v;
	if (online_) write_(configNplc(v));
	emit propertiesChanged();
}
void RtKeithley2182::setAutoZero(bool v)
{
	if (throwIfArmed()) return;
	autoZero_ = v;
	if (online_) write_(configAZero(v));
	emit propertiesChanged();
}
void RtKeithley2182::setLineSync(bool v)
{
	if (throwIfArmed()) return;
	lineSync_ = v;
	if (online_) write_(configLSync(v));
	emit propertiesChanged();
}
void RtKeithley2182::setAnalogFilter(bool v)
{
	if (throwIfArmed()) return;
	filter_ = v;
	if (online_) write_(configFilter(v));
	emit propertiesChanged();
}
void RtKeithley2182::setDualChannel(bool v)
{
	if (throwIfArmed()) return;
	dual_ = v;
	if (dual_) 
	{
		if (inputChannels_.size()==1) 
		{
			inputChannels_.push_back(ch2_);
		}
	}
	else 
	{
		if (inputChannels_.size()==2) 
		{
			inputChannels_.pop_back();
		}
	}
	if (online_) write_(configDual(dual_));
	emit propertiesChanged();
}
// config strings
QByteArray RtKeithley2182::configNplc(double v)
{
	char msg[64];
	int len = sprintf(msg,"sens:volt:nplc %g",v);
	return QByteArray(msg,len);
}
QList<QByteArray> RtKeithley2182::configRange(double v)
{
	QList<QByteArray> msglst;
	char msg[64];
	if (v==0.0f)
	{
		msglst << "sens:volt:chan1:rang:auto 1";
		msglst << "sens:volt:chan2:rang:auto 1";
	}
	else
	{
		sprintf(msg,"sens:volt:chan1:rang %g",v);
		msglst << msg;
		sprintf(msg,"sens:volt:chan2:rang %g",v);
		msglst << msg;
	}
	return msglst;
}
QList<QByteArray> RtKeithley2182::configFilter(bool on)
{
	QList<QByteArray> msglst;
	char msg[64];
	sprintf(msg,"sens:volt:chan1:lpas %c", on ? '1' : '0');
	msglst << msg;
	sprintf(msg,"sens:volt:chan2:lpas %c", on ? '1' : '0');
	msglst << msg;
	return msglst;
}
QByteArray RtKeithley2182::configAZero(bool on)
{
	char msg[64];
	int len = sprintf(msg,"syst:faz %c", on ? '1' : '0');
	return QByteArray(msg,len);
}
QByteArray RtKeithley2182::configLSync(bool on)
{
	char msg[64];
	int len = sprintf(msg,"syst:lsyn %c",on ? '1' : '0');
	return QByteArray(msg,len);
}
QList<QByteArray> RtKeithley2182::configDual(bool on)
{
	QList<QByteArray> msglst;
	if (on)
	{
		msglst << "rout:scan:int:cco 1";
		msglst << "rout:scan:lsel int";
	}
	else
	{
		msglst << "rout:scan:lsel none";
	}
	//trigger
	//msglst << "trig:del:auto 1";
	msglst << "trig:sour imm";
	msglst << "trig:coun 1";
	// sample count
	msglst << configSampleCount(on ? 2 : 1);
	// buffer clear
	msglst << "trac:cle";	
	return msglst;
}
QList<QByteArray> RtKeithley2182::configString_()
{
	QList<QByteArray> msglst;

	msglst << configNplc(nplc_);
	// disable dig. filter by default
	msglst << "sens:volt:chan1:dfil:stat 0";
	msglst << "sens:volt:chan2:dfil:stat 0";
	// analog filter
	msglst << configFilter(filter_);
	// front auto-zero
	msglst << configAZero(autoZero_);
	// line-sync
	msglst << configLSync(lineSync_);
	// format 
	msglst << configFormat(binary_);
	// set range
	msglst << configRange(range_);
	//trigger delay
	msglst << configMeasDelay(delay_);
	//scan
	msglst << configDual(dual_);
	// display on/off
	msglst << configDisplay(display_);

	return msglst;
}

//***********************************************************************//
#include "RtEnumHelper.h"
Q_SCRIPT_ENUM(MeasFunction,RtKeithley2000)

RtKeithley2000::RtKeithley2000(const QString& name, RtObject* parent, 
		RtInterface* aifc,  int addr, uint buffsz) :
RtKeithleyDevice(name,"Keithley 2000 DMM",parent,aifc,addr,buffsz),
range_(0.0), nplc_(1.), mfunc(VoltDC), scan_(false), scanChannel_(1)
{
	for (int i=0; i<10; ++i)
	{
		ch_[i] = new RtDataChannel(QString("ch%1").arg(i+1),QString("Channel%1").arg(i+1),this);
	}
	inputChannels_.push_back(ch_[0]);

	scanRange_.push_back(1);
	scanRange_.push_back(10);
}
RtKeithley2000::~RtKeithley2000(void)
{
}
void RtKeithley2000::registerTypes(QScriptEngine* e)
{
	qScriptRegisterMeasFunction(e);	
	RtKeithleyDevice::registerTypes(e);
}
// setters
void RtKeithley2000::setRange(double v)
{
	if (throwIfArmed()) return;
	range_ = v;
	emit propertiesChanged();
	if (online_) write_(configRange(mfunc,v));
}
void RtKeithley2000::setNplc(double v)
{
	if (throwIfArmed()) return;
	v = (v>50.) ? 50. : v;
	v = (v<0.01) ? 0.01 : v;
	nplc_ = v;
	emit propertiesChanged();
	if (online_) write_(configNplc(mfunc,v));
}
void RtKeithley2000::setScanChannel(int i)
{
	if (throwIfArmed()) return;
	i = (i>10) ? 10 : i;
	i = (i< 1) ? 1 : i;
	scanChannel_ = i;
	emit propertiesChanged();
	configScan_();
}
void RtKeithley2000::setFunc(MeasFunction f)
{
	if (throwIfArmed()) return;

	if ((int)f==-1)
	{
		QString msg("Invalid function specification. Availiable functions: ");
		const QMetaObject& mobj = RtKeithley2000::staticMetaObject;
		QMetaEnum menum = mobj.enumerator(mobj.indexOfEnumerator("MeasFunction"));
		if (menum.isValid())
		{
			for(int i=0; i<menum.keyCount(); ++i)
			{
				if (i) msg += ", ";
				msg += menum.key(i);
			}
		}
		throwScriptError(msg);
		return;
	}
	
	mfunc = f;
	emit propertiesChanged();
	if (online_) write_(configFunc(mfunc));
}
void RtKeithley2000::setScan(bool v)
{
	if (throwIfArmed()) return;

	if (scan_==v) return;

	scan_ = v;
	emit propertiesChanged();

	configScan_();
}
void RtKeithley2000::setScanRange(const RtIntVector& v)
{
	if (throwIfArmed()) return;

	if (scanRange_ == v) return;

	if ( v.size()==2 &&
		 v[0]>=1 && v[0]<=10 && 
		 v[1]>=1 && v[1]<=10 && 
		 v[1]>v[0])
	{
		scanRange_ = v;
		emit propertiesChanged();
		if (scan_) configScan_();
	}
	else
	{
		QString msg("Invalid scan range. Availiable channels: 1 - 10.");
		throwScriptError(msg);
		return;
	}

}
void RtKeithley2000::configScan_()
{
	inputChannels_.clear();

	if (scan_) 
	{
		for(int i=scanRange_[0]; i<=scanRange_[1]; ++i)
		{
			inputChannels_.push_back(ch_[i-1]);
		}
	}
	else 
	{
		RtDataChannel* ch = ch_[scanChannel_-1];
		inputChannels_.push_back(ch);
	}

	if (online_) 
	{
		if (scan_) write_(configScanStr(scanRange_[0],scanRange_[1]));
		else write_(configScanStr(scanChannel_,0));
	}

}
// config strings
const char* RtKeithley2000::measFuncName(MeasFunction f)
{
	static const char* func_names[] = {
		"volt:dc",
		"volt:ac",
		"curr:dc",
		"curr:ac",
		"res",
		"fres",
		"per",
		"freq",
		"temp"
	};
	return func_names[(int)f];
}
QByteArray RtKeithley2000::configFunc(MeasFunction f)
{
	char msg[64];
	const char* fname = measFuncName(f);
	int len = sprintf(msg,"func '%s'",fname);
	return QByteArray(msg,len);
}
QByteArray RtKeithley2000::configNplc(MeasFunction f, double v)
{
	char msg[64];
	const char* fname = measFuncName(f);
	int len = sprintf(msg,"%s:nplc %g",fname,v);
	return QByteArray(msg,len);
}
QByteArray RtKeithley2000::configRange(MeasFunction f, double v)
{
	char msg[64];
	const char* fname = measFuncName(f);
	int len = (v==0.0f) ? 
		sprintf(msg,"%s:rang:auto 1",fname) :
		sprintf(msg,"%s:rang %g",fname,v);
	return QByteArray(msg,len);
}
QList<QByteArray> RtKeithley2000::configScanStr(int ch0, int ch1)
{
	bool on = ch1!=0;
	int n = on ? ch1-ch0+1 : 1;

	QList<QByteArray> msglst;
	// sample count
	msglst << configSampleCount(n);
	// scanner
	char msg[64];
	if (on)
	{
		int len = sprintf(msg,"rout:scan (@%d:%d)",ch0,ch1);
		msglst << QByteArray(msg,len);
		msglst << "rout:scan:lsel int";
	}
	else
	{
		msglst << "rout:scan:lsel none";
		int len = sprintf(msg,"rout:clos (@%d)",ch0);
		msglst << QByteArray(msg,len);
	}
	//trigger
	//msglst << "trig:del:auto 1";
	msglst << "trig:sour imm";
	msglst << "trig:coun 1";
	// buffer clear
	msglst << "trac:cle";	
	return msglst;
}
QList<QByteArray> RtKeithley2000::configString_()
{
	QList<QByteArray> msglst;

	// set func
	msglst << configFunc(mfunc);
	// set nplc
	msglst << configNplc(mfunc,nplc_);
	//trigger delay
	msglst << configMeasDelay(delay_);

	// set range
	if (mfunc!=Temperature) msglst << configRange(mfunc,range_);
	else {
		msglst << "temp:tc:type K";
		msglst << "temp:tc:rjun:sim 25";
	}
	
	// format 
	msglst << configFormat(binary_);
	// display on/off
	msglst << configDisplay(display_);

	// scan or not
	if (scan_) msglst << configScanStr(scanRange_[0],scanRange_[1]);
	else msglst << configScanStr(scanChannel_,0);

	return msglst;
}
//********************************************************************************//
RtKeithley6220::RtKeithley6220(const QString& name, RtObject* parent, 
		RtInterface* aifc,  int addr, uint buffsz) :
RtKeithleyDevice(name,"Keithley 6220 DC current source",parent,aifc,addr,buffsz),
curr_(1e-3), range_(0.0), compliance_(10.), out_(false),
sweep_(false), sweepCount_(0), exttrig_(false), iline_(1)
{
	delay_ = 0.001; 
}
RtKeithley6220::~RtKeithley6220(void)
{
}

bool RtKeithley6220::arm_()
{
	JobLocker L(this);
	disarm_();
	RtDevice::arm_();
	if (sweep_)
	{
		write_("sour:swe:arm");
		write_("init");
	}
	return armed_;
}
void RtKeithley6220::disarm_()
{
	JobLocker L(this);
	if (sweep_)
	{
		write_("sour:swe:abor");
	}	
	RtDevice::disarm_();
}

// config strings
QList<QByteArray> RtKeithley6220::configSweep()
{
	QList<QByteArray> msgs;
	int n = sweepList_.size();
	if (n<1) {
		return msgs;
		//sweepList_ << curr_ << -curr_;
	}


	msgs << configCurrent(curr_);
	msgs << "sour:swe:spac list";
	msgs << "sour:swe:rang fix";

	// list of current values
	{
		QByteArray str("sour:list:curr ");
		str += QByteArray::number(sweepList_[0]);
		for(int i=1; i<n; ++i)
		{
			str += ',';
			str += QByteArray::number(sweepList_[i]);
		}
		msgs << str;
	}
	// list of delay values
	{
		QByteArray str("sour:list:del ");
		str += QByteArray::number(delay_);
		for(int i=1; i<n; ++i)
		{
			str += ',';
			str += QByteArray::number(delay_);
		}
		msgs << str;
	}
	// list of compliance values
	{
		QByteArray str("sour:list:comp ");
		str += QByteArray::number(compliance_);
		for(int i=1; i<n; ++i)
		{
			str += ',';
			str += QByteArray::number(compliance_);
		}
		msgs << str;
	}

	if (sweepCount_<1)
		msgs << "sour:swe:coun inf";
	else
		msgs << (QByteArray("sour:swe:coun ") + QByteArray::number(sweepCount_));


	msgs << "sour:swe:cab 1";

	//msgs << "sour:swe:arm";
	
	return msgs;
}
QList<QByteArray> RtKeithley6220::configTrigger()
{
	QList<QByteArray> msgs;
	if (exttrig_)
		msgs << "trig:sour tlin";
	else
		msgs << "trig:sour imm";

	QByteArray str("trig:ilin ");
	str += QByteArray::number(iline_);
	msgs << str;

	return msgs;
}

QByteArray RtKeithley6220::configCurrent(double v)
{
	char msg[64];
	int len = sprintf(msg,"sour:curr %g", v);
	return QByteArray(msg,len);
}
QByteArray RtKeithley6220::configRange(double v)
{
	char msg[64];
	int len = (v==0.0f) ? 
		sprintf(msg,"sour:curr:rang:auto 1") :
		sprintf(msg,"sour:curr:rang %g",v);
	return QByteArray(msg,len);
}
QByteArray RtKeithley6220::configCompliance(double v)
{
	char msg[64];
	int len = sprintf(msg,"sour:curr:comp %g", v);
	return QByteArray(msg,len);
}
QByteArray RtKeithley6220::configOutput(bool v)
{
	char msg[64];
	int len = sprintf(msg,"outp %c", v ? '1' : '0');
	return QByteArray(msg,len);
}
QByteArray RtKeithley6220::configFloating(bool v)
{
	char msg[64];
	// v=true means floating thus 'outp:lte 0'
	int len = sprintf(msg,"outp:lte %c", v ? '0' : '1');
	return QByteArray(msg,len);
}
QByteArray RtKeithley6220::configSourceDelay(double v)
{
	char msg[64];
	int len = sprintf(msg,"sour:del %g", v);
	return QByteArray(msg,len);
}
QList<QByteArray> RtKeithley6220::configString_()
{
	QList<QByteArray> msglst;

	// format 
	msglst << configFormat(binary_);
	// display on/off
	msglst << configDisplay(display_);
	// trigger
	msglst << configTrigger();
	// range
	msglst << configRange(range_);
	// floating source
	msglst << configFloating(float_);

	if (sweep_)
	{
		msglst << configSweep();
	}
	else
	{
		// current
		msglst << configCurrent(curr_);
		msglst << configCompliance(compliance_);
		msglst << configOutput(out_);
	}

	return msglst;
}
// setters
void RtKeithley6220::setCurrent(double v)
{
	static const double imin = -105.0e-3;
	static const double imax = 105.0e-3;
	if (throwIfArmed()) return;
	else
	{
		if (v<imin) v=imin;
		if (v>imax) v=imax;
		curr_ = v;
		if (online_ && !sweep_) write_(configCurrent(v));
		emit propertiesChanged();
	}
}

void RtKeithley6220::setRange(double v)
{
	static const double imin = -105.0e-3;
	static const double imax = 105.0e-3;
	if (throwIfArmed()) return;
	else
	{
		if (v<imin) v=imin;
		if (v>imax) v=imax;
		range_ = fabs(v);
		if (online_) write_(configRange(v));
		emit propertiesChanged();
	}
}

void RtKeithley6220::setCompliance(double v)
{
	static const double cmin = 0.5;
	static const double cmax = 105.0;
	if (throwIfArmed()) return;
	else
	{
		v = fabs(v);
		if (v<cmin) v=cmin;
		if (v>cmax) v=cmax;
		compliance_ = v;
		if (online_) write_(configCompliance(v));
		emit propertiesChanged();
	}
}
void RtKeithley6220::setOutput(bool on)
{
	if (throwIfArmed()) return;
	else
	{
		out_ = on;
		if (online_ && !sweep_) write_(configOutput(on));
		emit propertiesChanged();
	}
}
void RtKeithley6220::setFloating(bool on)
{
	if (throwIfArmed()) return;
	else
	{
		float_ = on;
		if (online_) write_(configFloating(on));
		emit propertiesChanged();
	}
}
void RtKeithley6220::setDelay(double v)
{
	static const double imin = 0.001;
	static const double imax = 1000.;
	if (throwIfArmed()) return;
	else
	{
		if (v<imin) v=imin;
		if (v>imax) v=imax;
		delay_ = v;
		if (online_) write_(configSourceDelay(v));
		emit propertiesChanged();
	}
}
void RtKeithley6220::setSweep(bool on)
{
	if (throwIfArmed()) return;
	else
	{
		sweep_ = on;
		emit propertiesChanged();
	}
}
void RtKeithley6220::setSweepCount(int c)
{
	static const int imin = 0;
	static const int imax = 9999;
	if (throwIfArmed()) return;
	else
	{
		if (c<imin) c=imin;
		if (c>imax) c=imax;
		sweepCount_ = c;
		emit propertiesChanged();
	}
}
void RtKeithley6220::setSweepList(const RtDoubleVector& lst)
{
	if (throwIfArmed()) return;
	if (lst.size() < 1) return;
	else
	{
		sweepList_ = lst;
		emit propertiesChanged();
	}
}
void RtKeithley6220::setExternalTrigger(bool on)
{
	if (throwIfArmed()) return;
	else
	{
		exttrig_ = on;
		if (online_) write_(configTrigger());
		emit propertiesChanged();
	}
}
void RtKeithley6220::setIline(int c)
{
	static const int imin = 1;
	static const int imax = 6;
	if (throwIfArmed()) return;
	else
	{
		if (c<imin) c=imin;
		if (c>imax) c=imax;
		iline_ = c;
		if (online_) write_(configTrigger());
		emit propertiesChanged();
	}
}


