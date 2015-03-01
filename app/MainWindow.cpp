#include <QtGui>

#include <QMdiArea>
#include <QFileDialog>
#include <QStatusBar>
#include <QAction>
#include <QMessageBox>
#include <QMenu>
#include <QMdiSubWindow>
#include <QMenuBar>
#include <QToolBar>

#include "MainWindow.h"
#include "ScriptEditor.h"
#include "ScriptConsole.h"
//#include "RtObjectInspector.h"
#include "RtRoot.h"

MainWindow::MainWindow(const QString &startupScript)
{
    mdiArea = new QMdiArea;
    mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setCentralWidget(mdiArea);
    connect(mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow *)),
            this, SLOT(updateMenus()));
    windowMapper = new QSignalMapper(this);
    connect(windowMapper, SIGNAL(mapped(QWidget *)),
            this, SLOT(setActiveSubWindow(QWidget *)));

    createActions();
    createMenus();
    createToolBars();
    createStatusBar();
	createDockers();
    updateMenus();

    readSettings();

    setWindowTitle(tr("RtLab"));

	RtObject::root()->setMainWindow(this);

    newConsole(startupScript);


}

void MainWindow::closeEvent(QCloseEvent *event)
{
    mdiArea->closeAllSubWindows();
    if (activeTextEdit()) {
        event->ignore();
    } else {
        writeSettings();
        event->accept();
    }
}

void MainWindow::newFile()
{
    ScriptEditor *child = createScriptEditor();
    child->newFile();
    child->show();
}

ScriptConsole *MainWindow::newConsole(const QString& startupScript)
{
	static int i = 1;
    ScriptConsole *child = createScriptConsole(startupScript);
    QString name = QString("Console:%1").arg(i++);
    child->setWindowTitle(name);
    child->setObjectName(name);
    child->show();

    return child;
}

void MainWindow::open()
{
	QFileDialog::Options options;
    options |= QFileDialog::DontUseNativeDialog;
    QString selectedFilter;
    QString fileName = QFileDialog::getOpenFileName(this,
                                "Open script file",
                                QString(),
                                tr("Script Files (*.js);;All Files (*)"),
                                &selectedFilter,
                                options);

    if (!fileName.isEmpty()) {
        QMdiSubWindow *existing = findEditor(fileName);
        if (existing) {
            mdiArea->setActiveSubWindow(existing);
            return;
        }

        ScriptEditor *child = createScriptEditor();
        if (child->loadFile(fileName)) {
            statusBar()->showMessage(tr("File loaded"), 2000);
            child->show();
        } else {
            child->close();
        }
    }
}

void MainWindow::save()
{
    if (activeScriptEditor() && activeScriptEditor()->save())
        statusBar()->showMessage(tr("File saved"), 2000);
}

void MainWindow::saveAs()
{
    if (activeScriptEditor() && activeScriptEditor()->saveAs())
        statusBar()->showMessage(tr("File saved"), 2000);
}

void MainWindow::cut()
{
    if (activeTextEdit())
        activeTextEdit()->cut();
}

void MainWindow::copy()
{
    if (activeTextEdit())
        activeTextEdit()->copy();
}

void MainWindow::paste()
{
    if (activeTextEdit())
        activeTextEdit()->paste();
}

void MainWindow::tabbedView()
{
	mdiArea->setViewMode(QMdiArea::TabbedView);
	mdiArea->setTabPosition(QTabWidget::South);
	mdiArea->setTabShape(QTabWidget::Rounded);
	updateWindowMenu();
}

void MainWindow::windowView()
{
	mdiArea->setViewMode(QMdiArea::SubWindowView);
	updateWindowMenu();
}

void MainWindow::about()
{
    QString msg = QString(
                "<h3>RtLab ver. %1</h3>"
                "<p>2015 - G. Apostolopoulos "
                "<a href=\"mailto:gapost@ipta.demokritos.gr\">gapost@ipta.demokritos.gr</a></p>"
                "<p>This program uses the following Open Source tools:</p>"
                "<ul>"
                    "<li>"
                    "The Qt C++ gui toolkit"
                    " <a href=\"http://www.trolltech.com/qt/\">www.trolltech.com/qt/</a>"
                    "</li>"
                "</ul>"
               ).arg(GIT_VERSION);

   QMessageBox::about(this, tr("About RtLab"),msg);
}

void MainWindow::updateMenus()
{
    bool hasTextEdit = (activeTextEdit() != 0);
	bool hasEditor = (activeScriptEditor() != 0);

    saveAct->setEnabled(hasEditor);
    saveAsAct->setEnabled(hasEditor);
    pasteAct->setEnabled(hasTextEdit);
    closeAct->setEnabled(hasTextEdit);
    closeAllAct->setEnabled(hasTextEdit);
    //tileAct->setEnabled(hasTextEdit);
    //cascadeAct->setEnabled(hasTextEdit);
    //nextAct->setEnabled(hasTextEdit);
    //previousAct->setEnabled(hasTextEdit);
    separatorAct->setVisible(hasTextEdit);

    bool hasSelection = (activeTextEdit() &&
                         activeTextEdit()->textCursor().hasSelection());
    cutAct->setEnabled(hasSelection);
    copyAct->setEnabled(hasSelection);
}

void MainWindow::updateWindowMenu()
{
    windowMenu->clear();
	foreach(QAction* act, toggleDockersActions) windowMenu->addAction(act);
    windowMenu->addSeparator();
    windowMenu->addAction(closeAct);
    windowMenu->addAction(closeAllAct);
    windowMenu->addSeparator();
	if (mdiArea->viewMode()==QMdiArea::SubWindowView)
	{
		windowMenu->addAction(tileAct);
		windowMenu->addAction(cascadeAct);
		windowMenu->addAction(tabbedViewAct);
	}
	else
	{
		windowMenu->addAction(windowViewAct);
	}
    windowMenu->addSeparator();
    windowMenu->addAction(nextAct);
    windowMenu->addAction(previousAct);
    windowMenu->addAction(separatorAct);

    QList<QMdiSubWindow *> windows = mdiArea->subWindowList();
    separatorAct->setVisible(!windows.isEmpty());

    for (int i = 0; i < windows.size(); ++i) {
        /*QPlainTextEdit *child = qobject_cast<QPlainTextEdit *>(windows.at(i)->widget());

        QString text;
        if (i < 9) {
            text = tr("&%1 %2").arg(i + 1)
                               .arg(child->windowTitle());
        } else {
            text = tr("%1 %2").arg(i + 1)
                              .arg(child->windowTitle());
        }*/
		QString text = windows.at(i)->windowTitle();
        QAction *action  = windowMenu->addAction(text);
        action->setCheckable(true);
        action ->setChecked(windows.at(i) == mdiArea->activeSubWindow());
        connect(action, SIGNAL(triggered()), windowMapper, SLOT(map()));
        windowMapper->setMapping(action, windows.at(i));
    }
}

ScriptEditor *MainWindow::createScriptEditor()
{
    ScriptEditor *child = new ScriptEditor;
    mdiArea->addSubWindow(child);

    connect(child, SIGNAL(copyAvailable(bool)),
            cutAct, SLOT(setEnabled(bool)));
    connect(child, SIGNAL(copyAvailable(bool)),
            copyAct, SLOT(setEnabled(bool)));

    return child;
}

ScriptConsole *MainWindow::createScriptConsole(const QString &startupScript)
{
    ScriptConsole *child = new ScriptConsole(startupScript);
    mdiArea->addSubWindow(child);

    connect(child, SIGNAL(copyAvailable(bool)),
            cutAct, SLOT(setEnabled(bool)));
    connect(child, SIGNAL(copyAvailable(bool)),
            copyAct, SLOT(setEnabled(bool)));

    return child;
}

void MainWindow::createActions()
{
    newAct = new QAction(QIcon(":/images/new.png"), tr("&New Script"), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("Create a new script file"));
    connect(newAct, SIGNAL(triggered()), this, SLOT(newFile()));

    newConsoleAct = new QAction(tr("New &Console"), this);
    newConsoleAct->setStatusTip(tr("Open new script console"));
    connect(newConsoleAct, SIGNAL(triggered()), this, SLOT(newConsole()));

	openAct = new QAction(QIcon(":/images/open.png"), tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

    saveAct = new QAction(QIcon(":/images/save.png"), tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save the document to disk"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

    saveAsAct = new QAction(tr("Save &As..."), this);
    saveAsAct->setShortcuts(QKeySequence::SaveAs);
    saveAsAct->setStatusTip(tr("Save the document under a new name"));
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

//! [0]
    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcut(tr("Ctrl+Q"));
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));
//! [0]

    cutAct = new QAction(QIcon(":/images/cut.png"), tr("Cu&t"), this);
    cutAct->setShortcuts(QKeySequence::Cut);
    cutAct->setStatusTip(tr("Cut the current selection's contents to the "
                            "clipboard"));
    connect(cutAct, SIGNAL(triggered()), this, SLOT(cut()));

    copyAct = new QAction(QIcon(":/images/copy.png"), tr("&Copy"), this);
    copyAct->setShortcuts(QKeySequence::Copy);
    copyAct->setStatusTip(tr("Copy the current selection's contents to the "
                             "clipboard"));
    connect(copyAct, SIGNAL(triggered()), this, SLOT(copy()));

    pasteAct = new QAction(QIcon(":/images/paste.png"), tr("&Paste"), this);
    pasteAct->setShortcuts(QKeySequence::Paste);
    pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current "
                              "selection"));
    connect(pasteAct, SIGNAL(triggered()), this, SLOT(paste()));

    closeAct = new QAction(tr("Cl&ose"), this);
    closeAct->setShortcut(tr("Ctrl+F4"));
    closeAct->setStatusTip(tr("Close the active window"));
    connect(closeAct, SIGNAL(triggered()),
            mdiArea, SLOT(closeActiveSubWindow()));

    closeAllAct = new QAction(tr("Close &All"), this);
    closeAllAct->setStatusTip(tr("Close all the windows"));
    connect(closeAllAct, SIGNAL(triggered()),
            mdiArea, SLOT(closeAllSubWindows()));

    tileAct = new QAction(tr("&Tile"), this);
    tileAct->setStatusTip(tr("Tile the windows"));
    connect(tileAct, SIGNAL(triggered()), mdiArea, SLOT(tileSubWindows()));

    cascadeAct = new QAction(tr("&Cascade"), this);
    cascadeAct->setStatusTip(tr("Cascade the windows"));
    connect(cascadeAct, SIGNAL(triggered()), mdiArea, SLOT(cascadeSubWindows()));

    tabbedViewAct = new QAction(tr("Tabbed &View"), this);
    tabbedViewAct->setStatusTip(tr("Show windows in overlapping tabs"));
    connect(tabbedViewAct, SIGNAL(triggered()), this, SLOT(tabbedView()));

    windowViewAct = new QAction(tr("Windows &View"), this);
    windowViewAct->setStatusTip(tr("Show separate windows"));
    connect(windowViewAct, SIGNAL(triggered()), this, SLOT(windowView()));

    nextAct = new QAction(tr("Ne&xt"), this);
    nextAct->setShortcuts(QKeySequence::NextChild);
    nextAct->setStatusTip(tr("Move the focus to the next window"));
    connect(nextAct, SIGNAL(triggered()),
            mdiArea, SLOT(activateNextSubWindow()));

    previousAct = new QAction(tr("Pre&vious"), this);
    previousAct->setShortcuts(QKeySequence::PreviousChild);
    previousAct->setStatusTip(tr("Move the focus to the previous "
                                 "window"));
    connect(previousAct, SIGNAL(triggered()),
            mdiArea, SLOT(activatePreviousSubWindow()));

    separatorAct = new QAction(this);
    separatorAct->setSeparator(true);

    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    aboutQtAct = new QAction(tr("About &Qt"), this);
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newAct);
    fileMenu->addAction(newConsoleAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(cutAct);
    editMenu->addAction(copyAct);
    editMenu->addAction(pasteAct);

    windowMenu = menuBar()->addMenu(tr("&Window"));
    updateWindowMenu();
    connect(windowMenu, SIGNAL(aboutToShow()), this, SLOT(updateWindowMenu()));

    menuBar()->addSeparator();

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);
}

void MainWindow::createToolBars()
{
    fileToolBar = addToolBar(tr("File"));
    fileToolBar->addAction(newAct);
    fileToolBar->addAction(openAct);
    fileToolBar->addAction(saveAct);

    editToolBar = addToolBar(tr("Edit"));
    editToolBar->addAction(cutAct);
    editToolBar->addAction(copyAct);
    editToolBar->addAction(pasteAct);
}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::readSettings()
{
    QSettings settings("INT-RP", "RtLab");
    QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("size", QSize(400, 400)).toSize();
    move(pos);
    resize(size);
}

void MainWindow::writeSettings()
{
    QSettings settings("INT-RP", "RtLab");
    settings.setValue("pos", pos());
    settings.setValue("size", size());
}

QPlainTextEdit* MainWindow::activeTextEdit()
{
    if (QMdiSubWindow *activeSubWindow = mdiArea->activeSubWindow())
        return qobject_cast<QPlainTextEdit *>(activeSubWindow->widget());
    return 0;
}

ScriptEditor* MainWindow::activeScriptEditor()
{
    if (QMdiSubWindow *activeSubWindow = mdiArea->activeSubWindow())
        return qobject_cast<ScriptEditor *>(activeSubWindow->widget());
    return 0;
}

QMdiSubWindow *MainWindow::findEditor(const QString &fileName)
{
    QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();

    foreach (QMdiSubWindow *window, mdiArea->subWindowList()) {
        ScriptEditor *editor = qobject_cast<ScriptEditor *>(window->widget());
        if (editor && (editor->currentFile() == canonicalFilePath))
            return window;
    }
    return 0;
}

void MainWindow::setActiveSubWindow(QWidget *window)
{
    if (!window)
        return;
    mdiArea->setActiveSubWindow(qobject_cast<QMdiSubWindow *>(window));
}

