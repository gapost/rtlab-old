#ifndef QDAQMX_H
#define QDAQMX_H

#include <QObject>
#include <QStringList>

struct daqmx_helper
{
    static QStringList ni_tokenize(const char* str);
    static void ni_error(const QObject* obj, int code);
};

class QDAQmx : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QStringList deviceNames READ deviceNames)
public:
    explicit QDAQmx(QObject *parent = 0);

    QStringList deviceNames() const;

signals:

public slots:

};

#endif // QDAQMX_H
