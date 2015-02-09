#include <QCoreApplication>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTreeWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QHeaderView>
#include <QMetaObject>
#include <QMetaMethod>

#include "RtObjectController.h"

#include "RtObjectBrowser.h"
#include "RtRoot.h"

RtObjectBrowser::RtObjectBrowser(QWidget* p) : QWidget(p)
{
	treeWidget = new QTreeWidget(this);
    treeWidget->setColumnCount(2);
    QStringList headers;
    headers << "Object" << "Class";
    treeWidget->setHeaderLabels(headers);
    treeWidget->setAlternatingRowColors(true);

    currentObject = new QLineEdit(this);

    tabWidget = new QTabWidget(this);
    propertyBrowser = new RtPropertyBrowser();
    tabWidget->addTab(propertyBrowser, "Properties");
    functionBrowser = new RtFunctionBrowser();
    tabWidget->addTab(functionBrowser, "Functions");

    QVBoxLayout* vlayout = new QVBoxLayout();
    vlayout->addWidget(currentObject);
    vlayout->addWidget(tabWidget);

    QHBoxLayout* hlayout = new QHBoxLayout();
    hlayout->addWidget(treeWidget);
    hlayout->addLayout(vlayout);
    setLayout(hlayout);

	foreach(QObject* o, RtObject::root()->children())
	{
		if (RtObject* rto = qobject_cast<RtObject*>(o))
			insertObject(treeWidget->invisibleRootItem(), rto, true);
	}

	connect(treeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
		this, SLOT(slotCurrentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)) );

	connect(this, SIGNAL(updateItem(QTreeWidgetItem* )),
		this, SLOT(slotUpdateItem(QTreeWidgetItem* )) , Qt::QueuedConnection);

    connect(this, SIGNAL(currentObjectChanged(RtObject*)),
        propertyBrowser, SLOT(setRtObject(RtObject*)));
    connect(this, SIGNAL(currentObjectChanged(RtObject*)),
        functionBrowser, SLOT(setRtObject(RtObject*)));

    // object creation is queued so that object is fully created
    connect(RtObject::root(), SIGNAL(objectCreated(RtObject*)),
        this, SLOT(slotInsertObject(RtObject*))); //, Qt::QueuedConnection);
    // object deletion should be normal so that all objects have the chance to deref
    connect(RtObject::root(), SIGNAL(objectDeleted(RtObject*)),
        this, SLOT(slotRemoveObject(RtObject*))); //, Qt::QueuedConnection);

    //connect(currentObject,SIGNAL(editingFinished()),this,SLOT(setByUser()));

}

RtObjectBrowser::~RtObjectBrowser(void)
{
}

void RtObjectBrowser::customEvent (QEvent *e)
{
	QWidget::customEvent(e);
}

void RtObjectBrowser::change(RtObject* obj, bool create)
{
	if (create)
	{
		QTreeWidgetItem* parentitem = objects2items.value(obj->parent());
		if (parentitem) 
		{
			insertObject(parentitem, obj);
			//treeWidget->expandItem(parentitem);
		}
	}
	else removeObject(obj);
}

void RtObjectBrowser::insertObject(QTreeWidgetItem* parent, RtObject* obj, bool recursive)
{
	QStringList nodedata;
	nodedata <<  obj->objectName() << obj->metaObject()->className();
	QTreeWidgetItem* node = new QTreeWidgetItem(parent, nodedata);
	objects2items.insert(obj,node);
	items2objects.insert(node,obj);
	//treeWidget->expandItem(node);
	emit updateItem(node);

	if (recursive)
	{
		foreach(QObject* o, obj->children())
		{
			if (RtObject* rtchild = qobject_cast<RtObject*>(o))
				insertObject(node, rtchild);
		}
	}

}

void RtObjectBrowser::removeObject(RtObject* obj, bool recursive)
{
		QTreeWidgetItem* item = objects2items.value(obj);
		if (item) 
		{
			objects2items.remove(obj);
			items2objects.remove(item);

			if (recursive)
			{
				QList<QTreeWidgetItem *> items = item->takeChildren();
				foreach(QTreeWidgetItem* i, items)
				{
					RtObject* obj1 = items2objects.value(i);
					if (obj1) removeObject(obj1);
				}
			}
			delete item;
		}
}

void RtObjectBrowser::slotInsertObject(RtObject* obj)
{
	change(obj,true);
}

void RtObjectBrowser::slotRemoveObject(RtObject* obj)
{
	change(obj,false);
}

void RtObjectBrowser::slotCurrentItemChanged( QTreeWidgetItem * current, QTreeWidgetItem * previous)
{
	RtObject* obj = items2objects.value(current);
    if (obj) {
        currentObject->setText(obj->fullName());
        emit currentObjectChanged(obj);
    }
}

void RtObjectBrowser::slotUpdateItem( QTreeWidgetItem * i)
{
	RtObject* obj = items2objects.value(i);
	if (obj)
		i->setData(1,Qt::DisplayRole,obj->metaObject()->className());
}

//***********************************************************************//
RtPropertyBrowser::RtPropertyBrowser(QWidget *parent) : QWidget(parent)
{
    objectController = new RtObjectController(this);

	QVBoxLayout* vlayout = new QVBoxLayout();
	vlayout->addWidget(objectController);

	setLayout(vlayout);
}
    
RtPropertyBrowser::~RtPropertyBrowser()
{
}

void RtPropertyBrowser::setRtObject(RtObject* obj)
{
	RtObject* old_obj = qobject_cast<RtObject*>(objectController->object());
	if (old_obj) 
	{
		disconnect(old_obj,SIGNAL(propertiesChanged()),objectController,SLOT(updateProperties()));
		//currentObject->setText("");
	}

	objectController->setObject(obj);
	if (obj)
	{
		connect(obj,SIGNAL(propertiesChanged()),objectController,SLOT(updateProperties()));
	}
}

void RtPropertyBrowser::customEvent (QEvent *e)
{
	if (RtEvent::isRtEvent(e))
	{
		RtEvent* rte = (RtEvent*)e;
		switch (rte->rtType())
		{
		case RtEvent::ObjectDeleted:
			//objectController->setObject(0);
			setRtObject(0);
			break;
		}
	}
	else QWidget::customEvent(e);
}

//***********************************************************************//
RtFunctionBrowser::RtFunctionBrowser(QWidget *parent) : QWidget(parent)
{
	methodsTree = new QTreeWidget(this);
	methodsTree->setColumnCount(1);
	methodsTree->setAlternatingRowColors(true);
	methodsTree->setHeaderHidden(true);

	QVBoxLayout* vlayout = new QVBoxLayout();
	vlayout->addWidget(methodsTree);

	setLayout(vlayout);

}

void RtFunctionBrowser::setRtObject(RtObject* obj)
{
	if (obj)
	{
		populateMethods(obj->metaObject());
	}
	else
	{
		methodsTree->clear();
	}
}

void RtFunctionBrowser::populateMethods(const QMetaObject* metaObject)
{
	methodsTree->clear();

	if (metaObject->superClass()==0) // stop at QObject
        return;

    populateMethods(metaObject->superClass());

	QTreeWidgetItem* iclass = new QTreeWidgetItem(QStringList(metaObject->className()));
	methodsTree->addTopLevelItem(iclass);
	QTreeWidgetItem* islots = new QTreeWidgetItem(iclass, QStringList("Methods"));
	QTreeWidgetItem* isignals = new QTreeWidgetItem(iclass, QStringList("Signals"));
	for(int i=metaObject->methodOffset(); i < metaObject->methodCount(); ++i)
	{
        QMetaMethod m = metaObject->method(i);
#if QT_VERSION >= 0x050000
        if (m.methodType()==QMetaMethod::Slot)
        {
            new QTreeWidgetItem(islots, QStringList(m.methodSignature()));
        }
        else if (m.methodType()==QMetaMethod::Signal)
        {
            new QTreeWidgetItem(isignals, QStringList(m.methodSignature()));
        }
    }
#else
		if (m.methodType()==QMetaMethod::Slot)
		{
			new QTreeWidgetItem(islots, QStringList(m.signature()));
		}
		else if (m.methodType()==QMetaMethod::Signal)
		{
			new QTreeWidgetItem(isignals, QStringList(m.signature()));
		}
	}
#endif


}

void RtFunctionBrowser::customEvent (QEvent *e)
{
	if (RtEvent::isRtEvent(e))
	{
		RtEvent* rte = (RtEvent*)e;
		switch (rte->rtType())
		{
		case RtEvent::ObjectDeleted:
			setRtObject(0);
			break;
		}
	}
	else QWidget::customEvent(e);
}



