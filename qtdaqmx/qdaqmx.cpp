#include "qdaqmx.h"

#include <QDebug>

#include <NIDAQmx.h>

#define BUFFLEN 4096

// parse comma separated string from NI function
QStringList daqmx_helper::ni_tokenize(const char* str)
{
    QString S(str);
    QStringList tokens = S.split(',');
    QStringList::iterator i;
    for(i = tokens.begin(); i!=tokens.end(); i++)
        (*i) = (*i).trimmed();
    return tokens;
}

void daqmx_helper::ni_error(const QObject *obj, int code)
{
    qDebug() << "NI DAQmx error " << code << " on object " << obj;
}

QDAQmx::QDAQmx(QObject *parent) :
    QObject(parent)
{
}

QStringList QDAQmx::deviceNames() const
{
    const int len = BUFFLEN;
    char buff[len];

    QStringList devs;

    int ret = DAQmxGetSysDevNames(buff,len);

    if (ret==0) devs = daqmx_helper::ni_tokenize(buff);
    else daqmx_helper::ni_error(this,ret);

    return devs;
}


