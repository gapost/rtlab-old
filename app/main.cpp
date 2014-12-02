#include <QApplication>

#include "mainwindow.h"
#include "RtDev.h"

//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>


int main(int argc, char *argv[])
{
	//_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );

    Q_INIT_RESOURCE(rtlab);

	RtAcquisition* dev = new RtAcquisition("dev",(RtObject*)RtObject::root());

    QApplication app(argc, argv);
    MainWindow mainWin;
    mainWin.show();
    return app.exec();
}
