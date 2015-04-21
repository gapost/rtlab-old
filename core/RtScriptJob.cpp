#include "RtScriptJob.h"
#include "RtSession.h"
//#include <QScriptProgram>

RtScriptJob::RtScriptJob(const QString& name, RtObject* parent) :
RtJob(name,"Script Job",parent), session_(0), program_(0)
{
}
RtScriptJob::~RtScriptJob(void)
{
}
bool RtScriptJob::arm_()
{
	disarm_();

	if (code_.isEmpty()) return false;
	
    session_ = new RtSession(objectName() + "_session",this);
	program_ = new QScriptProgram(code_,objectName() + "_code");
	if (!session_->canEvaluate(code_))
	{
		throwScriptError("Error in script code.");
		disarm_();
		return false;
	}

	return RtJob::arm_();
}
void RtScriptJob::disarm_()
{
	if (program_)
	{
		delete program_;
		program_ = 0;
	}
	if (session_)
	{
        //session_->detach();
		delete session_;
		session_ = 0;
	}
	RtJob::disarm_();
}
void RtScriptJob::run()
{
	QString msg;
	if (!session_->evaluate(*program_,msg))
	{
		disarm_();
		pushError("Error executing script job",msg);
	}
}
void RtScriptJob::setCode(const QString& cd)
{
	if (cd.isEmpty() || cd==code_) return;

	{
		bool onlineChange = armed_;
		if (onlineChange)
		{
			jobLock();
			disarm_();
		}
		code_ = cd;
		if (onlineChange)
		{
			arm_();
			jobUnlock();
		}
		emit propertiesChanged();
	}
}
