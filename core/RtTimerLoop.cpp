#include "RtTimerLoop.h"

RtTimerLoop::RtTimerLoop(const QString& name, RtObject* parent, unsigned int period) :
	RtLoop(name,"timer loop",parent), period_(period)
{
}

RtTimerLoop::~RtTimerLoop(void)
{
}

bool RtTimerLoop::arm_()
{
	disarm_();
	if (RtLoop::arm_())
	{
		clock_.start();
		t_[0] = (float)clock_.sec();
		armed_ = thread_.start(this,period_); //,THREAD_PRIORITY_TIME_CRITICAL); 
	}
	return armed_;
}

void RtTimerLoop::disarm_()
{
	thread_.stop();
	RtLoop::disarm_();
}
void RtTimerLoop::operator()()
{ 
	t_[1] = (float)clock_.sec(); 
    exec();
	perfmon[0] << (t_[1] - t_[0])*1000; t_[0] = t_[1];
	perfmon[1] << ((float)clock_.sec() - t_[1])*1000;
}
void RtTimerLoop::setPeriod(unsigned int p)
{
	if (p<10) p=10; // minimum 10 ms
	if (period_ != p)
	{
		bool onlineChange = armed_;
		if (onlineChange)
		{
			jobLock();
			disarm_();
		}
		period_ = p;
		if (onlineChange)
		{
			arm_();
			jobUnlock();
		}
		emit propertiesChanged();
	}
}
QString RtTimerLoop::stat()
{
	QString S("Timer statistics:");
	S += QString("\n  Period (ms): %1").arg(perfmon[0]());
	S += QString("\n  Load-time (ms): %2").arg(perfmon[1]());
	return S;
}
