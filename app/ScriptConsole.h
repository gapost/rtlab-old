#ifndef _SCRIPTCONSOLE_H_
#define _SCRIPTCONSOLE_H_

#include "QConsoleWidget.h"

class RtSession;

class ScriptConsole : public QConsoleWidget
{
    Q_OBJECT

	RtSession* session;
    QString execCode_;
public:
    ScriptConsole(const QString& startupScript = QString());

public slots:
    void evaluate(const QString& code)
    {
        exec(code);
    }

protected slots:
    void deferedEvaluate()
    {
        exec(execCode_);
    }

protected:
	void exec(const QString& code);
	bool canEvaluate(const QString& code);

	virtual void closeEvent ( QCloseEvent * event );

  //! derived key press event
  virtual void keyPressEvent (QKeyEvent * e);

    QStringList introspection(const QString& lookup);

};

#endif

