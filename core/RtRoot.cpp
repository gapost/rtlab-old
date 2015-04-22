#include "RtRoot.h"
#include "RtJob.h"
#include "RtVector.h"
#include "RtFigure.h"
#include "RtLogFile.h"

#include <QCoreApplication>
#include <QDir>

RtRoot RtObject::root_;

RtRoot::RtRoot(void) : RtObject("rt","root object",0)
{
    // root dir = current dir when app starts
    QDir pwd = QDir::current();
    rootDir_ = pwd.absolutePath();

    // log dir = rootDir/log
    if (!pwd.cd("log")) {
        if (pwd.mkdir("log")) pwd.cd("log");
    }
    logDir_ = pwd.absolutePath();

    // create error log file
    errorLog_ = new RtLogFile(false,',',this);
    errorLog_->open(RtLogFile::getDecoratedName("error"));
    if (errorLog_->isOpen()) addErrorSink(this);

	jobs = new RtJobFolder("jobs",this);
	data = new RtDataContainer("data","",this);
	figs = new RtFigureFolder("figs",this);
}

RtRoot::~RtRoot(void)
{
	foreach(QObject* obj, children())
	{
		RtObject* rtobj = qobject_cast<RtObject*>(obj);
		if (rtobj) 
		{
			rtobj->detach();
			delete rtobj;
		}
	}
}

void RtRoot::objectCreation(RtObject* obj, bool create)
{
	if (obj==this) return;
	if (obj->parent()==0) return;

	if (create)
		emit objectCreated(obj);
	else 
		emit objectDeleted(obj);
}

void RtRoot::postError(RtObject* obj, const ErrorEntry& e)
{
	os::auto_lock L(errorSinkLock);
	foreach(QObject* o, errorSinks)
		QCoreApplication::postEvent(o, new RtErrorEvent(obj, e));
}

void RtRoot::addErrorSink(QObject* s)
{
	os::auto_lock L(errorSinkLock);
	if (!errorSinks.contains(s)) errorSinks.push_back(s);
}
void RtRoot::removeErrorSink(QObject* s)
{
	os::auto_lock L(errorSinkLock);
	errorSinks.removeOne(s);
}

void RtRoot::rtEvent(RtEvent *e)
{
    if (e->rtType() == RtEvent::ObjectError) {
        const ErrorEntry& err = ((RtErrorEvent*)e)->errorEntry();

        if (errorLog_) *errorLog_ <<
               QString("%1,%2,%3,%4,%5")
                   .arg(err.t.toString("dd.MM.yyyy"))
                   .arg(err.t.toString("hh:mm:ss.zzz"))
                   .arg(err.objectName)
                   .arg(err.type).arg(err.descr);

        e->accept();

    } else RtObject::rtEvent(e);
}




