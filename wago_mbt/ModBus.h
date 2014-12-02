#ifndef _MODBUSPP_H_
#define _MODBUSPP_H_

#include <windows.h>
#include <mbt.h>
#include <string>
#include "os_utils.h"

#define BUFFLEN 256

class modbuslib
{
	int status_;
public:
	modbuslib()
	{
		status_ = MBTInit();
	}
	~modbuslib()
	{
		status_ = MBTExit();
	}
	int status() const
	{
		return status_;
	}
	static const char* getErrorMsg(DWORD ErrorCode);
};

class modbus
{
	enum ModBusFunction {
		mbfReadInputRegister, mbfReadOutputRegister, mbfWriteRegister, 
		mbfReadInputCoil, mbfReadOutputCoil, mbfWriteCoil};

	std::string ip_;
	int port_, reqTimeOut_;
	bool useTCP_, connected_;
	HANDLE hsocket_;

    os::critical_section lock_;

	char rbuff[BUFFLEN], wbuff[BUFFLEN]; 
	
	int exec(ModBusFunction mbf, WORD addr_, WORD count_);
	void clearwbuff()
	{
		memset(wbuff,0,BUFFLEN*sizeof(char));
	}

public:
	static bool decodeBitInByte(const char* data, int bit)
	{
		const char* p = data + (bit >> 3);
		char mask = 0x01 << (bit % 8);
		return (*p & mask) == mask;
	}
	static char encodeBitInByte(bool v, int bit)
	{
		return v ? (0x01 << (bit % 8)) : 0x00;
	}
	static WORD swap(WORD w) { return MBTSwapWord(w); }
	static void swap(WORD* w) { *w = MBTSwapWord(*w); }
	static DWORD swap(DWORD w) { return MBTSwapDWord(w); }
	static void swap(DWORD* w) { *w = MBTSwapDWord(*w); }

	static short swap(short s) 
	{ 
		WORD w = MBTSwapWord(*((WORD*)(&s)));
		return *((short*)(&w)); 
	}
	static void swap(short* s) { *s = swap(*s); } 
	
	static int swap(int i) 
	{ 
		DWORD w = MBTSwapDWord(*((DWORD*)(&i)));
		return *((int*)(&w)); 
	}
	static void swap(int* i) { *i = swap(*i); } 
	static float swap(float f) 
	{ 
		DWORD w = MBTSwapDWord(*((DWORD*)(&f)));
		return *((float*)(&w)); 
	}
	static void swap(float* f) { *f = swap(*f); } 

public:
	modbus();
	modbus(const std::string ip, int port, bool usetcp = true, int timeout = 1000);
	virtual ~modbus(void);

	const std::string ip() { return ip_; }
	int port() { return port_; }
	bool tcp() { return useTCP_; }
	bool connected() { return connected_; }

	int connect();
    int connect(const std::string ip, int port, bool usetcp = true, int timeout = 1000);
    void disconnect();

	// Bit Access
    int readInputCoils(WORD addr, bool& val); 
    int readInputCoils(WORD addr, WORD count, bool* data);
    int readOutputCoils(WORD addr, bool& val); 
    int readOutputCoils(WORD addr, WORD count, bool* data);
    int writeCoils(WORD addr, bool val);
    int writeCoils(WORD addr, WORD count, const bool* data);

	// Register Access
	int readInputRegister(WORD addr, bool high, char& val);
	int readInputRegister(WORD addr, WORD& val);
	int readInputRegister(WORD addr, DWORD& val);
	int readInputRegister(WORD addr, WORD count, char* val);
	int readInputRegister(WORD addr, WORD count, WORD* val);
	int readInputRegister(WORD addr, WORD count, DWORD* val);

	int readOutputRegister(WORD addr, bool high, char& val);
	int readOutputRegister(WORD addr, WORD& val);
	int readOutputRegister(WORD addr, DWORD& val);
	int readOutputRegister(WORD addr, WORD count, char* val);
	int readOutputRegister(WORD addr, WORD count, WORD* val);
	int readOutputRegister(WORD addr, WORD count, DWORD* val);

	int writeRegister(WORD addr, WORD val);
	int writeRegister(WORD addr, DWORD val);
	int writeRegister(WORD addr, WORD count, const char* val);
	int writeRegister(WORD addr, WORD count, const WORD* val);
	int writeRegister(WORD addr, WORD count, const DWORD* val);
 };

#endif
