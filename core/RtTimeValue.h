#ifndef _RTTIMEVALUE_H_
#define _RTTIMEVALUE_H_

#include <cmath>

#include <QByteArray>
#include <QString>
#include <QDateTime>

class RtTimeValue
{
	double v_;

public:

	RtTimeValue() : v_(0.0)
	{}
	explicit RtTimeValue(double d) : v_(d)
	{}
	RtTimeValue(const RtTimeValue& rhs) : v_(rhs.v_)
	{}

	RtTimeValue& operator=(const RtTimeValue& rhs)
	{
		v_ = rhs.v_;
		return *this;
	}

	operator double() { return v_; }

    operator QDateTime()
    {
        return QDateTime::fromMSecsSinceEpoch(1000*v_);
    }

	static RtTimeValue now()
	{
        return RtTimeValue(0.001*(QDateTime::currentMSecsSinceEpoch()));
	}

    QString toString()
	{
        QTime T = QDateTime(*this).time();
        return T.toString("hh:mm:ss.zzz");
	}

};

#endif


