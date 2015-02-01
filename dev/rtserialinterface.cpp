#include "rtserialinterface.h"

#include "RtEnumHelper.h"

Q_SCRIPT_ENUM(BaudRate, RtSerialInterface)
Q_SCRIPT_ENUM(Parity, RtSerialInterface)
Q_SCRIPT_ENUM(DataBits, RtSerialInterface)
Q_SCRIPT_ENUM(StopBits, RtSerialInterface)
Q_SCRIPT_ENUM(Handshake, RtSerialInterface)

RtSerialInterface::RtSerialInterface(const QString &name, RtObject *parent, const QString &portName) :
    RtInterface(name,"serial interface",parent,0)
{
    port_ = new QextSerialPort(portName,QextSerialPort::Polling,this);

    ports_.push_back((RtDevice*)0); // only 1 device
}

void RtSerialInterface::registerTypes(QScriptEngine* e)
{
    qScriptRegisterBaudRate(e);
    qScriptRegisterParity(e);
    qScriptRegisterDataBits(e);
    qScriptRegisterStopBits(e);
    qScriptRegisterHandshake(e);
    RtInterface::registerTypes(e);
}

void RtSerialInterface::setPortName(const QString &aname)
{
    if (throwIfOpen()) return;
    else
    {
        port_->setPortName(aname);
        emit propertiesChanged();
    }

}

void RtSerialInterface::setBaud(BaudRate v)
{
    if (throwIfOpen()) return;
    else
    {
        port_->setBaudRate((BaudRateType)v);
        emit propertiesChanged();
    }
}
void RtSerialInterface::setParity(Parity v)
{
    if (throwIfOpen()) return;
    else
    {
        port_->setParity((ParityType)v);
        emit propertiesChanged();
    }
}
void RtSerialInterface::setDatabits(DataBits v)
{
    if (throwIfOpen()) return;
    else
    {
        port_->setDataBits((DataBitsType)v);
        emit propertiesChanged();
    }
}
void RtSerialInterface::setStopbits(StopBits v)
{
    if (throwIfOpen()) return;
    else
    {
        port_->setStopBits((StopBitsType)v);
        emit propertiesChanged();
    }
}
void RtSerialInterface::setHandshake(Handshake v)
{
    if (throwIfOpen()) return;
    else
    {
        port_->setFlowControl((FlowType)v);
        emit propertiesChanged();
    }
}

int RtSerialInterface::read(uint port, char* buff, int len, int eos)
{
    os::auto_lock L(comm_lock);

    port=port;

    if (!port_->isOpen()) return 0;

    int read = 0;
    char c;
    char eos_char = eos & 0xFF;
    bool ok;
    while (ok = port_->read(&c,1))
    {
        buff[read++] = c;
        if (eos_char && c==eos_char)
        {
            read--;
            break;
        }
        if (read==len) break;
    }
    if (!ok)
    {
        pushError("Read char failed", "possibly timed-out");
    }
    return ok ? read : 0;
}

int RtSerialInterface::write(uint port, const char* buff, int len, int eos)
{
    os::auto_lock L(comm_lock);

    port=port;

    if (!port_->isOpen()) return 0;

    int written = 0;
    bool ok;
    while(written<len && (ok = port_->write(buff+written,1))) written++;
    char eos_char = eos & 0xFF;
    if (eos_char && ok) ok=port_->write(&eos_char,1);
    if (!ok)
    {
        pushError("Write char failed");
    }
    return ok ? written : 0;
}

bool RtSerialInterface::open_()
{
    if (isOpen()) return true;
    os::auto_lock L(comm_lock);
    port_->setTimeout(timeout());
    if (port_->open(QIODevice::ReadWrite))
        RtInterface::open_();
    else pushError(QString("Open %1 failed").arg(portName()));
    emit propertiesChanged();
    return isOpen();
}

void RtSerialInterface::close_()
{
    os::auto_lock L(comm_lock);
    RtInterface::close_();
    port_->close();
}

void RtSerialInterface::clear_()
{
    os::auto_lock L(comm_lock);
    if (port_->isOpen()) port_->flush();
}

