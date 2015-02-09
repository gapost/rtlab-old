#ifndef _RTOBJECTBROWSER_H_
#define _RTOBJECTBROWSER_H_

#include <QWidget> 
#include <QMap>

class QTreeWidget;
class QTreeWidgetItem;
class QTableWidget;
class QTabWidget;
class RtObjectController;
class QPushButton;
class QLineEdit;

class RtPropertyBrowser;
class RtFunctionBrowser;


class RtObject;

class RtObjectBrowser : public QWidget
{
	Q_OBJECT

	QMap<RtObject*,QTreeWidgetItem*> objects2items;
	QMap<QTreeWidgetItem*,RtObject*> items2objects;
public:
    RtObjectBrowser(QWidget* p = 0);
    virtual ~RtObjectBrowser(void);

protected:
	virtual void customEvent ( QEvent *);

	void insertObject(QTreeWidgetItem* parent, RtObject* obj, bool recursive=false);
	void removeObject(RtObject* obj, bool recursive = true);
	void change(RtObject* obj, bool create);

protected slots:
    void slotCurrentItemChanged ( QTreeWidgetItem * current, QTreeWidgetItem * previous);
    void slotInsertObject(RtObject* obj);
    void slotRemoveObject(RtObject* obj);
    void slotUpdateItem(QTreeWidgetItem* i);

signals:
    void currentObjectChanged(RtObject* obj);
    void updateItem(QTreeWidgetItem* i);

protected:
	QTreeWidget* treeWidget;
    QTabWidget* tabWidget;
    RtPropertyBrowser* propertyBrowser;
    RtFunctionBrowser* functionBrowser;
    QLineEdit* currentObject;

};

class RtPropertyBrowser : public QWidget
{
	Q_OBJECT
public:
    RtPropertyBrowser(QWidget *parent = 0);
    ~RtPropertyBrowser();

protected:
	virtual void customEvent ( QEvent * e );

public slots:
    void setRtObject(RtObject* obj);

protected:
    RtObjectController* objectController;
};

class RtFunctionBrowser : public QWidget
{
	Q_OBJECT
public:
    RtFunctionBrowser(QWidget *parent = 0);

protected:
	virtual void customEvent ( QEvent * e );

public slots:
    void setRtObject(RtObject* obj);

protected:
	void populateMethods(const QMetaObject* mobj);

protected:
	QTreeWidget* methodsTree;
};


#endif


