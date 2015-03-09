#include "RtSession.h"
#include "RtRoot.h"
#include "RtTypes.h"
#include "RtMainWindow.h"
#include "os_utils.h"

#include <QDebug>
#include <QScriptEngine>
#include <QFile>
#include <QTextStream>
#include <QTimer>
#include <QCoreApplication>
#include <QPointer>
#include <QStringList>
#include <QScriptValueIterator>
#include <QDir>
#include <QProcess>
#include <QStatusBar>


#define PROC_EVENTS_INTERVAL 250

RtSession::RtSession(const QString& name, RtObject* parent) : RtObject(name,"session",parent)
{
    engine_ = new QScriptEngine(this);
    engine_->setProcessEventsInterval ( PROC_EVENTS_INTERVAL );
    wait_timer_ = new QTimer(this);
    wait_timer_->setSingleShot(true);

    // register RtObject* with the engine (and RtObjectList)
    registerRtObjectStar(engine_);

    // register basic types with the engine
    registerVectorTypes(engine_);

    QScriptValue self = engine_->newQObject(
        this, QScriptEngine::QtOwnership,
        QScriptEngine::ExcludeSuperClassContents);

    QScriptValue globObj = engine_->globalObject();
    {
        QScriptValueIterator it(globObj);
        while (it.hasNext()) {
            it.next();
            self.setProperty(it.scriptName(), it.value(), it.flags());
        }
    }

    engine_->setGlobalObject(self);

    engine_->collectGarbage();

    const QObjectList& chlist = root_.children();

    foreach(QObject* o, chlist)
	{
		RtObject* rto = qobject_cast<RtObject*>(o);
		if (rto) 
		{
            QScriptValue v = engine_->newQObject(rto,
				QScriptEngine::QtOwnership,
                QScriptEngine::ExcludeDeleteLater | QScriptEngine::AutoCreateDynamicProperties
				);

            engine_->globalObject().setProperty(
				rto->objectName(),
				v,
				QScriptValue::Undeletable
				);
		}
	}

    QScriptValue killFunction = engine_->newFunction(kill_func);
    engine_->globalObject().setProperty(QLatin1String("kill"), killFunction);

}

RtSession::~RtSession( void)
{
	Q_ASSERT(!isEvaluating());
}

void RtSession::evaluate(const QString& program)
{
	//if (code=="quit")
    QScriptValue result = engine_->evaluate(program);

    if (engine_->hasUncaughtException()) {
        QStringList backtrace = engine_->uncaughtExceptionBacktrace();
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
    QScriptValue result = engine_->evaluate(program);

    if (engine_->hasUncaughtException())
	{
        QStringList backtrace = engine_->uncaughtExceptionBacktrace();
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
    return engine_->canEvaluate(program);
}

void RtSession::wait(uint ms)
{
	wait_timer_->start(ms);
	wait_aborted_ = false;

	do 
		QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents, PROC_EVENTS_INTERVAL); 
	while (wait_timer_->isActive());

    if (wait_aborted_)
        engine_->currentContext()->throwError(QScriptContext::UnknownError,"Wait Aborted.");
}

void RtSession::abortEvaluation()
{
    wait_timer_->stop();
    wait_aborted_ = true;
    engine_->abortEvaluation();
}

bool RtSession::isEvaluating() const
{
    return engine_->isEvaluating();
}

void RtSession::exec(const QString &fname)
{
    QFile file(fname);
    if (file.open(QIODevice::Text | QIODevice::ReadOnly))
    {
        QTextStream qin(&file);
        QString program = qin.readAll();

        QScriptContext* ctx = engine_->currentContext();

        ctx->setActivationObject(ctx->parentContext()->activationObject());
        ctx->setThisObject(ctx->parentContext()->thisObject());


        engine_->evaluate(program,fname);
    }
    else engine_->currentContext()->throwError(QScriptContext::ReferenceError,"File not found.");
}
void RtSession::print(const QString& str)
{
    stdOut(str);
    if (!str.endsWith('\n')) stdOut(QString('\n'));
}
void RtSession::textSave(const QString &str, const QString &fname)
{
    QFile file(fname);
    if (file.open(QFile::WriteOnly | QFile::Truncate))
    {
        QTextStream qout(&file);
        qout << str;
    }
    else engine_->currentContext()->throwError("File could not be opened.");
}

QString RtSession::textLoad(const QString &fname)
{
    QString str;
    QFile file(fname);
    if (file.open(QFile::ReadOnly))
    {
        QTextStream qin(&file);
        str = qin.readAll();
    }
    else engine_->currentContext()->throwError("File could not be opened.");
    return str;
}

void RtSession::quit()
{
    emit endSession();
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

RtObjectList RtSession::find(const QString& wc)
{
    return RtObject::findByWildcard(wc);
}
void RtSession::h5write(const QString& fname, const QString& comment)
{
    QString ret = root()->h5write(fname,comment);
    if (!ret.isNull())  engine_->currentContext()->throwError(ret);
}
void RtSession::beep()
{
    os::beep();
}
QString RtSession::pwd()
{
    return QDir::currentPath();
}

bool RtSession::cd(const QString &path)
{
    QDir dir = QDir::current();
    bool ret = dir.cd(path);
    if (ret) QDir::setCurrent(dir.path());
    return ret;
}
QStringList RtSession::dir(const QStringList& filters)
{
    return QDir::current().entryList(filters);
}
QStringList RtSession::dir(const QString& filter)
{
    return dir(QStringList(filter));
}
bool RtSession::isDir(const QString& name)
{
    QFileInfo fi(name);
    return fi.isDir();
}
void RtSession::saveWindowState(const QString& fname)
{
    QByteArray ba_geometry = root()->mainWindow()->saveGeometry();
    QByteArray ba_state = root()->mainWindow()->saveState();
    QFile file(fname);
    if (file.open(QFile::WriteOnly | QFile::Truncate))
    {
        QDataStream qout(&file);
        qout << ba_geometry << ba_state;
    }
    else engine_->currentContext()->throwError("File could not be opened.");
}

void RtSession::restoreWindowState(const QString& fname)
{
    QFile file(fname);
    QByteArray ba_geometry,ba_state;
    if (file.open(QFile::ReadOnly))
    {
        QDataStream qin(&file);
        qin >> ba_geometry >> ba_state;
        root()->mainWindow()->restoreGeometry(ba_geometry);
        root()->mainWindow()->restoreState(ba_state);
    }
    // if file could not open do nothing
    //else engine_->currentContext()->throwError("File could not be opened.");
}

QString RtSession::system(const QString &comm)
{
    QProcess p;
    //comm.split(QChar(' '),QString::SkipEmptyParts);
    p.start(comm);
    /*bool ret = */p.waitForFinished(1000);
    QByteArray pout = p.readAllStandardOutput();
    QByteArray perr = p.readAllStandardError();
    QByteArray pall = p.readAll();
    return QString(pout);
}

void RtSession::status(const QString &msg, int tmo)
{
    RtMainWindow* wnd = root()->mainWindow();
    wnd->statusBar()->showMessage(msg,tmo);
}


