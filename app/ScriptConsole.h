#ifndef _SCRIPTCONSOLE_H_
#define _SCRIPTCONSOLE_H_

#include "QConsoleWidget.h"

class RtSession;

class ScriptConsole : public QConsoleWidget
{
	RtSession* session;
public:
	ScriptConsole(void);
protected:
	void exec(const QString& code);
	bool canEvaluate(const QString& code);

	virtual void closeEvent ( QCloseEvent * event );

  //! derived key press event
  virtual void keyPressEvent (QKeyEvent * e);

    QStringList introspection(const QString& lookup);

};

#endif

