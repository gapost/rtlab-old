#ifndef _RTTIMEVALUE_H_
#define _RTTIMEVALUE_H_

#include <sys/timeb.h>
#include <time.h>

#include <QByteArray>
#include <QString>

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
		__timeb32 t;
		_ftime32(&t); 
		return RtTimeValue(t.time + 0.001*t.millitm);
	}

	void split(int& sec, int& msec)
	{
		sec = (int)v_;
		msec = (int)(1000*(v_-sec));
	}

	QString toString()
	{
		int s,ms;
		split(s,ms);
		return QString("%1.%2")
			.arg(QString(QByteArray(_ctime32((__time32_t*)&s)+11,8)))
			.arg(ms,3,10,QChar('0'));
	}

};

#endif


