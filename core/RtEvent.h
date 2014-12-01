#ifndef _RTEVENT_H_
#define _RTEVENT_H_


#include <QEvent>
#include <QAtomicInt>
#include <QMap>

#include "os_utils.h"

class RtObject;
class QObject;

class RtEvent : public QEvent
{
public:
	enum RtEventType {
		RtEventMin = QEvent::User,
		ObjectDeleted = QEvent::User,
		ObjectUpdated,
		ObjectError,
		WidgetAdd,
		WidgetRemove,
		ChildJobAdded,
		ChildJobRemoved,
		RtEventMax = QEvent::User + 20
	};

	RtEvent(RtEventType t, QObject* p) : QEvent((QEvent::Type)t), o(p)
	{}
	virtual ~RtEvent(void)
	{}	

	RtEventType rtType() const { return static_cast<RtEventType>(t); }

	QObject*  qObject() const { return o; }

	static bool isRtEvent(QEvent* e)
	{
		int n = static_cast<int>(e->type());
		return (n>=static_cast<int>(RtEventMin) && n<static_cast<int>(RtEventMax));
	}

protected:
	QObject* o;
};

class RtUpdateFlag
{
	QAtomicInt update_flag_;

protected:
	inline void resetUpdateFlag()
	{
		update_flag_ = 0;
	}

public:
	RtUpdateFlag()
	{}
	~RtUpdateFlag()
	{}

	inline bool needsUpdateNotification()
	{
		return update_flag_.fetchAndAddOrdered(1)==0;
	}
};

class RtEventSinkMap
{
	typedef QMap<QObject*,RtUpdateFlag*> sinkmap_t;
	QMap<QObject*,RtUpdateFlag*> sinkmap;
    os::critical_section lock;
public:
	RtEventSinkMap();
	~RtEventSinkMap();

	void add(QObject* o, RtUpdateFlag* f)
	{
        os::auto_lock L(lock);
		sinkmap.insert(o,f);
	}
	void remove(QObject* o)
	{
        os::auto_lock L(lock);
		sinkmap.remove(o);
	}
	void post(RtEvent::RtEventType t, RtObject* ptr);
	void postUpdate(RtObject* ptr);
	void send(RtEvent::RtEventType t, RtObject* ptr);
};

#endif


