#include "RtMainWindow.h"
#include "RtObjectInspector.h"
#include "RtChannelWidget.h"
#include "RtRoot.h"

#include <QMdiArea>
#include <QMdiSubWindow>
#include <QDockWidget>

RtMainWindow::RtMainWindow(void)
{
}

RtMainWindow::~RtMainWindow(void)
{
}

void RtMainWindow::createDockers()
{
    QDockWidget *dock = new QDockWidget("Object Inspector", this);
    dock->setObjectName("objectInspectorDocker");
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	objectInspector = new RtObjectInspector(dock);   
    dock->setWidget(objectInspector);
    addDockWidget(Qt::LeftDockWidgetArea, dock);
    toggleDockersActions << dock->toggleViewAction();

    dock = new QDockWidget("Property Browser", this);
    dock->setObjectName("propertyBrowserDocker");
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	propertyBrowser = new RtPropertyBrowser(dock);   
    dock->setWidget(propertyBrowser);
    addDockWidget(Qt::LeftDockWidgetArea, dock);
    toggleDockersActions << dock->toggleViewAction();

    dock = new QDockWidget("Function Browser", this);
    dock->setObjectName("functionBrowserDocker");
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	functionBrowser = new RtFunctionBrowser(dock);   
    dock->setWidget(functionBrowser);
    addDockWidget(Qt::LeftDockWidgetArea, dock);
    toggleDockersActions << dock->toggleViewAction();

	dock = new QDockWidget("Channel Viewer", this);
    dock->setObjectName("channelViewerDocker");
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	channelViewer_ = new RtChannelViewer(dock);   
    dock->setWidget(channelViewer_);
    addDockWidget(Qt::RightDockWidgetArea, dock);
    toggleDockersActions << dock->toggleViewAction();

	dock = new QDockWidget("Error Log", this);
    dock->setObjectName("errorLogDocker");
    dock->setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
	errorLog = new RtErrorLog(dock);   
    dock->setWidget(errorLog);
    addDockWidget(Qt::RightDockWidgetArea, dock);
	dock->hide();
    toggleDockersActions << dock->toggleViewAction();

	connect(objectInspector, SIGNAL(currentObjectChanged(RtObject*)),
		propertyBrowser, SLOT(setRtObject(RtObject*)));
	connect(objectInspector, SIGNAL(currentObjectChanged(RtObject*)),
		functionBrowser, SLOT(setRtObject(RtObject*)));

	// object creation is queued so that object is fully created
	connect(RtObject::root(), SIGNAL(objectCreated(RtObject*)),
		objectInspector, SLOT(slotInsertObject(RtObject*))); //, Qt::QueuedConnection);
	// object deletion should be normal so that all objects have the chance to deref
	connect(RtObject::root(), SIGNAL(objectDeleted(RtObject*)),
		objectInspector, SLOT(slotRemoveObject(RtObject*))); //, Qt::QueuedConnection);
}

void RtMainWindow::addFigureWindow(QWidget *child, const QString &name)
{
	QMdiSubWindow *subWindow = new QMdiSubWindow();
    subWindow->setObjectName(name);
	subWindow->setWidget(child);
	subWindow->resize(400,300);
	mdiArea->addSubWindow(subWindow);
}

void RtMainWindow::addDockPane(QWidget *child, const QString& name)
{
	QDockWidget *dock = new QDockWidget(this);
    dock->setObjectName(name);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	//dock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    dock->setWidget(child);
	dock->setFloating(true);
	dock->show();
	//addDockWidget(Qt::RightDockWidgetArea, dock);
}
