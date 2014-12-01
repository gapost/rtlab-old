#ifndef _RTOBJECTINSPECTOR_H_
#define _RTOBJECTINSPECTOR_H_

#include <QWidget> 
#include <QMap>

class QTreeWidget;
class QTreeWidgetItem;
class QTableWidget;
class ObjectController;
class QPushButton;
class QLineEdit;
class QMeatObject;

class RtObject;

class RtObjectInspector : public QWidget
{
	Q_OBJECT

	QMap<RtObject*,QTreeWidgetItem*> objects2items;
	QMap<QTreeWidgetItem*,RtObject*> items2objects;
public:
	RtObjectInspector(QWidget* p = 0);
	virtual ~RtObjectInspector(void);

protected:
	virtual void customEvent ( QEvent *);

	void insertObject(QTreeWidgetItem* parent, RtObject* obj, bool recursive=false);
	void removeObject(RtObject* obj, bool recursive = true);
	void change(RtObject* obj, bool create);

protected: // slots:
	Q_SLOT void slotCurrentItemChanged ( QTreeWidgetItem * current, QTreeWidgetItem * previous);
	Q_SLOT void slotInsertObject(RtObject* obj);
	Q_SLOT void slotRemoveObject(RtObject* obj);
	Q_SLOT void slotUpdateItem(QTreeWidgetItem* i);

public:
	Q_SIGNAL void currentObjectChanged(RtObject* obj);
	Q_SIGNAL void updateItem(QTreeWidgetItem* i);

protected:
	QTreeWidget* treeWidget;

};

class RtPropertyBrowser : public QWidget 
{
	Q_OBJECT
public:
    RtPropertyBrowser(QWidget *parent = 0);
    ~RtPropertyBrowser();

protected:
	virtual void customEvent ( QEvent * e );

public:
	Q_SLOT void setRtObject(RtObject* obj);

protected:
	Q_SLOT void setByUser();

protected:
	ObjectController* objectController;
	QLineEdit* currentObject;
};

class RtFunctionBrowser : public QWidget 
{
	Q_OBJECT
public:
    RtFunctionBrowser(QWidget *parent = 0);

protected:
	virtual void customEvent ( QEvent * e );

public:
	Q_SLOT void setRtObject(RtObject* obj);

protected:
	Q_SLOT void setByUser();
	void populateMethods(const QMetaObject* mobj);

protected:
	QTreeWidget* methodsTree;
	QLineEdit* currentObject;
};


class RtErrorLog : public QWidget
{
	Q_OBJECT
public:
	RtErrorLog(QWidget* p = 0);
	virtual ~RtErrorLog(void);

protected:
	virtual void customEvent ( QEvent *);

	Q_SLOT void clear();

	//QTreeWidget* treeWidget;
	QTableWidget* tableWidget;
	QPushButton* clearButton;
};

#endif


