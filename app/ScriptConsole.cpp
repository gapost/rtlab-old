#include <QCoreApplication>
#include <QKeyEvent>
#include <QMessageBox>
#include <QScriptEngine>
#include <QScriptValueIterator>

#include "ScriptConsole.h"

#include "RtSession.h"

ScriptConsole::ScriptConsole()
{

	setTabStopWidth ( 40 );

	session = new RtSession("s1");
		
	connect(session,SIGNAL(stdOut(const QString&)),this,SLOT(stdOut(const QString&)));
	connect(session,SIGNAL(stdErr(const QString&)),this,SLOT(stdErr(const QString&)));
	connect(session,SIGNAL(endSession()),this,SLOT(close()));

}

void ScriptConsole::exec(const QString& code)
{
	session->evaluate(code);
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
		if (k==Qt::Key_Cancel || k==Qt::Key_Pause)
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

    if (scriptObj.isUndefined()) return properties;

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



