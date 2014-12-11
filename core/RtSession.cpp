#include "RtSession.h"
#include "RtRoot.h"
#include "RtTypes.h"
#include "os_utils.h"

#include <QDebug>
#include <QScriptEngine>
#include <QFile>
#include <QTextStream>
#include <QTimer>
#include <QCoreApplication>
#include <QPointer>
#include <QStringList>



#define PROC_EVENTS_INTERVAL 250

RtSession::RtSession(const QString& name, RtObject* parent) : RtObject(name,"session",parent)
{
	engine = new QScriptEngine(this);
	engine->setProcessEventsInterval ( PROC_EVENTS_INTERVAL );
	register_functions();

        const QObjectList& chlist = root_.children();

        foreach(QObject* o, chlist)
	{
		RtObject* rto = qobject_cast<RtObject*>(o);
		if (rto) 
		{
			QScriptValue v = engine->newQObject(rto,
				QScriptEngine::QtOwnership,
				QScriptEngine::ExcludeDeleteLater
				);

			engine->globalObject().setProperty(
				rto->objectName(),
				v,
				QScriptValue::Undeletable
				);
		}
	}

	wait_timer_ = new QTimer(this);
	wait_timer_->setSingleShot(true);

}

RtSession::~RtSession( void)
{
	Q_ASSERT(!isEvaluating());
}

void RtSession::register_functions()
{
    QScriptValue global = engine->globalObject();

	QScriptValue quitFunction = engine->newFunction(quit_func);
    global.setProperty(QLatin1String("quit"), quitFunction);
	QScriptValue execFunction = engine->newFunction(exec_func);
    global.setProperty(QLatin1String("exec"), execFunction);
	QScriptValue printFunction = engine->newFunction(print_func);
    global.setProperty(QLatin1String("print"), printFunction);
	QScriptValue killFunction = engine->newFunction(kill_func);
    global.setProperty(QLatin1String("kill"), killFunction);
	QScriptValue waitFunction = engine->newFunction(wait_func);
    global.setProperty(QLatin1String("wait"), waitFunction);
	QScriptValue findFunction = engine->newFunction(find_func);
    global.setProperty(QLatin1String("find"), findFunction);
	QScriptValue textSaveFunction = engine->newFunction(textSave_func);
    global.setProperty(QLatin1String("textSave"), textSaveFunction);
	QScriptValue textLoadFunction = engine->newFunction(textLoad_func);
    global.setProperty(QLatin1String("textLoad"), textLoadFunction);
	QScriptValue h5writeFunction = engine->newFunction(h5write_func);
    global.setProperty(QLatin1String("h5write"), h5writeFunction);
	QScriptValue beepFunction = engine->newFunction(beep_func);
    global.setProperty(QLatin1String("beep"), beepFunction);

	registerVectorTypes(engine);
}


void RtSession::evaluate(const QString& program)
{
	//if (code=="quit")
	QScriptValue result = engine->evaluate(program);

    if (engine->hasUncaughtException()) {
        QStringList backtrace = engine->uncaughtExceptionBacktrace();
		QString msg(result.toString());
		msg += '\n';
		msg += backtrace.join("\n");
		emit stdErr(msg);
        //fprintf (stderr, "    %s\n%s\n\n", qPrintable(r.toString()),
        //         qPrintable(backtrace.join("\n")));
        return;
    }

	if (result.isError())
	{
		emit stdErr(result.toString());
		return;
	}

    if (! result.isUndefined())
        emit stdOut(result.toString());
}

bool RtSession::evaluate(const QScriptProgram& program, QString& ret)
{
	QScriptValue result = engine->evaluate(program);

    if (engine->hasUncaughtException()) 
	{
        QStringList backtrace = engine->uncaughtExceptionBacktrace();
		ret = result.toString();
		ret += '\n';
		ret += backtrace.join("\n");
        return false;
    }

	if (result.isError())
	{
		ret = result.toString();
		return false;
	}

    return true;
}

bool RtSession::canEvaluate(const QString& program)
{
	return engine->canEvaluate(program);
}

bool RtSession::wait(uint ms)
{
	wait_timer_->start(ms);
	wait_aborted_ = false;
	do 
		QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents, PROC_EVENTS_INTERVAL); 
	while (wait_timer_->isActive());
	return wait_aborted_;
}

void RtSession::abortEvaluation()
{
	//if (wait_timer_->isActive())
	//{
		wait_timer_->stop();
		wait_aborted_ = true;
	//}
	engine->abortEvaluation();
}

bool RtSession::isEvaluating() const
{
	return engine->isEvaluating();
}

QScriptValue RtSession::wait_func(QScriptContext *ctx, QScriptEngine *eng)
{
	static const char* usage = 
		"usage: wait(uint ms)";

	uint ms;
	
	// check arguments
	if (ctx->argumentCount()!=1)
	{
		return ctx->throwError(QScriptContext::SyntaxError,usage);
	}

	QScriptValue arg = ctx->argument(0);
	if (arg.isNumber()) ms = arg.toUInt32();
	else return ctx->throwError(QScriptContext::SyntaxError,usage);

	RtSession* s = qobject_cast<RtSession*>(eng->parent());
	//s->wait(ms);
	//if (s->abortController_->isRunning())
	if (s->wait(ms))
	{
		ctx->throwError(QScriptContext::UnknownError,"Wait Aborted.");
	}
	
    return eng->undefinedValue();
}

QScriptValue RtSession::quit_func(QScriptContext *ctx, QScriptEngine *eng)
{
    Q_UNUSED(ctx);

	RtSession* s = qobject_cast<RtSession*>(eng->parent());
	if (s) s->endSession();
	//emit endSession();
    return eng->undefinedValue();
}

QScriptValue RtSession::exec_func(QScriptContext *ctx, QScriptEngine *eng)
{
	static const char* usage = 
		"usage: exec(String script_name)";
	QString fname;

	// check arguments
	if (ctx->argumentCount()!=1)
	{
		return ctx->throwError(QScriptContext::SyntaxError,usage);
	}
	QScriptValue arg = ctx->argument(0);
	if (arg.isString()) fname = arg.toString();
	else
		return ctx->throwError(QScriptContext::SyntaxError,usage);

	QFile file(fname);
	if (file.open(QIODevice::Text | QIODevice::ReadOnly))
	{
		QTextStream qin(&file);
		QString program = qin.readAll();
		ctx->setActivationObject(ctx->parentContext()->activationObject());
		ctx->setThisObject(ctx->parentContext()->thisObject());
		return eng->evaluate(program,fname);
	}
	else return ctx->throwError(QScriptContext::ReferenceError,"File not found.");

    return eng->undefinedValue();
}

QScriptValue RtSession::print_func(QScriptContext *context, QScriptEngine *eng)
{
	static const char* usage = 
		"usage: print(String s)";
	QString str;

	// check arguments
	if (context->argumentCount()!=1)
	{
		return context->throwError(QScriptContext::SyntaxError,usage);
	}
	
	str = context->argument(0).toString();
	str += '\n';
	RtSession* s = qobject_cast<RtSession*>(eng->parent());
	if (s) s->stdOut(str);


    return eng->undefinedValue();
}

QScriptValue RtSession::textSave_func(QScriptContext *ctx, QScriptEngine *eng)
{
	static const char* usage = 
		"usage: textSave(String str, String filename)";

	// check arguments
	if (ctx->argumentCount()!=2)
	{
		return ctx->throwError(QScriptContext::SyntaxError,usage);
	}

	QString str, fname;
	QScriptValue arg;
	
	arg = ctx->argument(0);
	if (!arg.isString()) 
		return ctx->throwError(QScriptContext::SyntaxError,usage);
	else str = arg.toString();

	arg = ctx->argument(1);
	if (arg.isString()) fname = arg.toString();
	else
		return ctx->throwError(QScriptContext::SyntaxError,usage);

	QFile file(fname);
	if (file.open(QFile::WriteOnly | QFile::Truncate))
	{
		QTextStream qout(&file);
		qout << str;
	}
	else return ctx->throwError("File could not be opened.");

    return eng->undefinedValue();
}
QScriptValue RtSession::textLoad_func(QScriptContext *ctx, QScriptEngine *eng)
{
	static const char* usage = 
		"usage: String textLoad(String filename)";

	// check arguments
	if (ctx->argumentCount()!=1)
	{
		return ctx->throwError(QScriptContext::SyntaxError,usage);
	}

	QString fname, str;
	QScriptValue arg;
	
	arg = ctx->argument(0);
	if (!arg.isString()) 
		return ctx->throwError(QScriptContext::SyntaxError,usage);
	else fname = arg.toString();

	QFile file(fname);
	if (file.open(QFile::ReadOnly))
	{
		QTextStream qin(&file);
		str = qin.readAll();
	}
	else return ctx->throwError("File could not be opened.");

	return eng->toScriptValue(str);
}

QScriptValue RtSession::kill_func(QScriptContext *ctx, QScriptEngine *eng)
{
	static const char* usage = 
		"usage:\n"
		"  kill(RtObject obj)\n";
		//"  kill(String str), where str is a name matching string, e.g. \"*\" or \"dev.*\"";

	// check arguments
	if (ctx->argumentCount()!=1)
	{
		return ctx->throwError(QScriptContext::SyntaxError,usage);
	}

	QScriptValue arg = ctx->argument(0);
	RtObject* obj;
	if ( arg.isQObject() && (obj = qobject_cast<RtObject*>(arg.toQObject())) )
	{
		if (obj->canBeKilled()) 
		{
			obj->detach();
			delete obj;
		}
		return eng->undefinedValue();
	}

	if (arg.isArray())
	{
		// find all valid objects that can be killed
		QList< QPointer<RtObject> > toKill;
		quint32 n = arg.property("length").toUInt32();
		for(quint32 i=0; i<n; ++i)
		{
			RtObject* obj = qobject_cast<RtObject*>(arg.property(i).toQObject());
			if (obj && obj->canBeKilled()) toKill.push_back(obj);
		}

		// first detach
		foreach(QPointer<RtObject> obj, toKill) obj->detach();
		// then kill!
		foreach(QPointer<RtObject> obj, toKill) if (obj) delete obj;

		return eng->undefinedValue();
	}

	return ctx->throwError(QScriptContext::SyntaxError,usage);

}

QScriptValue RtSession::find_func(QScriptContext *ctx, QScriptEngine *eng)
{
	static const char* usage = 
		"usage: find(String)";

	// check arguments
	if (ctx->argumentCount()!=1)
	{
		return ctx->throwError(QScriptContext::SyntaxError,usage);
	}

	QScriptValue arg = ctx->argument(0);
	if ( arg.isString() )
	{
		QList<RtObject*> lst = RtObject::findByWildcard(arg.toString());
		QScriptValue ret = eng->newArray(lst.size());
		for(quint32 i=0; i<lst.size(); ++i)
		{
			ret.setProperty(i, eng->newQObject(lst.at(i)));
		}
		return ret;
	}
	else return ctx->throwError(QScriptContext::SyntaxError,usage);

}
QScriptValue RtSession::h5write_func(QScriptContext *ctx, QScriptEngine *e)
{
	static const char* usage = 
		"usage: h5write(String fileName, String comment)";

	// check arguments
	if (ctx->argumentCount()!=2)
	{
		return ctx->throwError(QScriptContext::SyntaxError,usage);
	}

	QString fname,comment;
	QScriptValue arg = ctx->argument(0);
	if ( arg.isString() ) fname=arg.toString();
	else return ctx->throwError(QScriptContext::SyntaxError,usage);
	arg = ctx->argument(1);
	if ( arg.isString() ) comment=arg.toString();
	else return ctx->throwError(QScriptContext::SyntaxError,usage);

	QString ret = root()->h5write(fname,comment);

	if (ret.isNull()) return e->undefinedValue();
	else return ctx->throwError(ret);
}

QScriptValue RtSession::beep_func(QScriptContext *ctx, QScriptEngine *e)
{
	static const char* usage = 
		"usage: beep()";

	// check arguments
	if (ctx->argumentCount()!=0)
	{
		return ctx->throwError(QScriptContext::SyntaxError,usage);
	}

        os::beep();

	return e->undefinedValue();
}

