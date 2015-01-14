#include <QApplication>
#include <QFile>
#include <QTextStream>

#include "MainWindow.h"
#include "RtDev.h"

//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>


int main(int argc, char *argv[])
{
	//_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );

    Q_INIT_RESOURCE(rtlab);

    QApplication app(argc, argv);

	RtAcquisition* dev = new RtAcquisition("dev",(RtObject*)RtObject::root());

    QString startup;
    if (argc>1) {
        QString fn(argv[1]);
        QFile fin(fn);
        if (fin.open(QFile::ReadOnly)) {
            QTextStream qin(&fin);
            startup = qin.readAll();
        }
    }



    MainWindow mainWin(startup);
    mainWin.show();
    return app.exec();
}
