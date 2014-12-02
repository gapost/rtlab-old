#include <QtGui>

#include "ScriptEditor.h"

ScriptEditor::ScriptEditor()
{
    setAttribute(Qt::WA_DeleteOnClose);
	setTabStopWidth ( 40 );

    isUntitled = true;

	QTextCharFormat fmt = currentCharFormat();
	fmt.setFontFamily("Courier New");
	fmt.setFontFixedPitch ( true );
	fmt.setFontPointSize(10);
	setCurrentCharFormat(fmt);

}

void ScriptEditor::newFile()
{
    static int sequenceNumber = 1;

    isUntitled = true;
    curFile = tr("document%1.txt").arg(sequenceNumber++);
    setWindowTitle(curFile + "[*]");

    connect(document(), SIGNAL(contentsChanged()),
            this, SLOT(documentWasModified()));
}

bool ScriptEditor::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Script Editor"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return false;
    }

    QTextStream in(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    setPlainText(in.readAll());
    QApplication::restoreOverrideCursor();

    setCurrentFile(fileName);

    connect(document(), SIGNAL(contentsChanged()),
            this, SLOT(documentWasModified()));

    return true;
}

bool ScriptEditor::save()
{
    if (isUntitled) {
        return saveAs();
    } else {
        return saveFile(curFile);
    }
}

bool ScriptEditor::saveAs()
{
	QFileDialog::Options options;
    options |= QFileDialog::DontUseNativeDialog;
    QString selectedFilter;
    QString fileName = QFileDialog::getSaveFileName(this,
                                "Save script file as",
                                curFile,
                                tr("Script Files (*.js);;All Files (*)"),
                                &selectedFilter,
                                options);

    if (fileName.isEmpty())
        return false;

    return saveFile(fileName);
}

bool ScriptEditor::saveFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Script Editor"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return false;
    }

    QTextStream out(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    out << toPlainText();
    QApplication::restoreOverrideCursor();

    setCurrentFile(fileName);
    return true;
}

QString ScriptEditor::userFriendlyCurrentFile()
{
    return strippedName(curFile);
}

void ScriptEditor::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        event->accept();
    } else {
        event->ignore();
    }
}

void ScriptEditor::documentWasModified()
{
    setWindowModified(document()->isModified());
}

bool ScriptEditor::maybeSave()
{
    if (document()->isModified()) {
	QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, tr("Script Editor"),
                     tr("'%1' has been modified.\n"
                        "Do you want to save your changes?")
                     .arg(userFriendlyCurrentFile()),
                     QMessageBox::Save | QMessageBox::Discard
		     | QMessageBox::Cancel);
        if (ret == QMessageBox::Save)
            return save();
        else if (ret == QMessageBox::Cancel)
            return false;
    }
    return true;
}

void ScriptEditor::setCurrentFile(const QString &fileName)
{
    curFile = QFileInfo(fileName).canonicalFilePath();
    isUntitled = false;
    document()->setModified(false);
    setWindowModified(false);
    setWindowTitle(userFriendlyCurrentFile() + "[*]");
}

QString ScriptEditor::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}
