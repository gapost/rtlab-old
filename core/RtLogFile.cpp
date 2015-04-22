#include "RtLogFile.h"

#include <QFile>
#include <QTextStream>
#include <QDate>
#include <QDateTime>
#include "RtObject.h"
#include "RtRoot.h"

RtLogFile::RtLogFile(bool tm, QChar delim, QObject *parent) : QObject(parent),
    file_(0), timestamp(tm), delimiter(delim)
{

}

RtLogFile::~RtLogFile()
{
    if (file_) delete file_;
}

bool RtLogFile::open(const QString &fname)
{
    if (file_) delete file_;
    file_ = new QFile(fname);
    QFile::OpenMode mode = file_->exists() ? QFile::Append : QFile::Truncate;
    mode |= QIODevice::WriteOnly;
    return file_->open(mode);
}

bool RtLogFile::isOpen()
{
    if (file_) return file_->isOpen();
    return false;
}

QString RtLogFile::getDecoratedName(const QString &simpleName)
{
    QString fname(simpleName), fullName;
    fname += QDate::currentDate().toString("_ddMMyyyy");
    fullName = RtObject::root()->logDir() + QChar('/') + fname + QString(".log");
    /* int idx = 0;
    while (QFile::exists(fullName))
    {
        idx++;
        fullName = RtObject::root()->logDir() + fname + QString("_%1.log").arg(idx);
    } */
    return fullName;
}

RtLogFile& RtLogFile::operator <<(const QString& str)
{
    if (!isOpen()) return *this;

    QTextStream stream(file_);

    if (timestamp) {
        QDateTime cdt = QDateTime::currentDateTime();
        stream << cdt.toString("dd.MM.yyyy") << delimiter
               << cdt.toString("hh:mm:ss") << delimiter;
    }

    stream << str << '\n';

    return *this;
}
