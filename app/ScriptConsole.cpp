#include <QCoreApplication>
#include <QKeyEvent>
#include <QMessageBox>
#include <QScriptEngine>
#include <QScriptValueIterator>
#include <QTimer>

#include "ScriptConsole.h"

#include "RtSession.h"

ScriptConsole::ScriptConsole(const QString& startupScript)
{

	setTabStopWidth ( 40 );

    session = new RtSession("console",this);

    setWindowTitle(QString("Console:%1").arg(session->index()));
		
	connect(session,SIGNAL(stdOut(const QString&)),this,SLOT(stdOut(const QString&)));
	connect(session,SIGNAL(stdErr(const QString&)),this,SLOT(stdErr(const QString&)));
    connect(session,SIGNAL(endSession()),this,SLOT(close()),Qt::QueuedConnection);


    if (!startupScript.isEmpty())
    {
        execCode_ = startupScript;
        QTimer::singleShot(1000,this,SLOT(deferedEvaluate()));
    }

}

void ScriptConsole::exec(const QString& code)
{
    session->print(QString("Running startup script %1 ...").arg(code));
    session->exec(code);
}
bool ScriptConsole::canEvaluate(const QString& code)
{
	return session->canEvaluate(code);
}

void ScriptConsole::closeEvent ( QCloseEvent * e )
{
	bool ok = true;

	if (session->isEvaluating())
	{
		QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, windowTitle(),
                     tr("A script is currently executing.\n"
					    "Closing this console will terminate execution.\n"
                        "Do you want to close the console?"),
						QMessageBox::Close | QMessageBox::Cancel );
		ok = ret==QMessageBox::Close;
		if (ok) 
		{
			session->abortEvaluation();
			QCoreApplication::postEvent(parentWidget(),new QCloseEvent());
		}
		e->ignore();
	}
	else
	{
		parentWidget()->close();
		e->accept();	
	}

	/*if (ok)
	{
		QWidget* w = parentWidget();
		w->close();
		e->accept();
	}
	else e->ignore();*/
}

void ScriptConsole::keyPressEvent (QKeyEvent * e)
{
	if (e->modifiers() & Qt::ControlModifier)
	{
		int k = e->key();
        //if (k==Qt::Key_Cancel || k==Qt::Key_Pause) // did not work on Linux. GA 24/3/2015
        if (k==Qt::Key_Q) // Ctrl-A aborts
		{
			session->abortEvaluation();
			e->accept();
			return;
		}
	}
	
	QConsoleWidget::keyPressEvent(e);
}

QStringList ScriptConsole::introspection(const QString& lookup)
{
    // list of found tokens
    QStringList properties;

    if (lookup.isEmpty()) return properties;

    QScriptEngine* eng = session->getEngine();
    QScriptValue scriptObj = eng->evaluate(lookup);

    // if the engine cannot recognize the variable return
    if (eng->hasUncaughtException()) return properties;

     // if a QObject add the named children
    if (scriptObj.isQObject())
    {
        QObject* obj = scriptObj.toQObject();
        foreach(QObject* ch, obj->children())
        {
            QString name = ch->objectName();
            if (!name.isEmpty())
                properties << name;
        }

    }

    // add the script properties
    {
        QScriptValueIterator it(scriptObj);
        while (it.hasNext()) {
             it.next();
             if (it.flags() & QScriptValue::SkipInEnumeration)
                 continue;
             properties << it.name();
         }
    }

    return properties;

}



