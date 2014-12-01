#include "RtRoot.h"
#include "RtJob.h"
#include "RtVector.h"
#include "RtFigure.h"

#include <QCoreApplication>

RtRoot RtObject::root_;

RtRoot::RtRoot(void) : RtObject("rt","root object",0)
{
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




