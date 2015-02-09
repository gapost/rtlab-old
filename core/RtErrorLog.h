#ifndef RTERRORLOG_H
#define RTERRORLOG_H

#include <QWidget>

class QTableWidget;
class QPushButton;

class RtErrorLog : public QWidget
{
    Q_OBJECT
public:
    RtErrorLog(QWidget* p = 0);
    virtual ~RtErrorLog(void);

protected:
    virtual void customEvent ( QEvent *);

    Q_SLOT void clear();

    //QTreeWidget* treeWidget;
    QTableWidget* tableWidget;
    QPushButton* clearButton;
};

#endif // RTERRORLOG_H
