#include "RtInterpChannel.h"
#include "RtVector.h"

#include "RtEnumHelper.h"
Q_SCRIPT_ENUM(InterpolationType, RtInterpolationChannel)

#include <QFile>
#include <QTextStream>

#include <QVector>

static const gsl_interp_type* InterpolationObjects[] = {
	0,
	gsl_interp_linear,
	gsl_interp_polynomial,
	gsl_interp_cspline
};


RtInterpolationChannel::RtInterpolationChannel(const QString& name, RtObject* parent) :
RtDataChannel(name,"Interpolator",parent), 
interpolator_(0), accel_(0), type_(None), xa(0,1), ya(0,1)
{
	accel_ = gsl_interp_accel_alloc ();
}

RtInterpolationChannel::~RtInterpolationChannel(void)
{
	if (interpolator_) gsl_interp_free (interpolator_);
	gsl_interp_accel_free (accel_);
}

void RtInterpolationChannel::registerTypes(QScriptEngine* e)
{
	qScriptRegisterInterpolationType(e);
	RtDataChannel::registerTypes(e);
}

void RtInterpolationChannel::run()
{
	RtDataChannel* in = getInputChannel();
	if (in->dataReady())
	{
		if (interpolator_) 
        {
            double val;
            int ret = gsl_interp_eval_e(interpolator_, xa.begin(), ya.begin(), in->value(), accel_, &val);
            if (ret==0)	push(val);
        }
        else push(in->value());
	}
	RtDataChannel::postRun();
}

void RtInterpolationChannel::setType(InterpolationType t)
{
	if (!throwIfArmed() && type_ != t && (int)t!=-1)
	{
		type_ = t;
		init_();
		emit propertiesChanged();
	}
}

void RtInterpolationChannel::init_()
{
	if (interpolator_) gsl_interp_free (interpolator_);
	interpolator_ = 0;
	int it = (int)type_;
	if (it>0 && xa.begin() && ya.begin()) 
	{
		unsigned int n = xa.size();
		interpolator_ = gsl_interp_alloc(InterpolationObjects[it],n);
		gsl_interp_init(interpolator_, xa.begin(), ya.begin(), n);
	}
}


void RtInterpolationChannel::setTable(const RtVector& x, const RtVector& y)
{
	if (throwIfArmed()) return;

	int n = qMin(x.size(), y.size());
	if (n<2) 
	{
		throwScriptError("Vectors must be at least of length 2.");
		return;
	}

	// check that x is ordered
	double d0 = x[1]-x[0];
	bool ok = true;
	for(int i=2; i<n; ++i)
	{
		if (d0*(x[i]-x[i-1])<=0.0) { ok=false; break; }
	}
	if (!ok)
	{
		throwScriptError("x vector must be ordered.");
		return;
	}

	// copy the data
	xa.setSize(n);
	ya.setSize(n);
	if (d0>0.0)
	{
		double *px = xa.begin();
		double *py = ya.begin();
		for(int i=0; i<n; ++i)
		{
			*px++ = x[i]; *py++ = y[i];
		}
	}
	else
	{
		double *px = xa.begin() + n - 1;
		double *py = ya.begin() + n - 1;
		for(int i=0; i<n; ++i)
		{
			*px-- = x[i]; *py-- = y[i];
		}
	}

	init_();

}

void RtInterpolationChannel::fromTextFile(const QString& fname)
{
	if (throwIfArmed()) return;
	QFile file(fname);
     
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		throwScriptError("File cannot be read.");
		return;
	}

 
	QTextStream in(&file);
	QVector<double> x;
	bool ok = true;
	while (ok && !in.atEnd() && in.status()==QTextStream::Ok) 
	{
		QString str;
		in >> str;
		double d = str.toDouble(&ok);
		if (ok) x.push_back(d);
	}

	int n = x.size();
	if (n<4)
	{
		throwScriptError("File does not contain data.");
		return;
	}
	
	// # of pairs
	if (n & 0x01) n--;
	n>>=1;

	// check if x ordered
	double d = x[2]-x[0];
	ok = true;
	for(int i=2; i<n; ++i)
	{
		int j=2*i;
		if (d*(x[j]-x[j-2])<=0.0) { ok=false; break; }
	}
	if (!ok)
	{
		throwScriptError("x vector must be ordered.");
		return;
	}


	// copy the data
	xa.setSize(n);
	ya.setSize(n);
	if (d>0.0)
	{
		double *px = xa.begin();
		double *py = ya.begin();
		const double *p = x.data();
		for(int i=0; i<n; ++i)
		{
			*px++ = *p++; 
			*py++ = *p++;
		}
	}
	else
	{
		double *px = xa.begin() + n - 1;
		double *py = ya.begin() + n - 1;
		const double *p = x.data();
		for(int i=0; i<n; ++i)
		{
			*px-- = *p++; 
			*py-- = *p++;
		}
	}

	init_();
}
