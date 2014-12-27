#include <QCoreApplication>
#include <iostream>

#include "qdaqmx.h"

using namespace std;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QDAQmx mx;

    foreach(QString d, mx.deviceNames())
        std::cout << d.toAscii().constData() << endl;

    return a.exec();
}
