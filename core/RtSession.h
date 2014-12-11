#ifndef _RTSESSION_H_
#define _RTSESSION_H_

#include "RtObject.h"

#include <QScriptValue>
#include <QExplicitlySharedDataPointer>
#include <QScriptProgram>

class QScriptEngine;
class QScriptContext;
class QTimer;

class RtSession : public RtObject
{
	Q_OBJECT

protected:
	QScriptEngine* engine;
	QTimer* wait_timer_;
	bool wait_aborted_;

protected:
	static QScriptValue quit_func(QScriptContext *ctx, QScriptEngine *e);
	static QScriptValue exec_func(QScriptContext *ctx, QScriptEngine *e);
	static QScriptValue print_func(QScriptContext *ctx, QScriptEngine *e);
	static QScriptValue kill_func(QScriptContext *ctx, QScriptEngine *e);
	static QScriptValue wait_func(QScriptContext *ctx, QScriptEngine *e);
	static QScriptValue find_func(QScriptContext *ctx, QScriptEngine *e);
	static QScriptValue textSave_func(QScriptContext *ctx, QScriptEngine *e);
	static QScriptValue textLoad_func(QScriptContext *ctx, QScriptEngine *e);
	static QScriptValue h5write_func(QScriptContext *ctx, QScriptEngine *e);
	static QScriptValue beep_func(QScriptContext *ctx, QScriptEngine *e);

	virtual void register_functions();

public:
	RtSession(const QString& name, RtObject* parent = (RtObject*)(&root_));
	virtual ~RtSession(void);

	bool canEvaluate(const QString&);

	bool wait(uint ms);
	bool isEvaluating() const;

	bool evaluate(const QScriptProgram& program, QString& ret);

public slots:
	void evaluate(const QString& program);
	void abortEvaluation();

signals:
	void stdOut(const QString&);
	void stdErr(const QString&);
	void endSession();
};

#endif


