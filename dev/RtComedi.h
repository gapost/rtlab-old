#ifndef RTCOMEDI_H
#define RTCOMEDI_H

#include "RtJob.h"

#include <QVector>
#include <QList>

class ni6221;
namespace comedi {
    class subdevice;
    class to_physical;
}
class Rt6221device;
class RtDataChannel;

class Rt6221 : public RtObject
{
    Q_OBJECT

    // comedi device name e.g. /dev/comedi0
    Q_PROPERTY(QString deviceName READ deviceName)

    Q_PROPERTY(bool isOpen READ isOpen)


    ni6221* dev_;
    QString deviceName_;

    QList<Rt6221device*> sub_dev_;

public:
    Rt6221(const QString& name, RtObject* parent, const QString& devName);


    const QString& deviceName() const { return deviceName_; }
    bool isOpen() const { return dev_!=0; }

    ni6221* dev() { return dev_; }


    void removeSubdevice(Rt6221device* d)
    {
        sub_dev_.removeAll(d);
    }

public slots:
    bool open();
    void close();
    // create subdevices
    RtObject* newCounter(const QString& name, int idx);
    RtObject* newDigitalInput(const QString& name);
    RtObject* newDigitalOutput(const QString& name);
    RtObject* newAnalogInput(const QString& name);
    RtObject* newAnalogOutput(const QString& name);
};


class Rt6221device : public RtJob
{

    Q_OBJECT

    // online=true means comedi device ready
    Q_PROPERTY(bool online READ online WRITE setOnline)

public:
    enum DaqType {
        Undefined,
        AnalogInput,
        AnalogOutput,
        DigitalInput,
        DigitalOutput,
        CountEdges
    };
protected:
    comedi::subdevice* subdev_;
    comedi::to_physical* conv_;
    DaqType subdevType_;
    int idx_; // subdev index

    QPointer<Rt6221> dev_;

    QVector<RtDataChannel*> channels_;
    QVector<unsigned int> channel_no_;
    QVector<unsigned int> channel_aref_;
    unsigned int ctr_val_;

    QVector<unsigned int> digitalBuffer_;

    // Perform low level comedi read or write
    bool doReadWrite_();
    // RtJob implementation
    virtual bool arm_();
    virtual void run() { doReadWrite_(); }

public:
    Rt6221device(const QString& name, const QString& desc, Rt6221* parent, int addr, DaqType t);

    virtual void detach();

    bool online() const { return subdev_!=0; }
    void setOnline(bool on);
    void forcedOffline();

    bool throwIfOffline();
    bool throwIfOnline();

public slots:
    bool addChannel(unsigned int chNo, const QString& optStr = QString());
    bool on();
    void off();
    void read();
    void write();
};

#endif // RTCOMEDI_H
