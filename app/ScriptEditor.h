#ifndef _SCRIPTEDITOR_H_
#define _SCRIPTEDITOR_H_

#include <QPlainTextEdit>

class ScriptEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    ScriptEditor();

    void newFile();
    bool loadFile(const QString &fileName);
    bool save();
    bool saveAs();
    bool saveFile(const QString &fileName);
    QString userFriendlyCurrentFile();
    QString currentFile() { return curFile; }

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void documentWasModified();

private:
    bool maybeSave();
    void setCurrentFile(const QString &fileName);
    QString strippedName(const QString &fullFileName);

    QString curFile;
    bool isUntitled;
};

#endif
