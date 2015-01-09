#ifndef DAQMX_H
#define DAQMX_H

#include <QString>

#ifdef USE_DAQMX_BASE
#include <NIDAQmxBase.h>
#else
#include <NIDAQmx.h>
#endif

class daqmx_thread;

struct daqmx
{
    static daqmx_thread* thread_;

    static void init();

    static QString getErrorMessage(int c);
    static int createTask(const QString& name, TaskHandle& h, QString& errmsg);
    static int clearTask(TaskHandle h, QString& errmsg);
    static int startTask(TaskHandle h, QString& errmsg);
    static int stopTask(TaskHandle h, QString& errmsg);
    static int createAIChan(TaskHandle h, const QString& name, int32 config,
                            double minVal, double maxVal, QString& errmsg);
    static int createAOChan(TaskHandle h, const QString& name,
                            double minVal, double maxVal, QString& errmsg);
    static int createDIChan(TaskHandle h, const QString& name, QString& errmsg);
    static int createDOChan(TaskHandle h, const QString& name, QString& errmsg);
    static int createCICountEdgesChan(TaskHandle h, const QString& name, int32 edge, int32 countDir, QString& errmsg);
    static int readAnalog(TaskHandle h, double timo, double* buff, int32& read, QString& msg);
    static int readDigital(TaskHandle h, double timo, uInt32* buff, int32& read, QString& msg);
    static int readCounter(TaskHandle h, double timo, uInt32* buff, int32& read, QString& msg);
    static int writeAnalog(TaskHandle h, double timo, const double* buff, int32& written, QString& msg);
    static int writeDigital(TaskHandle h, double timo, const uInt32* buff, int32& written, QString& msg);
};

#endif // DAQMX_HELPER_H
