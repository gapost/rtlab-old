#include "daqmx.h"

#include <QThread>
#include <QMutex>
#include <QWaitCondition>

// functor encapsulating a DAQmx Base function
struct mxFunc
{
    TaskHandle h;
    int32 retval;
    QString& errmsg;
    virtual void operator()() = 0;
    void checkError()
    {
        if (retval!=0) errmsg = daqmx::getErrorMessage(0);
    }

    mxFunc(TaskHandle ah, QString& aerrmsg) : h(ah), errmsg(aerrmsg)
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
/////////////////////////////////////////////////////////
struct mxCreateTask : public mxFunc
{
    QString name;
    virtual void operator()()
    {
        retval = DAQmxBaseCreateTask(name.toLatin1().constData(),&h);
        checkError();
    }

    mxCreateTask(const QString& aname, QString& msg) : mxFunc(0,msg), name(aname)
    {}
};
int daqmx::createTask(const QString &name, TaskHandle &h, QString &errmsg)
{
    mxCreateTask mxf(name, errmsg);
    daqmx::thread_->call_daqmx(&mxf);
    h = mxf.h;
    return mxf.retval;
}
/////////////////////////////////////////////////////////
struct mxClearTask : public mxFunc
{
    virtual void operator()()
    {
        retval = DAQmxBaseClearTask(h);
        checkError();
    }

    mxClearTask(TaskHandle h, QString& msg) : mxFunc(h,msg)
    {}
};
int daqmx::clearTask(TaskHandle h, QString &errmsg)
{
    mxClearTask mxf(h, errmsg);
    daqmx::thread_->call_daqmx(&mxf);
    return mxf.retval;
}
/////////////////////////////////////////////////////////
struct mxStartTask : public mxFunc
{
    virtual void operator()()
    {
        retval = DAQmxBaseStartTask(h);
        checkError();
    }

    DAQmxStartTask(TaskHandle h, QString& msg) : mxFunc(h,msg)
    {}
};
int daqmx::startTask(TaskHandle h, QString &errmsg)
{
    mxStartTask mxf(h, errmsg);
    daqmx::thread_->call_daqmx(&mxf);
    return mxf.retval;
}
/////////////////////////////////////////////////////////
struct mxStopTask : public mxFunc
{
    virtual void operator()()
    {
        retval = DAQmxStopTask(h);
        checkError();
    }

    mxStopTask(TaskHandle h, QString& msg) : mxFunc(h,msg)
    {}
};
int daqmx::stopTask(TaskHandle h, QString &errmsg)
{
    mxStopTask mxf(h, errmsg);
    daqmx::thread_->call_daqmx(&mxf);
    return mxf.retval;
}
/////////////////////////////////////////////////////////
struct mxCreateAIChan : public mxFunc
{
    QString name;
    int32 config;
    double minVal, maxVal;
    virtual void operator()()
    {
        retval = DAQmxBaseCreateAIVoltageChan(h,
                                          name.toLatin1().constData(),
                                          NULL,
                                          config, minVal, maxVal, DAQmx_Val_Volts, NULL);
        checkError();
    }

    mxCreateAIChan(TaskHandle h, const QString &aname,
                   int32 aconfig, double aminVal, double amaxVal, QString& msg) : mxFunc(h,msg),
        name(aname), config(aconfig), minVal(aminVal), maxVal(amaxVal)
    {}
};
int daqmx::createAIChan(TaskHandle h, const QString &name, int32 config, double minVal, double maxVal, QString &errmsg)
{
    mxCreateAIChan mxf(h,name,config,minVal,maxVal,errmsg);
    daqmx::thread_->call_daqmx(&mxf);
    return mxf.retval;
}
/////////////////////////////////////////////////////////
struct mxCreateAOChan : public mxFunc
{
    QString name;
    double minVal, maxVal;
    virtual void operator()()
    {
        retval = DAQmxBaseCreateAOVoltageChan(h,
                                          name.toLatin1().constData(),
                                          NULL,
                                          minVal, maxVal, DAQmx_Val_Volts, NULL);
        checkError();
    }

    mxCreateAOChan(TaskHandle h, const QString &aname,
                   double aminVal, double amaxVal, QString& msg) : mxFunc(h,msg),
        name(aname), config(aconfig), minVal(aminVal), maxVal(amaxVal)
    {}
};
int daqmx::createAOChan(TaskHandle h, const QString &name, double minVal, double maxVal, QString &errmsg)
{
    mxCreateAOChan mxf(h,name,minVal,maxVal,errmsg);
    daqmx::thread_->call_daqmx(&mxf);
    return mxf.retval;
}
/////////////////////////////////////////////////////////
struct mxCreateDIChan : public mxFunc
{
    QString name;
    virtual void operator()()
    {
        retval = DAQmxBaseCreateDIChan(h,name.toLatin1().constData(),
                                   NULL, DAQmx_Val_ChanForAllLines);
        checkError();
    }

    mxCreateDIChan(TaskHandle h, const QString &aname, QString& msg) : mxFunc(h,msg), name(aname)
    {}
};
int daqmx::createDIChan(TaskHandle h, const QString& name, QString& errmsg)
{
    mxCreateDIChan mxf(h,name,errmsg);
    daqmx::thread_->call_daqmx(&mxf);
    return mxf.retval;
}
/////////////////////////////////////////////////////////
struct mxCreateDOChan : public mxFunc
{
    QString name;
    virtual void operator()()
    {
        retval = DAQmxBaseCreateDOChan(h,name.toLatin1().constData(),
                                   NULL, DAQmx_Val_ChanForAllLines);
        checkError();
    }

    mxCreateDOChan(TaskHandle h, const QString &aname, QString& msg) : mxFunc(h,msg), name(aname)
    {}
};
int daqmx::createDOChan(TaskHandle h, const QString& name, QString& errmsg)
{
    mxCreateDOChan mxf(h,name,errmsg);
    daqmx::thread_->call_daqmx(&mxf);
    return mxf.retval;
}
/////////////////////////////////////////////////////////
struct mxCreateCICountEdgesChan : public mxFunc
{
    QString name;
    int32 edge, countDir;
    virtual void operator()()
    {
        retval = DAQmxBaseCreateCICountEdgesChan(h, name.toLatin1().constData(),
                                             NULL, edge, 0, countDir);
        checkError();
    }

    mxCreateCICountEdgesChan(TaskHandle h, const QString &aname,
                             int32 aedge, int32 acountDir, QString& msg) : mxFunc(h,msg),
        name(aname), edge(aedge), countDir(acountDir)
    {}
};
int daqmx::createCICountEdgesChan(TaskHandle h, const QString& name, int32 edge, int32 countDir, QString& errmsg)
{
    mxCreateCICountEdgesChan mxf(h,name,edge,countDir,errmsg);
    daqmx::thread_->call_daqmx(&mxf);
    return mxf.retval;
}
/////////////////////////////////////////////////////////
struct mxReadAnalog : public mxFunc
{
    const double& timo;
    double* buff;
    int32& read;
    virtual void operator()()
    {
        uInt32 ns = read;
        retval = DAQmxBaseReadAnalogF64(h,1,timo,DAQmx_Val_GroupByChannel,
                                    buff,ns,&read,NULL);
        checkError();
    }

    mxReadAnalog(TaskHandle h, const double &atimo, double* abuff, int32& aread, QString& msg) : mxFunc(h,msg),
        timo(atimo), buff(abuff), read(aread)
    {}
};
int daqmx::readAnalog(TaskHandle h, double timo, double *buff, int32 &read, QString &msg)
{
    mxReadAnalog mxf(h,timo,buff,read,errmsg);
    daqmx::thread_->call_daqmx(&mxf);
    return mxf.retval;
}
/////////////////////////////////////////////////////////
struct mxReadDigital : public mxFunc
{
    const double& timo;
    uInt32* buff;
    int32& read;
    virtual void operator()()
    {
        uInt32 ns = read;
        retval = DAQmxBaseReadDigitalU32(h,1,timo,DAQmx_Val_GroupByChannel,
                                    buff,ns,&read,NULL);
        checkError();
    }

    mxReadDigital(TaskHandle h, const double &atimo, double* abuff, int32& aread, QString& msg) : mxFunc(h,msg),
        timo(atimo), buff(abuff), read(aread)
    {}
};
int daqmx::readDigital(TaskHandle h, double timo, uInt32* buff, int32 &read, QString &msg)
{
    mxReadDigital mxf(h,timo,buff,read,errmsg);
    daqmx::thread_->call_daqmx(&mxf);
    return mxf.retval;
}
/////////////////////////////////////////////////////////
struct mxReadCounter : public mxFunc
{
    const double& timo;
    uInt32* buff;
    int32& read;
    virtual void operator()()
    {
        uInt32 ns = read;
        retval = DAQmxBaseReadCounterU32(h,1,timo,DAQmx_Val_GroupByChannel,
                                    buff,ns,&read,NULL);
        checkError();
    }

    mxReadCounter(TaskHandle h, const double &atimo, double* abuff, int32& aread, QString& msg) : mxFunc(h,msg),
        timo(atimo), buff(abuff), read(aread)
    {}
};
int daqmx::readCounter(TaskHandle h, double timo, uInt32* buff, int32 &read, QString &msg)
{
    mxReadCounter mxf(h,timo,buff,read,errmsg);
    daqmx::thread_->call_daqmx(&mxf);
    return mxf.retval;
}
/////////////////////////////////////////////////////////
struct mxWriteAnalog : public mxFunc
{
    const double& timo;
    const double* buff;
    int32& written;
    virtual void operator()()
    {
        retval = DAQmxBaseWriteAnalogF64(h,1,FALSE,timo,DAQmx_Val_GroupByChannel,
                                    buff,&written,NULL);
        checkError();
    }

    mxWriteAnalog(TaskHandle h, const double &atimo, const double* abuff, int32& aread, QString& msg) : mxFunc(h,msg),
        timo(atimo), buff(abuff), read(aread)
    {}
};
int daqmx::writeAnalog(TaskHandle h, double timo, const double *buff, int32 &written, QString &msg)
{
    mxWriteAnalog mxf(h,timo,buff,read,errmsg);
    daqmx::thread_->call_daqmx(&mxf);
    return mxf.retval;
}
/////////////////////////////////////////////////////////
struct mxWriteDigital : public mxFunc
{
    const double& timo;
    const uInt32* buff;
    int32& written;
    virtual void operator()()
    {
        retval = DAQmxBaseWriteDigitalU32(h,1,FALSE,timo,DAQmx_Val_GroupByChannel,
                                    buff,&written,NULL);
        checkError();
    }

    mxWriteDigital(TaskHandle h, const double &atimo, const uInt32* abuff, int32& aread, QString& msg) : mxFunc(h,msg),
        timo(atimo), buff(abuff), read(aread)
    {}
};
int daqmx::writeDigital(TaskHandle h, double timo, const uInt32* buff, int32 &written, QString &msg)
{
    mxWriteDigital mxf(h,timo,buff,read,errmsg);
    daqmx::thread_->call_daqmx(&mxf);
    return mxf.retval;
}

