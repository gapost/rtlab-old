#ifndef _RTINTERPCHANNEL_H_
#define _RTINTERPCHANNEL_H_

#include "RtDataChannel.h"
#include "ivector.h"

#include <gsl/gsl_interp.h>

class RtVector;

class RtInterpolationChannel : public RtDataChannel
{
	Q_OBJECT
	Q_PROPERTY(InterpolationType type READ type WRITE setType)
	Q_ENUMS(InterpolationType)

public:
	enum InterpolationType {
		None,
		Linear,
		Polynomial,
		CubicSpline
	};

protected:
	InterpolationType type_;
	gsl_interp* interpolator_;
	gsl_interp_accel* accel_;

	ibuffer<double> xa,ya;

protected:
	virtual void run();
	void init_();

public:
	RtInterpolationChannel(const QString& name, RtObject* parent);
	virtual ~RtInterpolationChannel(void);

	InterpolationType type() const { return type_; }
	void setType(InterpolationType t);

	virtual void registerTypes(QScriptEngine* e);

public: //slots
	Q_SLOT void setTable(const RtVector& x, const RtVector& y);
	Q_SLOT void fromTextFile(const QString& fname);
};

#endif

