#include "RtComedi.h"

#include "ni6221.h"

#include "RtDataChannel.h"

Rt6221::Rt6221(const QString& name, RtObject* parent, const QString& devName) :
    RtObject(name,"NI6221 comedi device",parent), dev_(0), deviceName_(devName)
{
}

bool Rt6221::open()
{
    if (dev_) return true;
    os::auto_lock L(comm_lock);
    try {
        dev_ = new ni6221(deviceName_.toLatin1().constData());
    }
    catch(std::runtime_error& err)
    {
        pushError("Error opening NI6221 comedi device",err.what());
        dev_ = 0;
    }
    emit propertiesChanged();
    return dev_!=0;
}

void Rt6221::close()
{
    os::auto_lock L(comm_lock);
    for(int i=0; i<sub_dev_.size(); i++) sub_dev_[i]->forcedOffline();
    if (dev_) {
        delete dev_;
        dev_ = 0;
    }

}
RtObject* Rt6221::newCounter(const QString &name, int idx)
{
    // check name
    if (!checkName(name)) return 0;
    // check idx
    if (idx<0 || idx>1)
    {
        throwScriptError("Invalid counter index idx. Allowed: idx=0 or 1");
        return 0;
    }
    int sd = idx + 11; // counter subdev idx

    Rt6221device* obj = new Rt6221device(name,"NI6221 counter",this,sd,Rt6221device::CountEdges);
    sub_dev_.push_back(obj);
    return obj;
}
RtObject* Rt6221::newDigitalInput(const QString& name)
{
    // check name
    if (!checkName(name)) return 0;

    int sd = 2; // dio idx

    Rt6221device* obj = new Rt6221device(name,"NI6221 DI",this,sd,Rt6221device::DigitalInput);
    sub_dev_.push_back(obj);
    return obj;
}
RtObject* Rt6221::newDigitalOutput(const QString& name)
{
    // check name
    if (!checkName(name)) return 0;

    int sd = 2; // dio idx

    Rt6221device* obj = new Rt6221device(name,"NI6221 DI",this,sd,Rt6221device::DigitalOutput);
    sub_dev_.push_back(obj);
    return obj;
}
RtObject* Rt6221::newAnalogInput(const QString& name)
{
    // check name
    if (!checkName(name)) return 0;

    int sd = 0; // ai idx

    Rt6221device* obj = new Rt6221device(name,"NI6221 DI",this,sd,Rt6221device::AnalogInput);
    sub_dev_.push_back(obj);
    return obj;
}
RtObject* Rt6221::newAnalogOutput(const QString& name)
{
    // check name
    if (!checkName(name)) return 0;

    int sd = 1; // ao idx

    Rt6221device* obj = new Rt6221device(name,"NI6221 DI",this,sd,Rt6221device::AnalogOutput);
    sub_dev_.push_back(obj);
    return obj;
}
/********************* Rt6221device ******************************/
Rt6221device::Rt6221device(const QString &name, const QString &desc, Rt6221 *parent,
                           int addr, DaqType t) :
    RtJob(name,desc,parent), dev_(parent), subdev_(0), idx_(addr), subdevType_(t), conv_(0)
{

}
void Rt6221device::detach()
{
    setOnline(false);
    RtJob::detach();
    dev_->removeSubdevice(this);
}

// states / messages
bool Rt6221device::throwIfOffline()
{
    bool ret = !online();
    if (ret) throwScriptError("Not possible when task is offline.");
    return ret;
}
bool Rt6221device::throwIfOnline()
{
    bool ret = online();
    if (ret) throwScriptError("Not possible when task is online.");
    return ret;
}
void Rt6221device::setOnline(bool on)
{
    if (throwIfArmed()) return;
    if (on==online()) return;
    os::auto_lock L(comm_lock);
    if (on)
    {
        if (dev_ && dev_->isOpen())
        {
            subdev_ = dev_->dev()->getSubDevice(idx_);
            bool ok = true;
            if (subdev_)
            {
                try {
                switch(subdevType_)
                {
                case AnalogInput:
                    conv_ = new comedi::to_physical(subdev_->hardcal_converter(0,0,COMEDI_TO_PHYSICAL));
                    break;
                case CountEdges:
                    ((ni6221::ctr*)subdev_)->setup_count_edges();
                    ((ni6221::ctr*)subdev_)->arm();
                    break;
                default:
                    break;
                }
                }
                catch(std::runtime_error& err)
                {
                    pushError(err.what());
                    ok = false;
                }
            }
            if (!ok) {
                subdev_ = 0;
                if (conv_) delete conv_;
                conv_ = 0;
            }
        }

    }
    else {
        subdev_ = 0;
        if (conv_) delete conv_;
        conv_ = 0;
    }
    emit propertiesChanged();
}
bool Rt6221device::on()
{
    setOnline(true);
    return online();
}

void Rt6221device::off()
{
    setOnline(false);
}

bool Rt6221device::addChannel(unsigned int chNo, const QString &optStr)
{
    if (throwIfOnline()) return false;
    if (throwIfArmed()) return false;

    unsigned int config_code1 = 0;

    QString funcname, msg;
    int nch = channels_.size();

    switch(subdevType_)
    {
    case AnalogInput:
        config_code1 = AREF_GROUND;
        if (!optStr.isEmpty())
        {
            QString config = optStr.toUpper();
            if (config=="GROUND") config_code1 = AREF_GROUND;
            else if (config=="COMMON") config_code1 = AREF_COMMON;
            else if (config=="DIFF") config_code1 = AREF_DIFF;
            else {
                throwScriptError("Not valid config argument for AI. "
                                 "Valid inputs: GROUND, COMMON, DIFF");
                return false;
            }
        }
        break;
    case AnalogOutput:
        break;
    case DigitalInput:
        break;
    case DigitalOutput:
        break;
    case CountEdges:
        if (nch) {
            throwScriptError("Only 1 counter channel allowed per device.");
            return false;
        }
        break;
    }


    // adjust channels
    nch++;
    channels_.push_back(new RtDataChannel(QString("ch%1").arg(nch),"NI6221 channel",this));
    channel_no_.push_back(chNo);
    channel_aref_.push_back(config_code1);

    // adjust buffers
    digitalBuffer_.resize(nch);


    emit propertiesChanged();
    return true;
}

bool Rt6221device::doReadWrite_()
{
    QString msg;
    int nch = channels_.size();

    // prepare data for writing
    switch(subdevType_)
    {
    case AnalogOutput:
    case DigitalOutput:
        for(int i=0; i<nch; i++) digitalBuffer_[i] = channels_[i]->value();
    default:
        break;
    }

    // perform actual read/write
    switch(subdevType_)
    {
    case AnalogInput:
    case DigitalInput:
    case CountEdges:
        try {
            for(int i=0; i<nch; i++)
                digitalBuffer_[i] = subdev_->data_read(channel_no_[i],
                                   0,
                                   channel_aref_[i]);
        }
        catch(std::runtime_error& err) {
            pushError("Error reading comedi channels",err.what());
            return false;
        }
        break;
    case DigitalOutput:
    case AnalogOutput:
        try {
            for(int i=0; i<nch; i++)
                subdev_->data_write(channel_no_[i],
                                   0,
                                   channel_aref_[i], digitalBuffer_[i]);
        }
        catch(std::runtime_error& err) {
            pushError("Error writing comedi channels",err.what());
            return false;
        }
    default:
        break;
    }

    // handle data after writing
    unsigned int v_;
    switch(subdevType_)
    {
    case AnalogInput:
        for(int i=0; i<nch; i++) channels_[i]->push(conv_->operator ()(digitalBuffer_[i]));
        break;
    case DigitalInput:
        for(int i=0; i<nch; i++) channels_[i]->push(digitalBuffer_[i]);
        break;
    case CountEdges:
        channels_[0]->push(digitalBuffer_[0] - ctr_val_);
        ctr_val_ = digitalBuffer_[0];
    default:
        break;
    }

    return true;
}
// arming
bool Rt6221device::arm_()
{
    if (throwIfOffline()) return armed_ = false;
    else {
        digitalBuffer_.fill(0);
        ctr_val_=0;
        return RtJob::arm_();
    }
}
void Rt6221device::read()
{
    if (throwIfArmed()) return;
    if (throwIfOffline()) return;

    if (doReadWrite_())
    {
        for(int i=0; i<channels_.size(); ++i)
            channels_[i]->forceProcces();
    }
    else throwScriptError("NI6221 read failed. Check error log.");
}
void Rt6221device::write()
{
    if (throwIfArmed()) return;
    if (throwIfOffline()) return;

    for(int i=0; i<channels_.size(); ++i)
        channels_[i]->forceProcces();
    if (!doReadWrite_())
        throwScriptError("NI6221 write failed. Check error log.");
}
void Rt6221device::forcedOffline()
{
    os::auto_lock L(comm_lock);
    if (armed_) disarm_();// forcedDisarm(reason);
    if (online())
    {
        setOnline(false);
        pushError("forced offline");
    }
}
