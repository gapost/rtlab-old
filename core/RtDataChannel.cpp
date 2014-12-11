#include "RtDataChannel.h"
#include "RtRoot.h"
#include "RtMainWindow.h"
#include "RtChannelWidget.h"
#include "RtTimeValue.h"
#include "RtInterpChannel.h"
#include <QScriptContext>

#include <math.h>
#include <cfloat>

#include <muParser.h>

#include "RtEnumHelper.h"
Q_SCRIPT_ENUM(AveragingType,RtDataChannel)

RtDataChannel::RtDataChannel(const QString& name, const QString& desc, 
							 RtObject* parent) :
RtJob(name, desc, parent), av(1), type_(None), time_channel_(false),
v_(0.), dv_(0.), 
offset_(0.), multiplier_(1.),
parser_(0),
dataReady_(false)
{
	range_ << -1e30 << 1.e30;
}

RtDataChannel::~RtDataChannel(void)
{
	if (parser_) delete parser_;
}

void RtDataChannel::detach()
{
	hide();
	RtJob::detach();
}

void RtDataChannel::registerTypes(QScriptEngine* e)
{
	qScriptRegisterAveragingType(e);
	RtJob::registerTypes(e);
}

void RtDataChannel::setSignalName(QString v)
{
	signalName_ = v;
	emit propertiesChanged();
}
void RtDataChannel::setUnit(QString v)
{
	unit_ = v;
	emit propertiesChanged();
}
void RtDataChannel::setRange(const RtDoubleVector& v)
{
	if((v!=range_) && (!v.isEmpty()) && (v.size()==2) && (v[1]>v[0]))
	{
		os::auto_lock L(comm_lock);
		range_ = v;
		emit propertiesChanged();
	}
}

void RtDataChannel::setOffset(double v)
{
	os::auto_lock L(comm_lock);
	offset_ = v;
}
void RtDataChannel::setMultiplier(double v)
{
	os::auto_lock L(comm_lock);
	multiplier_ = v;
}
void RtDataChannel::setAveraging(AveragingType t)
{
	//if (throwIfArmed()) return;
	if ((int)t==-1)
	{
		QString msg(
			"Invalid averaging specification. Availiable options: "
			"None, Running, Delta"
			);
		throwScriptError(msg);
		return;
	}
	if (type_ != t)
	{
		{
			os::auto_lock L(comm_lock);
			type_ = t;
		}
		emit propertiesChanged();
	}
}

void RtDataChannel::setDepth(uint d)
{
	if ((d!=depth()) && d>0)
	{
		{
			os::auto_lock L(comm_lock);
			av.setN(d);
		}
		emit propertiesChanged();
	}
}

bool RtDataChannel::arm_()
{
	av.clear();
	dataReady_ = false;
	return RtJob::arm_();
}

void RtDataChannel::postRun()
{
	if (dataReady_ = av.mean(averager<double>::type(type_),v_,dv_))
	{
		if (parser_)
		{
			try
			{
			  v_ = parser_->Eval();
			}
			catch (mu::Parser::exception_type &e)
			{
			  //std::cout << e.GetMsg() << endl;
			  dataReady_ = false;
			}
		}
		
		{
			v_ = multiplier_*v_ + offset_;
			dv_ = multiplier_*dv_ + offset_;
		}
		// check limits
                dataReady_ = dataReady_ && finite(v_) && (v_>range_[0]) && (v_<range_[1]);
		updateWidgets();
	}
	RtJob::run();
}

void RtDataChannel::run()
{
	RtDataChannel* in = getInputChannel();
	if (in && in->dataReady()) push(in->value());

	postRun();
}

void RtDataChannel::forceProcces()
{
	forceExec();
} 

QString RtDataChannel::formatedValue()
{
	return dataReady_ ? 
		(time_channel_ ? RtTimeValue(v_).toString() : QString::number(v_)) : QString();
}

void RtDataChannel::clear()
{ 
	os::auto_lock L(comm_lock); 
	av.clear();
	dataReady_ = false;
}

void RtDataChannel::show()
{
	hide();
	widget_ = root_.mainWindow()->channelViewer()->addChannel(this);
}
void RtDataChannel::hide()
{
	if (widget_) delete widget_;
	widget_ = 0;
}
void RtDataChannel::setInputChannel(RtDataChannel* ch)
{
	inputChannel_ = ch;
}
RtDataChannel* RtDataChannel::getInputChannel()
{
	if (inputChannel_) return inputChannel_;
	else if (qobject_cast<RtDataChannel*>(parent())) return (RtDataChannel*)(parent());
	else return 0;
}
QString RtDataChannel::parserExpression() const
{
	if (parser_) return QString(parser_->GetExpr().c_str());
	else return QString();
}
void RtDataChannel::setParserExpression(const QString& s)
{
	if (s!=parserExpression())
	{
		os::auto_lock L(comm_lock);

		if (s.isEmpty())
		{
			if (parser_) delete parser_;
			parser_ = 0;
		}
		else
		{
			if (!parser_) 
			{
				parser_= new mu::Parser();
				parser_->DefineVar("x",&v_);
			}
			parser_->SetExpr(s.toStdString());
		}

		emit propertiesChanged();
	}
}
//////////////////////////////////////////////////////////////////////////////
RtTimeChannel::RtTimeChannel(const QString& name, RtObject* parent) :
	RtDataChannel(name, "Time Channel", parent)
{
	time_channel_ = true;
	dv_ = 0.001;
}

RtTimeChannel::~RtTimeChannel(void)
{
}

void RtTimeChannel::run()
{
	// disabling averaging etc.
	v_ = RtTimeValue::now(); 
	// time data is always availiable
	dataReady_ = true;
	updateWidgets();
	RtJob::run();
}

////////////////////////////////////////////////////////////////////////////////////
RtTestChannel::RtTestChannel(const QString& name, RtObject* parent) :
	RtDataChannel(name, "test channel", parent), v(0), type_(Random)
{
}

RtTestChannel::~RtTestChannel(void)
{
}

void RtTestChannel::run()
{
	static int i;

	switch (type_)
	{
	case Random:
		v = 1.*rand()/RAND_MAX;
		break;
	case Inc:
		v += 1.;
		break;
	case Dec:
		v -= 1.;
		break;
	case Sin:
		v = sin(6.283185307*i++/256);
		break;
	case Pulse:
		v = (v<0.5) ? 1. : 0.;
	}

	push(v);
	RtDataChannel::postRun();
}

void RtTestChannel::setTestType(const QString& type)
{
	jobLock();
	if (type=="Random") type_= Random;
	else if (type=="Inc") type_= Inc;
	else if (type=="Dec") type_ = Dec;
	else if (type=="Sin") type_ = Sin;
	else if (type=="Pulse") type_ = Pulse;
	else throwScriptError("Unknown type.");
	jobUnlock();
}




