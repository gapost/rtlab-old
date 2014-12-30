#ifndef _RTTIMEVALUE_H_
#define _RTTIMEVALUE_H_

//#include <sys/timeb.h>
//#include <time.h>
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

	static RtTimeValue now()
	{
        const int epoch = 2440588; // Julian day of 1/1/1970
        const double s_per_day = 24*60*60.0;
        QTime T = QTime::currentTime();
        return RtTimeValue( (QDate::currentDate().toJulianDay() - epoch)*s_per_day +
                T.second()+60*(T.minute()+60*T.hour())+0.001*T.msec() );
	}

	QString toString()
	{
        const int s_per_day = 24*60*60.0;
        double sec = std::floor(v_);
        int ms = (int)((v_-sec)*1000);
        sec = std::fmod(sec,s_per_day);
        QTime T(0,0);
        T = T.addSecs((int)sec);
        T = T.addMSecs(ms);
        return T.toString("hh:mm:ss.zzz");
	}

};

#endif


