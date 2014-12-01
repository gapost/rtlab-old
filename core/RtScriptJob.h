#ifndef _RTSCRIPTJOB_H_
#define _RTSCRIPTJOB_H_

#include "RtJob.h"

class QScriptProgram;
class RtSession;

class RtScriptJob : public RtJob
{
	Q_OBJECT
	/** Script code executed by the job.
	*/
	Q_PROPERTY(QString code READ code WRITE setCode)

protected:
	QString code_;

	RtSession* session_;
	QScriptProgram* program_;

	/// Creates and initializes the job's script engine
	virtual bool arm_();
	/// Deletes the job's script engine
	virtual void disarm_();
	/// Runs the script code
	virtual void run();

public:
	RtScriptJob(const QString& name, RtObject* parent);
	virtual ~RtScriptJob(void);

	const QString& code() const { return code_; }
	void setCode(const QString& cd);

};

#endif

