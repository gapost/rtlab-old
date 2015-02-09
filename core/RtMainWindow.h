#ifndef _RTMAINWINDOW_H_
#define _RTMAINWINDOW_H_

#include <QMainWindow>

class QMdiArea;

class RtChannelViewer;
class RtErrorLog;
class RtObjectBrowser;
class QAction;

class RtMainWindow : public QMainWindow
{
public:
	RtMainWindow(void);
	virtual ~RtMainWindow(void);

    void addFigureWindow(QWidget* child, const QString& name);
    void addDockPane(QWidget* child, const QString &name);

	RtChannelViewer* channelViewer() const { return channelViewer_; }

protected:
    RtObjectBrowser* objectBrowser;
	RtErrorLog* errorLog;
	RtChannelViewer* channelViewer_;

        QMdiArea *mdiArea;

	QList<QAction*> toggleDockersActions;

	void createDockers();
};

#endif

