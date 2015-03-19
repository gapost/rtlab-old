#ifndef _RTSESSION_H_
#define _RTSESSION_H_

#include "RtObject.h"

#include <QScriptValue>
#include <QExplicitlySharedDataPointer>
#include <QScriptProgram>
#include <QStringList>

class QScriptEngine;
class QScriptContext;
class QTimer;


class RtSession : public RtObject
{
	Q_OBJECT

protected:
    QScriptEngine* engine_;
	QTimer* wait_timer_;
	bool wait_aborted_;
    os::stopwatch watch_;
    static QScriptValue kill_func(QScriptContext *ctx, QScriptEngine *e);

public:
	RtSession(const QString& name, RtObject* parent = (RtObject*)(&root_));
	virtual ~RtSession(void);

	bool canEvaluate(const QString&);

	bool isEvaluating() const;

	bool evaluate(const QScriptProgram& program, QString& ret);

    QScriptEngine* getEngine() { return engine_; }

public slots:
	void evaluate(const QString& program);
	void abortEvaluation();

    void quit();
    void exec(const QString& fname);
    void print(const QString& str);
    void wait(uint ms);
    void textSave(const QString& str, const QString& fname);
    QString textLoad(const QString& fname);
    RtObjectList find(const QString& wildCard);
    void h5write(const QString& fname, const QString& comment);
    void beep();
    // file and folder
    QString pwd();
    bool cd(const QString& path);
    QStringList dir(const QStringList& filters = QStringList());
    QStringList dir(const QString& filter);
    bool isDir(const QString& name);
    // windows
    void saveWindowState(const QString& fname);
    void restoreWindowState(const QString& fname);
    // timing
    void tic() { watch_.start(); }
    double toc() { watch_.stop(); return watch_.sec(); }
    // system call
    QString system(const QString& comm);
    bool ispc()
    {
#ifdef __linux__
        return false;
#else
        return true;
#endif
    }



signals:
	void stdOut(const QString&);
	void stdErr(const QString&);
	void endSession();
};

#endif


