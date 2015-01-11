#include "rtmodbus.h"

#include <modbus.h>

//#elif __linux__
#include <errno.h>
//#endif


RtModbus::RtModbus(const QString& name, RtObject* parent, const QString& host, uint portn) :
    RtTcpip(name, parent, host, portn), ctx_(0)
{
}

RtModbus::~RtModbus(void)
{
}

bool RtModbus::open_()
{
    if (isOpen() && ctx_) return true;

    os::auto_lock L(comm_lock);

    /* TCP */
    modbus_t* ctx = modbus_new_tcp(host().toLatin1().constData(), port_);
    //modbus_set_debug(ctx, TRUE);

    if (modbus_connect(ctx) == -1) {
        pushError("modbus_connect failed", modbus_strerror(errno));
        modbus_free(ctx);
        isOpen_ = false;
    }
    else {
        isOpen_ = true;
        ctx_ = ctx;
    }

    emit propertiesChanged();

    return isOpen();
}

void RtModbus::close_()
{
    os::auto_lock L(comm_lock);
    RtInterface::close_();
    /* Close the connection */
    if (ctx_) {
        modbus_t* ctx = (modbus_t*)ctx_;
        modbus_close(ctx);
        modbus_free(ctx);
        ctx_ = 0;
        emit propertiesChanged();
    }
}

int RtModbus::read(uint port, char* buff, int len, int eos)
{
    os::auto_lock L(comm_lock);

    modbus_t* ctx = (modbus_t*)ctx_;

    eos = eos;
    int addr = port;
    uint16_t* regs = (uint16_t*)buff;
    int nb = (len/2) + (len%2);
    int ret = modbus_read_registers(ctx, addr, nb, regs);
    if (ret == -1) {
        pushError("modbus_read_registers failed", modbus_strerror(errno));
        return 0;
    }
    else return len;
}

int RtModbus::write(uint port, const char* buff, int len, int eos)
{
    os::auto_lock L(comm_lock);

    modbus_t* ctx = (modbus_t*)ctx_;

    eos = eos;
    int addr = port;
    const uint16_t* regs = (const uint16_t*)buff;
    int nb = (len/2) + (len%2);
    int ret = modbus_write_registers(ctx, addr, nb, regs);
    if (ret == -1) {
        pushError("modbus_write_registers failed", modbus_strerror(errno));
        return 0;
    }
    return len;
}

