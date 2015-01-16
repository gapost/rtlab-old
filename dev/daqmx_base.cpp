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

/* A synchronization "pin"
 * The pin synchronizes the communication between a server and a client thread
 */
template<class T_>
class sync_pin
{
    QMutex mtxRequest, mtxReady;
    QWaitCondition waitRequest, waitReady;
    bool ready_;
    T_ d;

public:
    sync_pin() : ready_(false), d(0)
    {}

    void reset()
    {
        ready_ = false;
    }
    T_& data()
    {
        return d;
    }

    /* Wait until the server is ready and waiting for input.
     * Called from client.
     */
    void wait_for_ready()
    {
        mtxReady.lock();
        while (!ready_) waitReady.wait(&mtxReady);
        mtxReady.unlock();
    }
    /* Wait until the server is ready and waiting for input and
     * set a request for processing the data q.
     * Called from client.
     */
    void set_request(T_ q)
    {
        wait_for_ready();

        mtxRequest.lock();
        d = q;
        waitRequest.wakeOne();
        mtxRequest.unlock();
    }
    /* Called from the server.
     * Puts the server is ready mode, waiting for clients
     */
    void operator()()
    {
        mtxRequest.lock();
        {
            mtxReady.lock();
            ready_ = true;
            waitReady.wakeOne();
            mtxReady.unlock();
        }
        waitRequest.wait(&mtxRequest);
        {
            mtxReady.lock();
            ready_ = false;
            mtxReady.unlock();
        }
        mtxRequest.unlock();
    }
};

// This QThread handles all DAQmx Base calls
class daqmx_thread : public QThread
{

    sync_pin<mxFunc*> input_pin;
    sync_pin<int> output_pin;
    bool quit_;
public:
    daqmx_thread() : QThread()
    {}
    // overload QThread::start to ensure that
    // the thread is waiting for input
    void start()
    {
        if (isRunning()) return;

        quit_ = false;
        input_pin.reset();
        output_pin.reset();
        QThread::start();

        input_pin.wait_for_ready();
    }
    // main thread loop
    void run()
    {
        // begin the loop
        while (1)
        {
            // wait for request
            input_pin();

            // if requested to quit ...
            if (quit_) break;

            // call the DAQmx function
            input_pin.data()->operator ()();

            // unblock the caller
            output_pin.set_request(0);
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
        input_pin.set_request(g);

        // wait for completion
        output_pin();

        // return daqmx retval
        return g->retval;
    }
    void quit()
    {
        if (!isRunning()) return;
        // wait until thread is ready
        input_pin.wait_for_ready();
        quit_ = true;
        input_pin.set_request(0);
        wait();
    }
};

daqmx_thread* daqmx::thread_ = 0;
QAtomicInt daqmx::refcount_;

void daqmx::init()
{
    if (thread_ == 0) {
        thread_ = new daqmx_thread();
        thread_->start();
    }
    refcount_.ref();
}
void daqmx::deinit()
{
    if (!refcount_.deref())
    {
        thread_->quit();
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

    mxStartTask(TaskHandle h, QString& msg) : mxFunc(h,msg)
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
        retval = DAQmxBaseStopTask(h);
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
        name(aname), minVal(aminVal), maxVal(amaxVal)
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
    mxReadAnalog mxf(h,timo,buff,read,msg);
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

    mxReadDigital(TaskHandle h, const double &atimo, uInt32* abuff, int32& aread, QString& msg) : mxFunc(h,msg),
        timo(atimo), buff(abuff), read(aread)
    {}
};
int daqmx::readDigital(TaskHandle h, double timo, uInt32* buff, int32 &read, QString &msg)
{
    mxReadDigital mxf(h,timo,buff,read,msg);
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
        retval = DAQmxBaseReadCounterU32(h,1,timo,buff,ns,&read,NULL);
        checkError();
    }

    mxReadCounter(TaskHandle h, const double &atimo, uInt32* abuff, int32& aread, QString& msg) : mxFunc(h,msg),
        timo(atimo), buff(abuff), read(aread)
    {}
};
int daqmx::readCounter(TaskHandle h, double timo, uInt32* buff, int32 &read, QString &msg)
{
    mxReadCounter mxf(h,timo,buff,read,msg);
    daqmx::thread_->call_daqmx(&mxf);
    return mxf.retval;
}
/////////////////////////////////////////////////////////
struct mxWriteAnalog : public mxFunc
{
    const double& timo;
    double* buff;
    int32& written;
    virtual void operator()()
    {
        retval = DAQmxBaseWriteAnalogF64(h,1,FALSE,timo,DAQmx_Val_GroupByChannel,
                                    buff,&written,NULL);
        checkError();
    }

    mxWriteAnalog(TaskHandle h, const double &atimo, double* abuff, int32& w, QString& msg) : mxFunc(h,msg),
        timo(atimo), buff(abuff), written(w)
    {}
};
int daqmx::writeAnalog(TaskHandle h, double timo, const double *buff, int32 &written, QString &msg)
{
    mxWriteAnalog mxf(h,timo,(double*)buff,written,msg);
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
                                    (uInt32*)buff,&written,NULL);
        checkError();
    }

    mxWriteDigital(TaskHandle h, const double &atimo, const uInt32* abuff, int32& w, QString& msg) : mxFunc(h,msg),
        timo(atimo), buff(abuff), written(w)
    {}
};
int daqmx::writeDigital(TaskHandle h, double timo, const uInt32* buff, int32 &written, QString &msg)
{
    mxWriteDigital mxf(h,timo,buff,written,msg);
    daqmx::thread_->call_daqmx(&mxf);
    return mxf.retval;
}

