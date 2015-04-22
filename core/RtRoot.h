#ifndef _RTROOT_H_
#define _RTROOT_H_

#include "RtObject.h"

class RtMainWindow;
class RtLogFile;

class RtRoot : public RtObject
{
	Q_OBJECT

    Q_PROPERTY(QString rootDir READ rootDir)
    Q_PROPERTY(QString logDir READ logDir)

protected:

    QString rootDir_, logDir_;
	RtObject* jobs;
	RtObject* data;
	RtObject* figs;

	RtMainWindow* mainWindow_;

	QObjectList errorSinks;
	os::critical_section errorSinkLock;

    RtLogFile* errorLog_;

protected:
    virtual void rtEvent(RtEvent* e);

public:
	RtRoot(void);
	virtual ~RtRoot(void);

    const QString& rootDir() const { return rootDir_; }
    const QString& logDir() const { return logDir_; }

	void objectCreation(RtObject* obj, bool c);

	RtMainWindow* mainWindow() const { return mainWindow_; }
	void setMainWindow(RtMainWindow* m) { mainWindow_ = m; }

	void addErrorSink(QObject* s);
	void removeErrorSink(QObject* s);
	void postError(RtObject* obj, const ErrorEntry& e);

	QString h5write(const QString& filename, const QString& comment);

};




#endif


