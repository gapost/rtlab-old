#ifndef _RTOBJECT_H_
#define _RTOBJECT_H_

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QList>
#include <QSet>
#include <QScriptable>
#include <QMetaType>

#include "RtEvent.h"

#define ERROR_QUEUE_DEPTH 5

class RtRoot;

/** Base class of all RtLab objects.
\ingroup RtBase
*/
class RtObject : public QObject, protected QScriptable
{
	Q_OBJECT
	Q_PROPERTY(QString whatsThis READ whatsThis)
	Q_PROPERTY(QString lastError READ lastError)

protected:
	QString whatsThis_;

public:
	const QString& whatsThis() const { return whatsThis_; }
	QString lastError() const 
	{ 
		return error_queue.isEmpty() ? QString() : error_queue.first().toString();
	}

public:
	/**
	Structure containing error data.
	*/
	struct ErrorEntry
	{
		/// Date/time of occurence
		QDateTime t;
		/// Name of object causing the error
		QString objectName;
		/// Error type description
		QString type;
		/// Extended description (optional)
		QString descr;
		ErrorEntry(const QDateTime& at, const QString& aname, const QString& atype) :
			t(at), objectName(aname), type(atype)
		{}
		ErrorEntry(const QDateTime& at, const QString& aname, const QString& atype, const QString& adesc) :
			t(at), objectName(aname), type(atype), descr(adesc)
		{}
		ErrorEntry(const ErrorEntry& e) :
			t(e.t), objectName(e.objectName), type(e.type), descr(e.descr)
		{}
		QString toString() const
		{
			return QString("%1\t%2\t%3\t%4").arg(t.toString()).arg(objectName).arg(type).arg(descr);
		}
			//ErrorEntry(const ErrorEntry& other) : t(other.t), msg(other.msg)
		//{}
	};

protected:
	/// Queue of last errors
	QList<ErrorEntry> error_queue;
	/// Push an error in the error queue
	void pushError(const QString& type, const QString& descr = QString());

	/// Create a script object of obj in this RtObject's script engine
	void createScriptObject(RtObject* obj) const;
	/// Throw a script error with message msg
	void throwScriptError(const QString& msg) const;
	/** Check if name is a legal name for an RtObject.
	Names should start with a letter and contain letters, numbers or the underscore _.
	This function also checks if there are any sibbling objects with the same name.
	*/
	bool checkName(const QString& name) const;

protected:
	RtEventSinkMap widgets;

	void updateWidgets() 
	{
		//widgets.post(RtEvent::ObjectUpdated, this);
		widgets.postUpdate(this);
	}

public:
	void add_widget(QObject* o, RtUpdateFlag* f) { widgets.add(o,f); }
	void remove_widget(QObject* o) { widgets.remove(o); }

protected:
	void customEvent(QEvent* e)
	{
		if (RtEvent::isRtEvent(e)) rtEvent((RtEvent*)e);
		else QObject::customEvent(e);
	}
	virtual void rtEvent(RtEvent* e);

public:
    os::critical_section comm_lock;

protected:
	bool canBeKilled_;

public:
	RtObject(const QString& name, const QString& descr, RtObject* parent);
	virtual ~RtObject(void);

	//virtual void attach();
	virtual void detach();

	bool canBeKilled() const { return canBeKilled_; }

	/// Get this RtObject's parent
	RtObject* parent() const { return static_cast<RtObject*>(QObject::parent()); }

	void objectTree(QString& S, int level) const;

	/// Register this RtObject's types with the script engine.
	virtual void registerTypes(QScriptEngine* eng) { Q_UNUSED(eng); }

	/// Get the objects full name, e.g., dev.ifc1.obj1
	QString fullName() const;
	static RtObject* findByName(const QString& name);
	static QList<RtObject*> findByWildcard(const QString& wildcard, RtObject* from = 0);

public slots:
	/// Print a backtrace of recent errors
	QString errorBacktrace() const;
	/// Print a string representation of the object
	QString toString() const { return fullName(); }
	/// Print the objects children hierarchy
	QString objectTree() const
	{
		QString S;
		int ind = 0;
		objectTree(S,ind);
		return S;
	}
	/// List the objects properties
	QString listProperties() const;
	/// List the objects scriptable functions
	QString listFunctions() const;

protected:
	static RtRoot root_;

public:
	/// Obtain a pointer to the RtRoot object.
	static RtRoot* root() { return &root_; }

signals:
	/// Fired when object is deleted
	void objectDeleted(RtObject* obj);
	/// Fired when object is created
	void objectCreated(RtObject* obj);
	/// Fired when properties have changed
	void propertiesChanged();
};

class RtErrorEvent : public RtEvent
{
public:
	RtErrorEvent(RtObject* obj, const RtObject::ErrorEntry& err);
	virtual ~RtErrorEvent();

	const RtObject::ErrorEntry& errorEntry() const { return e; }

protected:
	RtObject::ErrorEntry e;
};

typedef QList<RtObject*> RtObjectList;

Q_DECLARE_METATYPE(RtObject*)
Q_DECLARE_METATYPE(RtObjectList)

int registerRtObjectStar(QScriptEngine* eng);

#endif
