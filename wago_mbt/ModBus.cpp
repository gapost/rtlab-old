#include "ModBus.h"
#include "MBTError.h"

modbus::modbus():
ip_("127.0.0.1"), port_(2400), useTCP_(true), reqTimeOut_(1000),
connected_(false), hsocket_(0)
{
}

modbus::modbus(const std::string ip, int port, bool usetcp, int timeout):
ip_(ip), port_(port), useTCP_(usetcp), reqTimeOut_(timeout),
connected_(false), hsocket_(0)
{
}

modbus::~modbus()
{
	disconnect();
}

int modbus::connect(const std::string ip, int port, bool usetcp, int timeout)
{
    os::auto_lock L(lock_);
	int ret = -1;
	if (!connected_)
	{
		ip_ = ip;
		port_ = port;
		useTCP_ = usetcp;
		reqTimeOut_ = timeout;
		ret = connect();
	}
	return ret;
}

int modbus::connect()
{
    os::auto_lock L(lock_);
	int ret = -1;
	if (!connected_)
	{
		ret = MBTConnect((LPCTSTR)(ip_.c_str()),port_,useTCP_,reqTimeOut_,&hsocket_);
		if (ret==0) connected_ = true;
		else disconnect();
	}
	return ret;
}

void modbus::disconnect()
{
    os::auto_lock L(lock_);
	connected_ = false;
	if (hsocket_!=0)
	{
		MBTDisconnect(hsocket_);
		hsocket_ = 0;
	}
}

int modbus::exec(ModBusFunction mbf, WORD addr_, WORD count_)
{
	int ret = 0;
	if (!connected_) ret = 0x80072750;
	else
	{
		switch (mbf)
		{
		case mbfReadInputRegister:
			ret = MBTReadRegisters(hsocket_, MODBUSTCP_TABLE_INPUT_REGISTER,
                                   addr_, count_, (LPBYTE)rbuff, 0);
			break;
		case mbfReadOutputRegister:
			ret = MBTReadRegisters(hsocket_, MODBUSTCP_TABLE_OUTPUT_REGISTER,
                                   addr_, count_, (LPBYTE)rbuff, 0);
			break;
		case mbfWriteRegister:
			ret = MBTWriteRegisters(hsocket_, addr_, count_,
                                    (LPBYTE)wbuff, 0);
			break;
		case mbfReadInputCoil:
			ret = MBTReadCoils(hsocket_, MODBUSTCP_TABLE_INPUT_COIL,
                               addr_, count_, (LPBYTE)rbuff, 0);
			break;
		case mbfReadOutputCoil:
			ret = MBTReadCoils(hsocket_, MODBUSTCP_TABLE_OUTPUT_COIL,
                               addr_, count_, (LPBYTE)rbuff, 0);
			break;
		case mbfWriteCoil:
			ret = MBTWriteCoils(hsocket_, addr_, count_,
                                (LPBYTE)wbuff, 0);
			break;
		}
	}
	return ret;
}

// Bit Access
int modbus::readInputCoils(WORD addr, bool& val)
{
    os::auto_lock L(lock_);

	int ret = exec(mbfReadInputCoil,addr,1);
    if (ret==0) val = rbuff[0];
	return ret;
}
int modbus::readInputCoils(WORD addr, WORD count, bool* data)
{
    os::auto_lock L(lock_);

	int ret = exec(mbfReadInputCoil,addr,count);
	if (ret==0)
	{
		for(int i=0; i<count; i++)
		{
			data[i] = decodeBitInByte(rbuff,i);
		}
	}
	return ret;
}
int modbus::readOutputCoils(WORD addr, bool& val)
{
    os::auto_lock L(lock_);

	int ret = exec(mbfReadOutputCoil,addr,1);
    if (ret==0) val = rbuff[0];
	return ret;
}
int modbus::readOutputCoils(WORD addr, WORD count, bool* data)
{
    os::auto_lock L(lock_);

	int ret = exec(mbfReadOutputCoil,addr,count);
	if (ret==0)
	{
		for(int i=0; i<count; i++)
		{
			data[i] = decodeBitInByte(rbuff,i);
		}
	}
	return ret;
}
int modbus::writeCoils(WORD addr, bool val)
{
    os::auto_lock L(lock_);

	clearwbuff();
	wbuff[0] = val ? 0xFF : 0x00;
	return exec(mbfWriteCoil,addr,1);
}
int modbus::writeCoils(WORD addr, WORD count, const bool* data)
{
    os::auto_lock L(lock_);

	clearwbuff();
	for(int i=0; i<count; i++)
	{
		wbuff[i>>3] ^= encodeBitInByte(data[i],i);
	}
	return exec(mbfWriteCoil,addr,count);
}


// Register Access
int modbus::readInputRegister(WORD addr, bool high, char &val)
{
    os::auto_lock L(lock_);

	int ret = exec(mbfReadInputRegister, addr, 1);
	if (ret==0) val = high ? rbuff[1] : rbuff[0];
	return ret;
}
int modbus::readInputRegister(WORD addr, WORD& val)
{
    os::auto_lock L(lock_);

	int ret = exec(mbfReadInputRegister, addr, 1);
	if (ret==0) 
	{
		WORD* w = (WORD*) rbuff;
		val = MBTSwapWord(*w);
	}
	return ret;
}
int modbus::readInputRegister(WORD addr, DWORD& val)
{
    os::auto_lock L(lock_);
	int ret = exec(mbfReadInputRegister, addr, 1);
	if (ret==0) 
	{
		DWORD* dw = (DWORD*) rbuff;
		val = MBTSwapDWord(*dw);
	}
	return ret;
}
int modbus::readInputRegister(WORD addr, WORD count, char* data)
{
    os::auto_lock L(lock_);

	int ret = exec(mbfReadInputRegister, addr, (count/2) + (count%2));
	if (ret==0) 
	{
		for (int i=0; i<count; i++) data[i] = rbuff[i];
	}
	return ret;
}
int modbus::readInputRegister(WORD addr, WORD count, WORD* data)
{
    os::auto_lock L(lock_);

	int ret = exec(mbfReadInputRegister, addr, count);
	if (ret==0) 
	{
		WORD* w = (WORD*)rbuff;
		for (int i=0; i<count; i++) 
		{
			data[i] = MBTSwapWord(w[i]);
		}
	}
	return ret;
}
int modbus::readInputRegister(WORD addr, WORD count, DWORD* data)
{
    os::auto_lock L(lock_);

	int ret = exec(mbfReadInputRegister, addr, 2*count);
	if (ret==0) 
	{
		DWORD* dw = (DWORD*)rbuff;
		for (int i=0; i<count; i++) 
		{
			data[i] = MBTSwapDWord(dw[i]);
		}
	}
	return ret;
}
int modbus::readOutputRegister(WORD addr, bool high, char &val)
{
    os::auto_lock L(lock_);

	int ret = exec(mbfReadOutputRegister, addr, 1);
	if (ret==0) val = high ? rbuff[1] : rbuff[0];
	return ret;
}
int modbus::readOutputRegister(WORD addr, WORD& val)
{
    os::auto_lock L(lock_);

	int ret = exec(mbfReadOutputRegister, addr, 1);
	if (ret==0) 
	{
		WORD* w = (WORD*) rbuff;
		val = MBTSwapWord(*w);
	}
	return ret;
}
int modbus::readOutputRegister(WORD addr, DWORD& val)
{
    os::auto_lock L(lock_);

	int ret = exec(mbfReadOutputRegister, addr, 2);
	if (ret==0) 
	{
		DWORD* dw = (DWORD*) rbuff;
		val = MBTSwapDWord(*dw);
	}
	return ret;
}
int modbus::readOutputRegister(WORD addr, WORD count, char* data)
{
    os::auto_lock L(lock_);

	int ret = exec(mbfReadOutputRegister, addr, (count/2) + (count%2));
	if (ret==0) 
	{
		for (int i=0; i<count; i++) data[i] = rbuff[i];
	}
	return ret;
}
int modbus::readOutputRegister(WORD addr, WORD count, WORD* data)
{
    os::auto_lock L(lock_);

	int ret = exec(mbfReadOutputRegister, addr, count);
	if (ret==0) 
	{
		WORD* w = (WORD*)rbuff;
		for (int i=0; i<count; i++) 
		{
			data[i] = MBTSwapWord(w[i]);
		}
	}
	return ret;
}
int modbus::readOutputRegister(WORD addr, WORD count, DWORD* data)
{
    os::auto_lock L(lock_);

	int ret = exec(mbfReadOutputRegister, addr, 2*count);
	if (ret==0) 
	{
		DWORD* dw = (DWORD*)rbuff;
		for (int i=0; i<count; i++) 
		{
			data[i] = MBTSwapDWord(dw[i]);
		}
	}
	return ret;
}

int modbus::writeRegister(WORD addr, WORD val)
{
    os::auto_lock L(lock_);

	clearwbuff();
	WORD* w = (WORD*)wbuff;
	w[0] = MBTSwapWord(val);
	return exec(mbfWriteRegister,addr,1);
}
int modbus::writeRegister(WORD addr, DWORD val)
{
    os::auto_lock L(lock_);

	clearwbuff();
	DWORD* w = (DWORD*)wbuff;
	w[0] = MBTSwapDWord(val);
	return exec(mbfWriteRegister,addr,2);
}
int modbus::writeRegister(WORD addr, WORD count, const char* data)
{
    os::auto_lock L(lock_);

	clearwbuff();
	for(int i=0; i<count; i++) wbuff[i] = data[i];
	return exec(mbfWriteRegister,addr,(count/2) + (count%2));
}
int modbus::writeRegister(WORD addr, WORD count, const WORD* data)
{
    os::auto_lock L(lock_);

	clearwbuff();
	WORD* w = (WORD*)wbuff;
	for(int i=0; i<count; i++) w[i] = MBTSwapWord(data[i]);
	return exec(mbfWriteRegister,addr,count);
}
int modbus::writeRegister(WORD addr, WORD count, const DWORD* data)
{
    os::auto_lock L(lock_);

	clearwbuff();
	DWORD* w = (DWORD*)wbuff;
	for(int i=0; i<count; i++) w[i] = MBTSwapDWord(data[i]);
	return exec(mbfWriteRegister,addr,2*count);
}

static char* __msgError[] =
{
"Unknown error",
"Not connected to device",
"Can""t connect to Device", //if Connect failed
"MBT_THREAD_CREATION_ERROR",
"MBT_EXIT_TIMEOUT_ERROR",
"MBT_UNKNOWN_THREAD_EXIT_ERROR",
"MBT_UNAVAILABLE_CLOCK_ERROR",
"MBT_NO_ENTRY_ADDABLE_ERROR",
"MBT_NO_JOB_ADDABLE_ERROR",
"MBT_HANDLE_INVALID_ERROR",
"MBT_CLOSE_FLAG_SET_ERROR",
"MBT_SOCKET_TIMEOUT_ERROR",
"MBT_WRONG_RESPONSE_FC_ERROR",
"MBT_RESPONSE_FALSE_LENGTH_ERROR",
"MBT_EXIT_ERROR",
"MBT_ILLEGAL_FUNCTION",
"MBT_ILLEGAL_DATA_ADDRESS",
"MBT_ILLEGAL_DATA_VALUE",
"MBT_ILLEGAL_RESPONSE_LENGTH",
"MBT_ACKNOWLEDGE",
"MBT_SLAVE_DEVICE_BUSY",
"MBT_NEGATIVE_ACKNOWLEDGE",
"MBT_MEMORY_PARITY_ERROR",
"MBT_GATEWAY_PATH_UNAVAILABLE",
"MBT_GATEWAY_TARGET_DEVICE_FAILED_TO_RESPOND"
};

const char* modbuslib::getErrorMsg(DWORD errcode)
{
	int idx = 0;
	if(errcode==0x80072750) idx = 1; 
	else if (errcode==0x80072751) idx = 2; 
	else if (errcode>=MBT_ERROR_PREFIX && errcode<MBT_ERROR_PREFIX+0x0C)
		idx = 3 + (errcode - MBT_ERROR_PREFIX);
	else if (errcode>=(MBT_EXCEPTION_PREFIX+0x01) && errcode<MBT_EXCEPTION_PREFIX+0x0C)
		idx = 15 + errcode - (MBT_EXCEPTION_PREFIX + 0x01);
	return __msgError[idx];
}

