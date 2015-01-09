#include "daqmx.h"

#include <QThread>
#include <QMutex>
#include <QWaitCondition>

// functor encapsulating a DAQmx command
struct mxfunc
{
    TaskHandle h;
    int32 retval;
    QString errmsg;
    virtual void operator()() = 0;
    mxfunc(TaskHandle ah) : h(ah)
    {}
};

// This QThread handles all DAQmx Base calls
class daqmx_thread : public QThread
{
    QMutex mtx;
    QWaitCondition waitRequest, waitCompletion;
    mxfunc* mxf;
    bool quit_;
public:
    daqmx_thread() : QThread()
    {}
    // overload QThread::start to ensure that
    // the thread is waiting for input
    void start()
    {
        mtx.lock();
        QThread::start();
        waitCompletion.wait(&mtx); // wait for the thread to begin
        mtx.unlock();
    }
    // main thread loop
    void run()
    {
        quit_ = false;

        // unblock the start() function
        mtx.lock();
        waitCompletion.wakeOne();
        mtx.unlock();

        // begin the loop
        while (1)
        {
            // wait for request
            mtx.lock();
            waitRequest.wait(&mtx);
            mtx.unlock();

            // if requested to quit ...
            if (quit_) break;

            // call the DAQmx function
            f->operator()();

            // unblock the caller
            mtx.lock();
            waitCompletion.wakeOne();
            mtx.unlock();
        }
    }
    // this function is called from client threads
    // to perform a DAQmx function
    void call_daqmx(mxfunc* g)
    {
        // request the server to run the command
        mtx.lock();
        f = g;
        waitRequest.wakeOne(); // wake the server
        mtx.unlock();

        // wait for completion
        mtx.lock();
        waitCompletion.wait(&mtx);
        mtx.unlock();
    }
    void quit()
    {
        if (!isRunning()) return;
        mtx.lock();
        quit_ = true;
        waitRequest.wakeOne();
        mtx.unlock();
        wait();
    }
};


daqmx_thread* daqmx::thread_ = 0;

void daqmx::init()
{
    if (thread_ == 0) {
        thread_ = new daqmx_thread();
        thread_->start();
    }
}
QString daqmx::getErrorMessage(int c)
{
    int len = DAQmxGetErrorString(c, NULL, 0);
    if (len) {
        QByteArray ar(len,char(0));
        DAQmxGetErrorString(c, ar.data(), len);
        return QString(ar);
    }
    else return QString();
}
int daqmx::createTask(const QString &name, TaskHandle &h, QString &errmsg)
{
    int ret = DAQmxCreateTask(name.toLatin1().constData(),&h);
    if (ret!=0) errmsg = getErrorMessage(ret);
    return ret;
}
int daqmx::clearTask(TaskHandle h, QString &errmsg)
{
    int ret = DAQmxClearTask(h);
    if (ret!=0) errmsg = getErrorMessage(ret);
    return ret;
}
int daqmx::startTask(TaskHandle h, QString &errmsg)
{
    int ret = DAQmxStartTask(h);
    if (ret!=0) errmsg = getErrorMessage(ret);
    return ret;
}
int daqmx::stopTask(TaskHandle h, QString &errmsg)
{
    int ret = DAQmxStopTask(h);
    if (ret!=0) errmsg = getErrorMessage(ret);
    return ret;
}
int daqmx::createAIChan(TaskHandle h, const QString &name, int32 config, double minVal, double maxVal, QString &errmsg)
{
    int ret = DAQmxCreateAIVoltageChan(h,
                                   name.toLatin1().constData(),
                                   NULL,
                                   config, minVal, maxVal, DAQmx_Val_Volts, NULL);
    if (ret!=0) errmsg = getErrorMessage(ret);
    return ret;
}
int daqmx::createAOChan(TaskHandle h, const QString &name, double minVal, double maxVal, QString &errmsg)
{
    int ret = DAQmxCreateAOVoltageChan(h,
                                   name.toLatin1().constData(),
                                   NULL,
                                   minVal, maxVal, DAQmx_Val_Volts, NULL);
    if (ret!=0) errmsg = getErrorMessage(ret);
    return ret;
}
int daqmx::createDIChan(TaskHandle h, const QString& name, QString& errmsg)
{
    int ret = DAQmxCreateDIChan(h,name.toLatin1().constData(),NULL, DAQmx_Val_ChanForAllLines);
    if (ret!=0) errmsg = getErrorMessage(ret);
    return ret;
}
int daqmx::createDOChan(TaskHandle h, const QString& name, QString& errmsg)
{
    int ret = DAQmxCreateDOChan(h,name.toLatin1().constData(),NULL, DAQmx_Val_ChanForAllLines);
    if (ret!=0) errmsg = getErrorMessage(ret);
    return ret;
}
int daqmx::createCICountEdgesChan(TaskHandle h, const QString& name, int32 edge, int32 countDir, QString& errmsg)
{
    int ret = DAQmxCreateCICountEdgesChan(h, name.toLatin1().constData(),
                                      NULL, edge, 0, countDir);
    if (ret!=0) errmsg = getErrorMessage(ret);
    return ret;
}
int daqmx::readAnalog(TaskHandle h, double timo, double *buff, int32 &read, QString &msg)
{
    bool32 _r;
    int ret = DAQmxReadAnalogF64(h,1,timo,DAQmx_Val_GroupByChannel,
                                 buff,1,&read,&_r);
    if (ret!=0) msg = getErrorMessage(ret);
    return ret;
}
int daqmx::readDigital(TaskHandle h, double timo, uInt32* buff, int32 &read, QString &msg)
{
    bool32 _r;
    int ret = DAQmxReadDigitalU32(h,1,timo,DAQmx_Val_GroupByChannel,
                                 buff,1,&read,&_r);
    if (ret!=0) msg = getErrorMessage(ret);
    return ret;
}
int daqmx::readCounter(TaskHandle h, double timo, uInt32* buff, int32 &read, QString &msg)
{
    bool32 _r;
    int ret = DAQmxReadCounterU32(h,1,timo,
                                 buff,1,&read,&_r);
    if (ret!=0) msg = getErrorMessage(ret);
    return ret;
}
int daqmx::writeAnalog(TaskHandle h, double timo, const double *buff, int32 &written, QString &msg)
{
    bool32 _r;
    int ret = DAQmxWriteAnalogF64(h,1,FALSE,timo,DAQmx_Val_GroupByChannel,
                                 buff,&written,&_r);
    if (ret!=0) msg = getErrorMessage(ret);
    return ret;
}
int daqmx::writeDigital(TaskHandle h, double timo, const uInt32* buff, int32 &written, QString &msg)
{
    bool32 _r;
    int ret = DAQmxWriteDigitalU32(h,1,FALSE,timo,DAQmx_Val_GroupByChannel,
                                 buff,&written,&_r);
    if (ret!=0) msg = getErrorMessage(ret);
    return ret;
}

