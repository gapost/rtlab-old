#ifndef _RTROOT_H_
#define _RTROOT_H_

#include "rtobject.h"

class RtMainWindow;

class RtRoot : public RtObject
{
	Q_OBJECT
protected:
	RtObject* jobs;
	RtObject* data;
	RtObject* figs;

	RtMainWindow* mainWindow_;

	QObjectList errorSinks;
	os::critical_section errorSinkLock;

public:
	RtRoot(void);
	virtual ~RtRoot(void);

	void objectCreation(RtObject* obj, bool c);

	RtMainWindow* mainWindow() const { return mainWindow_; }
	void setMainWindow(RtMainWindow* m) { mainWindow_ = m; }

	void addErrorSink(QObject* s);
	void removeErrorSink(QObject* s);
	void postError(RtObject* obj, const ErrorEntry& e);

	QString h5write(const QString& filename, const QString& comment);

};




#endif


