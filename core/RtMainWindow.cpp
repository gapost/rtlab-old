#include "RtMainWindow.h"
#include "RtErrorLog.h"
#include "RtChannelWidget.h"
#include "RtRoot.h"
#include "RtObjectBrowser.h"

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
    QDockWidget *dock;

    dock = new QDockWidget("Object Browser", this);
    dock->setObjectName("objectBrowserDocker");
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    objectBrowser = new RtObjectBrowser(dock);
    dock->setWidget(objectBrowser);
    dock->setFloating(true);
    dock->hide();
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
    dock->setFloating(true);
	dock->hide();
    toggleDockersActions << dock->toggleViewAction();
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
