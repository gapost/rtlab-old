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

#include "objectcontroller.h"

#include "RtObject.h"
#include "RtObjectInspector.h"
#include "RtRoot.h"

RtObjectInspector::RtObjectInspector(QWidget* p) : QWidget(p)
{
	treeWidget = new QTreeWidget(this);
	QVBoxLayout* vlayout = new QVBoxLayout();
	vlayout->addWidget(treeWidget);
	setLayout(vlayout);

	treeWidget->setColumnCount(2);
	QStringList headers;
	headers << "Object" << "Class";
	treeWidget->setHeaderLabels(headers);
	treeWidget->setAlternatingRowColors(true);

	foreach(QObject* o, RtObject::root()->children())
	{
		if (RtObject* rto = qobject_cast<RtObject*>(o))
			insertObject(treeWidget->invisibleRootItem(), rto, true);
	}

	connect(treeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
		this, SLOT(slotCurrentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)) );

	connect(this, SIGNAL(updateItem(QTreeWidgetItem* )),
		this, SLOT(slotUpdateItem(QTreeWidgetItem* )) , Qt::QueuedConnection);

}

RtObjectInspector::~RtObjectInspector(void)
{
}

void RtObjectInspector::customEvent (QEvent *e)
{
	QWidget::customEvent(e);
}

void RtObjectInspector::change(RtObject* obj, bool create)
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

void RtObjectInspector::insertObject(QTreeWidgetItem* parent, RtObject* obj, bool recursive)
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

void RtObjectInspector::removeObject(RtObject* obj, bool recursive)
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

void RtObjectInspector::slotInsertObject(RtObject* obj)
{
	change(obj,true);
}

void RtObjectInspector::slotRemoveObject(RtObject* obj)
{
	change(obj,false);
}



void RtObjectInspector::slotCurrentItemChanged( QTreeWidgetItem * current, QTreeWidgetItem * previous)
{
	RtObject* obj = items2objects.value(current);
	if (obj) emit currentObjectChanged(obj);
}

void RtObjectInspector::slotUpdateItem( QTreeWidgetItem * i)
{
	RtObject* obj = items2objects.value(i);
	if (obj)
		i->setData(1,Qt::DisplayRole,obj->metaObject()->className());
}

//***********************************************************************//
RtPropertyBrowser::RtPropertyBrowser(QWidget *parent) : QWidget(parent)
{
	currentObject = new QLineEdit(this);
	objectController = new ObjectController(this);

	QVBoxLayout* vlayout = new QVBoxLayout();
	vlayout->addWidget(currentObject);
	vlayout->addWidget(objectController);

	setLayout(vlayout);

	connect(currentObject,SIGNAL(editingFinished()),this,SLOT(setByUser()));

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
		currentObject->setText(obj->fullName());

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

void RtPropertyBrowser::setByUser()
{
	QString name = currentObject->text();
	RtObject* obj = RtObject::findByName(name);
	//if (obj) 
		setRtObject(obj);
}

//***********************************************************************//
RtFunctionBrowser::RtFunctionBrowser(QWidget *parent) : QWidget(parent)
{
	currentObject = new QLineEdit(this);
	methodsTree = new QTreeWidget(this);
	methodsTree->setColumnCount(1);
	methodsTree->setAlternatingRowColors(true);
	methodsTree->setHeaderHidden(true);

	QVBoxLayout* vlayout = new QVBoxLayout();
	vlayout->addWidget(currentObject);
	vlayout->addWidget(methodsTree);

	setLayout(vlayout);

	connect(currentObject,SIGNAL(editingFinished()),this,SLOT(setByUser()));

}

void RtFunctionBrowser::setRtObject(RtObject* obj)
{
	if (obj)
	{
		currentObject->setText(obj->fullName());
		populateMethods(obj->metaObject());
	}
	else
	{
		methodsTree->clear();
		//currentObject->setText("");
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

void RtFunctionBrowser::setByUser()
{
	QString name = currentObject->text();
	RtObject* obj = RtObject::findByName(name);
	setRtObject(obj);
}

//*******************************************************************//

RtErrorLog::RtErrorLog(QWidget* p) : QWidget(p)
{
	tableWidget = new QTableWidget(this);

	tableWidget->setColumnCount(4);
	QStringList headers;
	headers << "Time" << "Object" << "Type" << "Description";
	tableWidget->setHorizontalHeaderLabels(headers);

	tableWidget->setShowGrid(false);
	tableWidget->setAlternatingRowColors(true);
	tableWidget->verticalHeader()->hide();
	tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	tableWidget->setWordWrap(false);

	clearButton = new QPushButton("Clear",this);
	connect(clearButton,SIGNAL(clicked()),this,SLOT(clear()));

	QVBoxLayout* vlayout = new QVBoxLayout();
	QHBoxLayout* hlayout = new QHBoxLayout();
	hlayout->addWidget(clearButton);
	hlayout->addStretch();
	vlayout->addLayout(hlayout);
	vlayout->addWidget(tableWidget);

	setLayout(vlayout);

	RtObject::root()->addErrorSink(this);
}

RtErrorLog::~RtErrorLog(void)
{
	RtObject::root()->removeErrorSink(this);
}

void RtErrorLog::customEvent (QEvent *e)
{
	if (e->type()==RtEvent::ObjectError)
	{
		RtErrorEvent* rte = (RtErrorEvent*)e;
		RtObject* obj = (RtObject*)(rte->qObject());
		const RtObject::ErrorEntry& e = rte->errorEntry();

		tableWidget->insertRow(0);
		tableWidget->setItem(0,0,new QTableWidgetItem(e.t.toString("hh:mm:ss.zzz")));
		tableWidget->setItem(0,1,new QTableWidgetItem(e.objectName));
		tableWidget->setItem(0,2,new QTableWidgetItem(e.type));
		tableWidget->setItem(0,3,new QTableWidgetItem(e.descr));
		tableWidget->resizeRowToContents(0);
	}
	else QWidget::customEvent(e);
}

void RtErrorLog::clear()
{
	tableWidget->clear();
	QStringList headers;
	headers << "Time" << "Object" << "Type" << "Description";
	tableWidget->setHorizontalHeaderLabels(headers);
}
