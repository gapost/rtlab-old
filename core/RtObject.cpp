#include "RtObject.h"
#include "RtRoot.h"
#include <QScriptContext>
#include <QScriptEngine>
#include <QCoreApplication>
#include <QDebug>
#include <QStringList>
#include <QRegExp>
#include <QMetaObject>
#include <QMetaProperty>
#include <QMetaEnum>

RtObject::RtObject(const QString& name, const QString& descr, RtObject* parent) : 
QObject(parent), whatsThis_(descr)
{
	// first childs of root cannot be deleted!
	canBeKilled_ = parent != &root_;
	setObjectName(name);
	root_.objectCreation(this,true);
	qDebug() << "creating" << fullName() << "@" << (void*)this;
}

RtObject::~RtObject(void)
{
	qDebug() << "destroying" << fullName() << "@" << (void*)this;
}

/** Attach this RtObject to the Rt-framework.
This function should always be called right after the constructor.
It establishes the required links to other objects of the framework. 
This has to be done after the constructor so that the object's C++ pointer
is fully qualified.
*/
//void RtObject::attach()
//{
//	qDebug() << "attaching" << fullName() << "@" << (void*)this;
//	root_.objectCreation(this,true);
//}
/** Detach this RtObject to the Rt-framework.
This function should always be called before the destructor.
It removes links to this object from other objects in the framework.
It stops RtLoop objects, disarms RtJob objects, etc. so that they can
be safely deleted.
*/
void RtObject::detach()
{
	qDebug() << "detaching" << fullName() << "@" << (void*)this;
	root_.objectCreation(this,false);
	widgets.send(RtEvent::ObjectDeleted,this);
	foreach(QObject* obj, children())
	{
		RtObject* rtobj = qobject_cast<RtObject*>(obj);
		if (rtobj) 
		{
			rtobj->detach();
		}
	}
}

QString RtObject::errorBacktrace() const
{
	QString S;
	int i=0;
	foreach(const ErrorEntry& e, error_queue)
	{
		if (i) S += '\n';
		S += e.toString();
		i++;
	}
	return S;
}

void RtObject::createScriptObject(RtObject* obj) const
{
	if (!engine()) return;

	obj->registerTypes(engine());
	QScriptValue v = engine()->newQObject(obj,
		QScriptEngine::QtOwnership,
		QScriptEngine::ExcludeDeleteLater
		);
	thisObject().setProperty(
		obj->objectName(), 
		v, 
		QScriptValue::Undeletable
		);
}

void RtObject::throwScriptError(const QString& msg) const
{
	if (context()) context()->throwError(msg);
}


void RtObject::objectTree(QString& S, int level) const
{
	QString pre;
	if (level)
	{
		int k = level;
		pre.prepend("|--"); k--;
		while (k)
		{
			pre.prepend("|  "); k--;
		}
	}
	S += pre + objectName() + '\n';
	level += 1;
	foreach(QObject* obj, children())
	{
		RtObject* rtobj = qobject_cast<RtObject*>(obj);
		if (rtobj) rtobj->objectTree(S, level);
	}
}

bool RtObject::checkName(const QString& name) const
{
	int n = name.length();
	if (n==0)
	{
		throwScriptError("Empty string is not allowed for object name.");
		return false;
	}
	int i = 0;
	if (!name.at(i++).isLetter())
	{
		throwScriptError("Object names must start with a letter.");
		return false;
	}
	while(i<n)
	{
		const QChar ch = name.at(i++);
		if (!(ch.isLetterOrNumber() || ch=='_'))
		{
			throwScriptError("Object names may only contain letters, numbers & the underscore _.");
			return false;
		}
	}
	QObject* obj;
	foreach(obj, children())
	{
		if (obj->objectName()==name)
		{
			throwScriptError("Name is used by another object.");
			return false;
		}
	}
	return true;
}

void RtObject::rtEvent(RtEvent* e)
{
	QObject* obj;
	RtUpdateFlag* fl;
	switch (e->rtType())
	{
	case RtEvent::WidgetAdd:
		obj = e->qObject();
		fl = dynamic_cast<RtUpdateFlag*>(obj);
		if (fl) {
			widgets.add(obj,fl);
			e->accept();
		} else e->ignore();
		break;
	case RtEvent::WidgetRemove:
		widgets.remove(e->qObject());
		e->accept();
		break;
	}
}

QString RtObject::fullName() const
{
	QString name = objectName();
	RtObject* p = parent();
	while(p && p!=&root_)
	{
		name.prepend('.');
		name.prepend(p->objectName());
		p = p->parent();
	}
	return name;
}

RtObject* RtObject::findByName(const QString& name)
{
	QStringList tokens = name.split('.');
	if (tokens.isEmpty()) return 0;

	RtObject* o = &root_;
	do
	{
		QString str = tokens.front();
		tokens.pop_front();
		RtObject* child = 0;
		foreach(QObject* q, o->children())
		{
			RtObject* p = qobject_cast<RtObject*>(q);
			if (p && p->objectName()==str)
			{
				child = p;
				break;
			}
		}
		o = child;
	}
	while (!tokens.isEmpty() && o);
	return o;
}

void findByWildcardHelper(const QRegExp& rx, QList<RtObject*>& lst, const RtObject* from)
{
	foreach(const QObject* q, from->children())
	{
		const RtObject* o = qobject_cast<const RtObject*>(q);
		if (o)
		{
			if (rx.exactMatch(o->fullName())) lst.push_back(const_cast<RtObject*>(o));
			findByWildcardHelper(rx,lst,o);
		}
	}
}

QList<RtObject*> RtObject::findByWildcard(const QString& wildcard, RtObject* from)
{
	QList<RtObject*> lst;

	if (from==0) from = &root_;

	QRegExp rx(wildcard);
	rx.setPatternSyntax(QRegExp::Wildcard);

	findByWildcardHelper(rx, lst, from);

	return lst;
}

void RtObject::pushError(const QString& type, const QString& descr)
{
	ErrorEntry e(QDateTime::currentDateTime(), fullName(), type, descr);
	{
		os::auto_lock L(comm_lock);
		if (error_queue.size()==ERROR_QUEUE_DEPTH) error_queue.pop_back();
		error_queue.push_front(e);
	}
	root_.postError(this,e);

}

void listPropertiesHelper(const RtObject* m_object, QString& S, const QMetaObject* metaObject, int& level)
{
	const QMetaObject* super = metaObject->superClass();

	if (!super) return;

	listPropertiesHelper(m_object, S, super, level);

	//if (level) S += '\n';
	//S += metaObject->className();
	//S += " properties:\n";

	QString P;
	for (int idx = metaObject->propertyOffset(); idx < metaObject->propertyCount(); idx++) 
	{
		QMetaProperty metaProperty = metaObject->property(idx);

		P += "  ";
		P += metaProperty.name();
		P += " : ";


		if (!metaProperty.isReadable()) P += "< Non Readable >";
		else if (metaProperty.isEnumType()) 
		{
			if (metaProperty.isFlagType()) P += "< Flags >";
			else
			{
				QMetaEnum metaEnum = metaProperty.enumerator();
				int i = *reinterpret_cast<const int *>(metaProperty.read(m_object).constData());
				P += metaEnum.valueToKey(i);
			}
		}
		else P += metaProperty.read(m_object).toString();
		P += '\n';
	}

	if (!P.isEmpty())
	{
		S += metaObject->className();
		S += '\n';
		S += P;
	}

	level++;
}

QString RtObject::listProperties() const
{
	QString S;
	int level = 0;
	listPropertiesHelper(this, S, metaObject(), level);
	return S;
}

void listFunctionsHelper(QString& S, const QMetaObject* metaObject, int& level)
{
	const QMetaObject* super = metaObject->superClass();

	if (!super) return;

	listFunctionsHelper(S, super, level);

	for(int i=metaObject->methodOffset(); i < metaObject->methodCount(); ++i)
	{
		QMetaMethod m = metaObject->method(i);
		if (m.methodType()==QMetaMethod::Slot)
		{
			S += m.signature();
			S += '\n';
		}
		else if (m.methodType()==QMetaMethod::Signal)
		{
			S += m.signature();
			S += " [slot]";
			S += '\n';
		}
	}

	level++;
}

QString RtObject::listFunctions() const
{
	QString S;
	int level = 0;
	listFunctionsHelper(S, metaObject(), level);
	return S;
}

//###################

typedef RtObject* RtObjectStar;

QScriptValue toScriptValue(QScriptEngine *eng, const RtObjectStar& obj)
{
    return eng->newQObject(obj,
                           QScriptEngine::AutoOwnership,
                           QScriptEngine::ExcludeDeleteLater);
}

void fromScriptValue(const QScriptValue &value, RtObjectStar& obj)
{
    obj = qobject_cast<RtObject*>(value.toQObject());
}

QScriptValue toScriptValue(QScriptEngine *eng, const RtObjectList& L)
{
    QScriptValue V = eng->newArray();
    RtObjectList::const_iterator begin = L.begin();
    RtObjectList::const_iterator end = L.end();
    RtObjectList::const_iterator it;
    for (it = begin; it != end; ++it)
        V.setProperty(quint32(it - begin), qScriptValueFromValue(eng, *it));
    return V;
}

void fromScriptValue(const QScriptValue &value, RtObjectList& L)
{
    quint32 len = value.property("length").toUInt32();
    for (quint32 i = 0; i < len; ++i) {
        QScriptValue item = value.property(i);
        L.push_back(qscriptvalue_cast<RtObject*>(item));
    }
}

int registerRtObjectStar(QScriptEngine* eng)
{
    int i1 = qScriptRegisterMetaType<RtObjectStar>(eng,toScriptValue,fromScriptValue);
    int i2 = qScriptRegisterMetaType<RtObjectList>(eng,toScriptValue,fromScriptValue);
    return i1 && i2;
}
















