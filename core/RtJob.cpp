#include "RtJob.h"
#include "RtTimerLoop.h"
#include "RtDataChannel.h"
#include "RtDataBuffer.h"
#include "RtBinaryChannelOp.h"
#include "RtInterpChannel.h"
#include "RtScriptJob.h"

#include <QCoreApplication>
#include <QScriptContext>

RtJob::RtJob(const QString& name, const QString& desc, RtObject* parent) :
	RtObject(name,desc,parent), armed_(false)
{
	RtJob* p = qobject_cast<RtJob*>(parent);
	if (p) p->addChild(this);
}
RtJob::~RtJob(void)
{
}
void RtJob::detach()
{
	// detach from my loop
	if (loop_) loop_->decommit(this);
	// detach from my parent job (if it is a job)
	RtJob* p = qobject_cast<RtJob*>(parent());
	if (p) p->removeChild(this);
	disarm_();
	RtObject::detach();
}

bool RtJob::throwIfArmed()
{
	if (armed_) throwScriptError("Not possible when armed");
	return armed_;
}

void RtJob::jobLock()
{
	comm_lock.lock();
	subjobs.lock();
}

void RtJob::jobUnlock()
{
	subjobs.unlock();
	comm_lock.unlock();
}

void RtJob::JobList::lock()
{
	JobList::iterator i = begin();
	for(; i!=end(); ++i) (*i)->jobLock();
}

void RtJob::JobList::unlock()
{
	if (!empty())
	{
		JobList::iterator i = end();
		do {
			--i;
			(*i)->jobUnlock();
		} while (i!=begin());
	}
}

void RtJob::addChild(RtJob* job)
{
    os::auto_lock L(comm_lock);
	Q_ASSERT(subjobs.indexOf(job)==-1);
	subjobs.push_back(job);
}

void RtJob::removeChild(RtJob* job)
{
    os::auto_lock L(comm_lock);
	//Q_ASSERT(subjobs.count(job)==1);
	subjobs.removeOne(job);
}
void RtJob::exec()
{
	//** Job has been locked at the RtLoop level
	if (armed_)
	{
		// run this job's task
		run();
		// execute all child tasks
		subjobs.exec();
	}
}
void RtJob::forceExec()
{
	// Excecute even if not armed
	//** Job has been locked at the RtLoop level
	//if (armed_)
	{
		// run this job's task
		run();
		// execute all child tasks
		subjobs.forceExec();
	}
}

bool RtJob::setArmed(bool on)
{
	jobLock();
	if (on) 
	{
		bool ok = arm_();
		if (ok)
		{
			foreach(RtJob* j, subjobs)
			{
				if (!(ok = j->setArmed(true))) break;
			}
		}
		armed_ = ok;
	}
	else 
	{
		disarm_();
		foreach(RtJob* j, subjobs) j->setArmed(false);
		armed_ = false;
	}
	jobUnlock();
	emit propertiesChanged();
	return armed_;
}
RtObject *RtJob::newJob(const QString& name, const QString& classname)
{
    if (!checkName(name)) return 0;
	RtJob* j = newJobImpl(this, context(), name, classname);
	if (j) this->createScriptObject(j);
    return j;
}
RtJob* RtJob::newJobImpl(RtObject* parent, QScriptContext* ctx, 
				  const QString& name, const QString& className)
{
	static const char* classNames[] = {
		"",
		"ScriptJob",
		"DataChannel",
		"TestChannel",
		"TimeChannel",
		"BinaryOp",
		"DataBuffer",
		"LinearCorrelator",
		"Interpolator",
		0
	};

	const char** p = classNames;
	int i=0;
	while(*p && className!=*p) 
	{ 
		++p; ++i; 
	};

	// invalid className
	if (*p==0)
	{
		QString usage(
			"usage:\n"
			"  newJob(String name, String className)\n\n"
			"Valid className options:\n"
			"  empty string (default job)\n"
			);
		p = classNames;
		p++;
		while(*p) 
		{
			usage += "  \"";
			usage += *p;
			usage += "\"\n";
			++p; 
		};
		ctx->throwError(QScriptContext::SyntaxError,usage);
		return 0;
	}

	RtJob* j;
	switch(i)
	{
	case 0: j = new RtJob(name,"",parent); break;
	case 1: j = new RtScriptJob(name,parent); break;
	case 2: j = new RtDataChannel(name,"",parent); break;
	case 3: j = new RtTestChannel(name,parent); break;
	case 4: j = new RtTimeChannel(name,parent); break;
	case 5: j = new RtBinaryChannelOp(name,parent); break;
	case 6: j = new RtDataBuffer(name,parent); break;
	case 7: j = new RtLinearCorrelator(name,parent); break;
	case 8: j = new RtInterpolationChannel(name,parent); break;
	}
	return j;
}
//////////////////////////////////////////////////////////////////////////////
RtLoop::RtLoop(const QString& name, const QString& desc, RtObject* parent) :
	RtJob(name,desc,parent)
{
}

RtLoop::~RtLoop(void)
{
}

void RtLoop::detach()
{
	disarm_();
	RtJob::detach();
}

void RtLoop::commit(RtJob* job)
{
    os::auto_lock L(comm_lock);
	if (jobs.indexOf(job)==-1)
	{
		jobs.push_back(job);
		job->setLoop(this);
	}
}

void RtLoop::decommit(RtJob* job)
{
    os::auto_lock L(comm_lock);
	int i = jobs.indexOf(job);
	if (i!=-1)
	{
		jobs.removeAt(i);
		job->setLoop(0);
	}
}

void RtLoop::exec()
{
	// Lock Loop & subjobs
	jobLock();
	// Lock committed jobs
	jobs.lock();

	RtJob::exec();
	jobs.exec();

	// unlock everything in reverse order
	jobs.unlock();
	jobUnlock();
}
void RtLoop::newDelayLoop(const QString& name, unsigned int d)
{
	if (!checkName(name)) return;
	RtDelayLoop* obj = new RtDelayLoop(name,this,d);
	createScriptObject(obj);
}	
//////////////////////////////////////////////////////////////////
RtDelayLoop::RtDelayLoop(const QString& name, RtLoop* parent, unsigned int d) :
RtLoop(name,"delay sub-loop",parent), delay_(d), counter_(d), preload_(0)
{
}

RtDelayLoop::~RtDelayLoop(void)
{
}
void RtDelayLoop::exec()
{
	comm_lock.lock();
	if (counter_) counter_--;
	if (counter_ == 0) 
	{
		RtLoop::exec();
		counter_ = delay_;
	}
	comm_lock.unlock();
}
void RtDelayLoop::setDelay(unsigned int d)
{
	if (delay_ != d)
	{
		// locked code
		{
            os::auto_lock L(comm_lock);
			delay_ = d;
			//counter_ = delay_;
		}
		emit propertiesChanged();
	}
}
void RtDelayLoop::setPreload(unsigned int d)
{
	if (preload_ != d)
	{
		preload_ = d;
		emit propertiesChanged();
	}
}
//////////////////////////////////////////////////////////////////////////
RtJobFolder::RtJobFolder(const QString& name, RtObject* parent) :
RtObject(name,"rtlab jobs folder",parent)
{
	canBeKilled_ = false;
}

RtJobFolder::~RtJobFolder(void)
{
}

RtObject* RtJobFolder::newTimer(const QString& name, unsigned int period_ms)
{
    if (!checkName(name)) return 0;
	RtTimerLoop* obj = new RtTimerLoop(name,this,period_ms);
	createScriptObject(obj);
    return obj;
}

RtObject *RtJobFolder::newJob(const QString& name, const QString& classname)
{
    if (!checkName(name)) return 0;
	RtJob* j = RtJob::newJobImpl(this,context(),name,classname);
	if (j) createScriptObject(j);
    return j;
}


