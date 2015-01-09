#include "rtdaqmxtask.h"
#include <RtDataChannel.h>
#include <RtEnumHelper.h>
Q_SCRIPT_ENUM(DaqType,RtDAQmxTask)

#include "daqmx.h"

RtDAQmxTask::RtDAQmxTask(const QString& name, RtObject *parent) :
    RtJob(name,"NI-DAQmx Task",parent), handle_(0),
    online_(false), daqType_(Undefined), timeout_(0.1)
{
    TaskHandle h;
    QString msg;
    daqmx::init();
    if (daqmx::createTask(name,h,msg)==0)
    {
        handle_ = (ni_handle_t)h;
    }
    else {
        pushError("NI DAQmx Task creation failed", msg);
        throwScriptError(msg);
    }
}
RtDAQmxTask::~RtDAQmxTask()
{
    QString msg;
    if (handle_) daqmx::clearTask((TaskHandle)handle_,msg);
}
void RtDAQmxTask::detach()
{
    RtJob::detach();
    setOnline_(false);
}
void RtDAQmxTask::registerTypes(QScriptEngine* e)
{
    qScriptRegisterDaqType(e);
    RtJob::registerTypes(e);
}
void RtDAQmxTask::setOnline(bool on)
{
    if (throwIfArmed()) return;
    os::auto_lock L(comm_lock);
    if (on!=online_)
    {
        setOnline_(on);
        emit propertiesChanged();
    }
}
void RtDAQmxTask::setTimeout(const double& v)
{
    os::auto_lock L(comm_lock);
    if (v!=timeout_)
    {
        timeout_=v;
        emit propertiesChanged();
    }
}
bool RtDAQmxTask::setOnline_(bool on)
{
    // returns true on succes
    if (handle_==0) return false;
    if (on==online_) return true;
    if (on) // start the task
    {
        QString msg;
        int ret = daqmx::startTask((TaskHandle)handle_, msg);
        if (ret==0) {
            online_ = on;
            return true;
        }
        else {
            pushError("NI DAQmx Start Task failed", msg);
            throwScriptError(msg);
            return false;
        }
    }
    else // stop the task
    {
        QString msg;
        int ret = daqmx::stopTask((TaskHandle)handle_, msg);
        if (ret==0) {
            online_ = on;
            return true;
        } else {
            pushError("NI DAQmx Stop Task failed", msg);
            throwScriptError(msg);
            return false;
        }
    }
    return false;
}
bool RtDAQmxTask::addChannel_(DaqType type, const QString& physName, const QString& config1, const QString& config2, double minVal, double maxVal)
{
    // check if the type of channel is OK with the task daqType
    if (daqType_!=type || daqType_!=Undefined)
    {
        throwScriptError("Not compatible channel type");
        return false;
    }
    if (throwIfOnline()) return false;
    if (throwIfArmed()) return false;

    int32 config_code1, config_code2;
    int ret;
    QString funcname, msg;

    switch(type)
    {
    case AnalogInput:
        config_code1 = DAQmx_Val_Cfg_Default;
        if (!config1.isEmpty())
        {
            QString config = config1.toUpper();
            if (config=="RSE") config_code1 = DAQmx_Val_RSE;
            else if (config=="NRSE") config_code1 = DAQmx_Val_NRSE;
            else if (config=="DIFF") config_code1 = DAQmx_Val_Diff;
            else {
                throwScriptError("Not valid config argument for AI. Valid inputs: RSE, NRSE, DIFF");
                return false;
            }
        }
        ret = daqmx::createAIChan((TaskHandle)handle_,physName,config_code1,minVal,maxVal,msg);
        funcname = "DAQmxCreateAIVoltageChan";
        break;
    case AnalogOutput:
        ret = daqmx::createAOChan((TaskHandle)handle_,physName,minVal,maxVal,msg);
        funcname = "DAQmxCreateAOVoltageChan";
        break;
    case DigitalInput:
        ret = daqmx::createDIChan((TaskHandle)handle_,physName,msg);
        funcname = "DAQmxBaseCreateDIChan";
        break;
    case DigitalOutput:
        ret = daqmx::createDOChan((TaskHandle)handle_,physName,msg);
        funcname = "DAQmxBaseCreateDOChan";
        break;
    case CountEdges:
         // edge config
         config_code1 = DAQmx_Val_Rising;
         if (!config1.isEmpty())
         {
             QString config = config1.toUpper();
             if (config=="RISING") config_code1 = DAQmx_Val_Rising;
             else if (config=="FALLING") config_code1 = DAQmx_Val_Falling;
             else {
                 throwScriptError("Not valid edge config argument (1st) for CI. Valid inputs: Rising, Falling");
                 return false;
             }
         }

         // count dir
         config_code2 = DAQmx_Val_CountUp;
         if (!config2.isEmpty())
         {
             QString config = config2.toUpper();
             if (config=="UP") config_code2 = DAQmx_Val_CountUp;
             else if (config=="DOWN") config_code2 = DAQmx_Val_CountDown;
             else if (config=="EXT") config_code2 = DAQmx_Val_ExtControlled;
             else {
                 throwScriptError("Not valid count dir config argument (2nd) for CI. Valid inputs: Up, Down, Ext");
                 return false;
             }
         }

         ret = daqmx::createCICountEdgesChan((TaskHandle)handle_,physName,config_code1,config_code2,msg);
        funcname = "DAQmxBaseCreateCICountEdgesChan";
        break;
    }



    if (ret!=0) {
        funcname += " failed";
        pushError(funcname, msg);
        throwScriptError(funcname + ": " + msg);
        return false;
    }

    int ch_num = channels_.size() + 1;
    channels_.push_back(new RtDataChannel(QString("ch%1").arg(ch_num),physName,this));
    daqType_ = type;
    emit propertiesChanged();
    return true;
}
bool RtDAQmxTask::doReadWrite_()
{
    int32 ns;
    QString msg;
    int nch = channels_.size();

    if (daqType_==AnalogOutput)
        for(int i=0; i<nch; i++) analogBuffer_[i] = channels_[i]->value();
    else if (daqType_==DigitalOutput)
        for(int i=0; i<nch; i++) digitalBuffer_[i] = channels_[i]->value();

    switch (daqType_)
    {
    case AnalogInput:
        if (daqmx::readAnalog((TaskHandle)handle_,timeout_,analogBuffer_.data(),ns,msg)!=0)
        {
            pushError("DAQmxReadAnalogF64 failed",msg);
            return false;
        }
        break;
    case AnalogOutput:
        if (daqmx::writeAnalog((TaskHandle)handle_,timeout_,analogBuffer_.data(),ns,msg)!=0)
        {
            pushError("DAQmxWriteAnalogF64 failed",msg);
            return false;
        }
        break;
    case DigitalInput:
        if (daqmx::readDigital((TaskHandle)handle_,timeout_,(uInt32*)digitalBuffer_.data(),ns,msg)!=0)
        {
            pushError("DAQmxReadDigitalU32 failed",msg);
            return false;
        }
        break;
    case DigitalOutput:
        if (daqmx::writeDigital((TaskHandle)handle_,timeout_,(uInt32*)digitalBuffer_.data(),ns,msg)!=0)
        {
            pushError("DAQmxWriteAnalogF64 failed",msg);
            return false;
        }
        break;
    case CountEdges:
        if (daqmx::readCounter((TaskHandle)handle_,timeout_,(uInt32*)digitalBuffer_.data(),ns,msg)!=0)
        {
            pushError("DAQmxReadCounterU32 failed",msg);
            return false;
        }
        break;
    }
    if (ns!=1)
    {
        pushError("DAQmx read/write operations failed", "no samples");
        return false;
    }

    if (daqType_==AnalogInput)
        for(int i=0; i<nch; i++) channels_[i]->push(analogBuffer_[i]);
    else if (daqType_==DigitalInput)
        for(int i=0; i<nch; i++) channels_[i]->push(digitalBuffer_[i]);

    return true;
}
// states / messages
bool RtDAQmxTask::throwIfOffline()
{
    bool ret = !online_;
    if (ret) throwScriptError("Not possible when task is offline.");
    return ret;
}
bool RtDAQmxTask::throwIfOnline()
{
    if (online_) throwScriptError("Not possible when task is online.");
    return online_;
}
// arming
bool RtDAQmxTask::arm_()
{
    if (throwIfOffline()) return armed_ = false;
    else return RtJob::arm_();
}
void RtDAQmxTask::read()
{
    if (throwIfArmed()) return;
    if (throwIfOffline()) return;

    if (doReadWrite_())
    {
        for(int i=0; i<channels_.size(); ++i)
            channels_[i]->forceProcces();
    }
    else throwScriptError("NI DAQmx read failed. Check error log.");
}
void RtDAQmxTask::write()
{
    if (throwIfArmed()) return;
    if (throwIfOffline()) return;

    for(int i=0; i<channels_.size(); ++i)
        channels_[i]->forceProcces();
    if (!doReadWrite_())
        throwScriptError("NI DAQmx write failed. Check error log.");
}
