#ifndef _RTJOB_H_
#define _RTJOB_H_

#include "rtobject.h"

#include <QPointer>



class RtLoop;

/** Objects that perform a specific task reqursively.

Jobs belong to a RtLoop, which encapsulates a software loop
and executes all jobs that belong to it at each repetition. 
To assign a job to a loop call RtLoop::commit . 

The job must also be armed by calling RtJob::setArmed from C++ or
by setting the armed property to true in script code.
Arming does all the necessary initialization in the object.

\ingroup RtBase
*/
class RtJob : public RtObject
{
	Q_OBJECT
	/** True if job is armed.
	An armed job is ready to run. If the RtJob belongs to a loop
	it will execute its task.
	Setting the property true will arm the job and all child-jobs.
	*/
	Q_PROPERTY(bool armed READ armed WRITE setArmed)

protected:
	bool armed_;

	/** Performs internal initialization for the job.
	*/
	virtual bool arm_() { armed_ = true; return armed_; }
	/// Disarms the job and performs internal de-initialization.
	virtual void disarm_() { armed_ = false;}

public:
	bool armed() { return armed_; }
	bool setArmed(bool on);

protected:

	friend class JobList;
	class JobList : public QList<RtJob*>
	{
		public:
			void exec() { foreach(RtJob* job, *this) job->exec();  } 
			void forceExec() { foreach(RtJob* job, *this) job->forceExec();  } 
			void lock();  
			void unlock();
	};

	friend class JobLocker;
	class JobLocker
	{
		RtJob* j;
	public:
		JobLocker(RtJob* aj) : j(aj)
		{
			j->jobLock();
		}
		~JobLocker()
		{
			j->jobUnlock();
		}
	};

	JobList subjobs;

	void jobLock();
	void jobUnlock();

protected:
	void addChild(RtJob* j);
	void removeChild(RtJob* j);

protected:
	virtual void exec();
	virtual void forceExec();
	virtual void run() {  }

	QPointer<RtLoop> loop_;

	bool throwIfArmed();

public:
	/** Constructor of RtJob objects.
	If parent is also a RtJob then the currently created RtJob is added to the
	list of this parent's child-jobs.
	*/
	RtJob(const QString& name, const QString& desc, RtObject* parent);
	virtual ~RtJob(void);

	virtual void detach();

	void setLoop(RtLoop* l) { loop_ = l; }

	static RtJob* newJobImpl(RtObject* parent, QScriptContext* ctx, 
		const QString& name, const QString& classname);

public slots:
	/// Arm the job & child-jobs.
	bool arm() { return setArmed(true); }
	/// Disarm the job & child-jobs.
	void disarm() { setArmed(false); }
	/// Create a new job of specified class
	void newJob(const QString& name, const QString& classname);
};

/** Base class of all RtLab loops.
\ingroup RtBase
RtLoop encapsulates a loop that is executed reqursively.
*/
class RtLoop : public RtJob
{
	Q_OBJECT
protected:

	RtJob::JobList jobs;

	virtual void exec();

public:
	RtLoop(const QString& name, const QString& desc, RtObject* parent);
	virtual ~RtLoop(void);
	virtual void detach();

public slots:
	/// Commit a job to this loop.
	void commit(RtJob* job);
	/// Decommit a job from this loop.
	void decommit(RtJob* job);
	/// Create a new RtDelayLoop job.
	void newDelayLoop(const QString& name, unsigned int delay = 1);

};

/** A sub-loop that is executed with a specified delay.
\ingroup RtBase
RtDelayLoop is a child of an RtLoop and is executed every delay cycles of the parent loop.

The number of delay cycles is a property of RtDelayLoop and can be set by scrit code.
*/
class RtDelayLoop : public RtLoop
{
	Q_OBJECT
	/** Number of parent loop cycles before this loop is called.
	The value of 0 is allowed.
	*/
	Q_PROPERTY(uint delay READ delay WRITE setDelay)
	Q_PROPERTY(uint preload READ preload WRITE setPreload)
protected:
	unsigned int delay_, counter_, preload_;

	virtual void exec();

	virtual bool arm_() { counter_ = preload_; return RtLoop::arm_(); }
	
public:
	RtDelayLoop(const QString& name, RtLoop* parent, unsigned int delay = 1);
	virtual ~RtDelayLoop(void);

	unsigned int delay() const { return delay_; }
	unsigned int preload() const { return preload_; }
	void setDelay(unsigned int d);
	void setPreload(unsigned int d);
};

class RtJobFolder : public RtObject
{
	Q_OBJECT
public: 
	RtJobFolder(const QString& name, RtObject* parent);
	virtual ~RtJobFolder(void);
public slots:
	void newJob(const QString& name, const QString& className);
	void newTimer(const QString& name, unsigned int period_ms = 1000);
};

#endif


 