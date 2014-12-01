#include "RtObject.h"
#include <QCoreApplication>

RtErrorEvent::RtErrorEvent(RtObject* obj, const RtObject::ErrorEntry& err) :
RtEvent(ObjectError,obj), e(err)
{
}
RtErrorEvent::~RtErrorEvent()
{
}

RtEventSinkMap::RtEventSinkMap()
{
}

RtEventSinkMap::~RtEventSinkMap()
{
}

void RtEventSinkMap::post(RtEvent::RtEventType t, RtObject* ptr)
{
	os::auto_lock L(lock);
	sinkmap_t::iterator i = sinkmap.begin();
	for(; i!=sinkmap.end(); ++i)
		QCoreApplication::postEvent (i.key(), new RtEvent(t, ptr)); 
}

void RtEventSinkMap::postUpdate(RtObject* ptr)
{
	os::auto_lock L(lock);
	sinkmap_t::iterator i = sinkmap.begin();
	for(; i!=sinkmap.end(); ++i)
		if (i.value()->needsUpdateNotification())
			QCoreApplication::postEvent (i.key(), 
			new RtEvent(RtEvent::ObjectUpdated, ptr)
			); 
}

void RtEventSinkMap::send(RtEvent::RtEventType t, RtObject* ptr)
{
	RtEvent e(t,ptr);
	sinkmap_t::iterator i = sinkmap.begin();
	for(; i!=sinkmap.end(); ++i)
		QCoreApplication::sendEvent ( i.key(), &e ); 
}


