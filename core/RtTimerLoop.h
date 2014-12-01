#ifndef _RTTIMERLOOP_H_
#define _RTTIMERLOOP_H_

#include "rtjob.h"
#include "os_utils.h"
#include "filters.h"

/** A real-time timer loop based on Win32 multimedia timers.
\ingroup RtBase
*/
class RtTimerLoop : public RtLoop
{
	Q_OBJECT

	/** The repetition period in ms.
	*/
	Q_PROPERTY(uint period READ period WRITE setPeriod)

protected:

    typedef os::timer<RtTimerLoop> timer_t;
    //typedef os::timer_thread<RtTimerLoop> timer_t;
	friend class timer_t;
	
	timer_t thread_;
	unsigned int period_;

	virtual bool arm_(); 
	virtual void disarm_();

	void operator()();

	typedef running_average<float,10> perfmon_t;
	perfmon_t perfmon[2];
    os::stopwatch clock_;
	float t_[2];


public:
	RtTimerLoop(const QString& name, RtObject* parent, unsigned int period = 1000);
	virtual ~RtTimerLoop(void);

	unsigned int period() const { return period_; }
	void setPeriod(unsigned int p);

public: // slots:
	Q_SLOT QString stat();
};

#endif


