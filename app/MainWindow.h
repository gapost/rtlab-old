#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

#include "RtMainWindow.h"

class ScriptEditor;
class ScriptConsole;
class QAction;
class QMenu;
class QMdiSubWindow;
class QSignalMapper;
class QPlainTextEdit;

class MainWindow : public RtMainWindow
{
    Q_OBJECT

public:
    MainWindow();

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void newFile();
    void newConsole();
    void open();
    void save();
    void saveAs();
    void cut();
    void copy();
    void paste();
    void about();
    void updateMenus();
    void updateWindowMenu();
	void tabbedView();
	void windowView();
    ScriptEditor *createScriptEditor();
    ScriptConsole *createScriptConsole();
    void setActiveSubWindow(QWidget *window);

private:
    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void readSettings();
    void writeSettings();
	QPlainTextEdit* activeTextEdit();
	ScriptEditor* activeScriptEditor();
    QMdiSubWindow *findEditor(const QString &fileName);

    QSignalMapper *windowMapper;

    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *windowMenu;
    QMenu *helpMenu;
    QToolBar *fileToolBar;
    QToolBar *editToolBar;
    QAction *newAct;
    QAction *newConsoleAct;
    QAction *openAct;
    QAction *saveAct;
    QAction *saveAsAct;
    QAction *exitAct;
    QAction *cutAct;
    QAction *copyAct;
    QAction *pasteAct;
    QAction *closeAct;
    QAction *closeAllAct;
    QAction *tileAct;
    QAction *cascadeAct;
    QAction *tabbedViewAct;
    QAction *windowViewAct;
    QAction *nextAct;
    QAction *previousAct;
    QAction *separatorAct;
    QAction *aboutAct;
    QAction *aboutQtAct;
};

#endif

