#include "daqmx.h"

#include <QThread>
#include <QMutex>
#include <QWaitCondition>

// functor encapsulating a DAQmx Base function
struct mxFunc
{
    TaskHandle& h;
    int32 retval;
    QString& errmsg;
    virtual void operator()() = 0;
    void checkError()
    {
        if (retval!=0) errmsg = daqmx::getErrorMessage(0);
    }

    mxFunc(TaskHandle& ah, QString& aerrmsg) : h(ah), errmsg(aerrmsg)
    {}
};

// This QThread handles all DAQmx Base calls
class daqmx_thread : public QThread
{
    QMutex mtx;
    QWaitCondition waitRequest, waitCompletion;
    mxFunc* mxf;
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
            mxf->operator()();

            // unblock the caller
            mtx.lock();
            waitCompletion.wakeOne();
            mtx.unlock();
        }
    }
    // this function is called from client threads
    // to perform a DAQmx function
    int call_daqmx(mxFunc* g)
    {
        if (!isRunning())
        {
            g->retval = -1;
            g->errmsg = "DAQmx Base thread not running";
            return -1;
        }

        // request the server to run the command
        mtx.lock();
        mxf = g;
        waitRequest.wakeOne(); // wake the server
        mtx.unlock();

        // wait for completion
        mtx.lock();
        waitCompletion.wait(&mtx);
        mtx.unlock();

        // return daqmx retval
        return g->retval;
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
    Q_UNUSED(c);
    int len = DAQmxBaseGetExtendedErrorInfo(NULL,0);
    if (len) {
        QByteArray ar(len,char(0));
        DAQmxBaseGetExtendedErrorInfo(ar.data(), len);
        return QString(ar);
    }
    else return QString();
}

struct mxCreateTask : public mxFunc
{
    QString name;
    virtual void operator()()
    {
        retval = DAQmxBaseCreateTask(name.toLatin1().constData(),&h);
        checkError();
    }

    mxCreateTask(const QString& aname, TaskHandle& h, QString& msg) : mxFunc(h,msg), name(aname)
    {}
};
int daqmx::createTask(const QString &name, TaskHandle &h, QString &errmsg)
{
    mxCreateTask mxf(name, h, errmsg);
    daqmx::thread_->call_daqmx(&mxf);
    return mxf.retval;
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

