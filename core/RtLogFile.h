#ifndef RTLOGFILE_H
#define RTLOGFILE_H

#include <QObject>

class QFile;

class RtLogFile : public QObject
{
    QFile* file_;

    bool timestamp;
    QChar delimiter;

public:


    RtLogFile(bool tm = true, QChar delim = QChar('\t'), QObject* parent = 0);
    ~RtLogFile();

    bool open(const QString& fname);
    bool isOpen();

    // returns a fully decorated name:
    // logFolder/simpleName_ddMMyyyy_x.log
    static QString getDecoratedName(const QString& simpleName);

    RtLogFile& operator<< (const QString& str);

};

#endif // RTLOGFILE_H
