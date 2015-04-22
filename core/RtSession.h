#ifndef _RTSESSION_H_
#define _RTSESSION_H_

#include "RtObject.h"

#include <QScriptValue>
#include <QExplicitlySharedDataPointer>
#include <QScriptProgram>
#include <QStringList>
#include <QSet>

class QScriptEngineDebugger;
class QScriptEngine;
class QScriptContext;
class QTimer;

class RtLogFile;


class RtSession : public QObject
{
	Q_OBJECT

protected:
    QScriptEngine* engine_;
    QScriptEngineDebugger* debugger_;
	QTimer* wait_timer_;
	bool wait_aborted_;
    os::stopwatch watch_;

    // console index = 1,2,3,...
    // idx_ = 0 means not indexed, silent consoles
    // not indexed consoles have no log file
    int idx_;
    // set contains actice index values
    static QSet<int> idx_set;
    RtLogFile* logFile_;

    static QScriptValue kill_func(QScriptContext *ctx, QScriptEngine *e);

public:

    RtSession(const QString& name, QObject* parent, bool not_indexed = false);
	virtual ~RtSession(void);

    int index() const { return idx_; }

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
    // set debugging on (enable Qt script debugger)
    void debug(bool on);
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
    // display in status bar
    void status(const QString& msg, int tmo=0);
    // check if it is win32 or linux
    bool ispc()
    {
#ifdef __linux__
        return false;
#else
        return true;
#endif
    }

protected slots:
    void log_in(const QString& str)  { log__(0,str); }
    void log_out(const QString& str) { log__(1,str); }
    void log_err(const QString& str) { log__(2,str); }

protected:
    void log__(int fd, const QString& str);




signals:
	void stdOut(const QString&);
	void stdErr(const QString&);
	void endSession();
};

#endif


