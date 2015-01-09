#ifndef RTDAQMXTASK_H
#define RTDAQMXTASK_H

#include <RtJob.h>

#include <QVector>

class RtDataChannel;

class RtDAQmxTask : public RtJob
{
    Q_OBJECT

    // online=true means NI Task started
    Q_PROPERTY(bool online READ online WRITE setOnline)

    // The daqType property shows what kind of aqcuisition is being performed (see enum)
    // Initially it is 'undefined'
    // It becomes defined after adding the 1st channel
    // Adding subsequent channels is allowed only if they are of the same type
    Q_PROPERTY(DaqType daqType READ daqType)

    // timeout in sec is used with reading/writing NI DAQmx calls
    Q_PROPERTY(double timeout READ timeout WRITE setTimeout)

    Q_ENUMS(DaqType)

public:
    enum DaqType {
        Undefined,
        AnalogInput,
        AnalogOutput,
        DigitalInput,
        DigitalOutput,
        CountEdges
    };

    virtual void registerTypes(QScriptEngine* e);

protected:
    // the NI TaskHandle as a 32-bit pointer
    typedef quint32 ni_handle_t;
    ni_handle_t handle_;
    bool online_;
    DaqType daqType_;
    double timeout_;

    // implementation of start/stop for the NI task
    virtual bool setOnline_(bool on);

    QVector<RtDataChannel*> channels_;

    QVector<double> analogBuffer_;
    QVector<quint32> digitalBuffer_;

    bool throwIfOffline();
    bool throwIfOnline();

    // Generic routine that adds a channel to the NI task
    bool addChannel_(DaqType type, const QString& physName, const QString& config1 = QString(), const QString& config2 = QString(), double minVal = 0.0, double maxVal = 1.0);

    // Perform low level NI DAQmx read or write
    bool doReadWrite_();

    // RtJob implementation
    virtual bool arm_();
    virtual void run() { doReadWrite_(); }

public:
    RtDAQmxTask(const QString& name, RtObject *parent);
    virtual ~RtDAQmxTask();
    virtual void detach();
    // virtual void registerTypes(QScriptEngine* e);

    // property get/set
    bool online() const { return online_; }
    DaqType daqType() const { return daqType_; }
    double timeout() const { return timeout_; }

    void setOnline(bool v);
    void setTimeout(const double& v);

signals:

public slots:
    bool addAnalogInputChannel(const QString& physName, const QString& config = QString(), double minVal = 0.0, double maxVal = 10.0)
    {
        return addChannel_(AnalogInput, physName, config, QString(), minVal, maxVal);
    }
    bool addAnalogOutputChannel(const QString& physName, double minVal = 0.0, double maxVal = 10.0)
    {
        return addChannel_(AnalogOutput, physName, QString(), QString(), minVal, maxVal);
    }
    bool addDigitalInputChannel(const QString& physName)
    {
        return addChannel_(DigitalInput, physName);
    }
    bool addDigitalOutputChannel(const QString& physName)
    {
        return addChannel_(DigitalOutput, physName);
    }
    bool addCountEdgesChannel(const QString& physName, const QString& edgeConfig = QString(), const QString& countDir = QString())
    {
        return addChannel_(CountEdges, physName, edgeConfig, countDir);
    }
    void read();
    void write();
    bool on();
    void off();
};

#endif // RTDAQMXTASK_H
