#ifndef _RTINTERFACE_H_
#define _RTINTERFACE_H_

#include "RtObject.h"

#include <QVector>
#include <QHostAddress>
#include <QTcpSocket>

#include "serialport.h"

#include <modbus.h>

class RtDevice;

class RtInterface : public RtObject
{
	Q_OBJECT
	Q_PROPERTY(uint address READ address WRITE setAddress)
	Q_PROPERTY(uint timeout READ timeout WRITE setTimeout)
	Q_PROPERTY(bool isOpen READ isOpen)

protected:
	uint addr_, 
		timeout_; // ms
	bool isOpen_;

protected:
	QVector<RtDevice*> ports_;

public:
	RtInterface(const QString& name, const QString& descr, RtObject* parent, uint addr);
	virtual ~RtInterface(void);
	virtual void detach();

	// getters
	uint address() const { return addr_; }
	uint timeout() const { return timeout_; }
	bool isOpen() const { return isOpen_; }

	// setters
	void setAddress(uint v);
	void setTimeout(uint v);

	// io
	virtual bool open_port(uint i, RtDevice*);
	virtual void close_port(uint i);
	virtual void clear_port(uint i) { i=i; }
	virtual int read(uint port, char* buff, int len, int eos = 0);
	virtual int write(uint port, const char* buff, int len, int eos = 0);
	virtual bool isValidPort(uint i) { i=i; return true; }

	// diagnostics
	virtual int readStatusByte(uint port) { port=port; return 0; }

protected:
	virtual bool open_() { return isOpen_ = true; }
	virtual void close_();
	virtual void clear_() {}
	virtual void setTimeout_(uint v) { timeout_=v; }

	bool throwIfOpen();

public: // slots
	Q_SLOT bool open() { return open_(); } 
	Q_SLOT void close() { close_(); }
	Q_SLOT void clear() { clear_(); }
};

#define WIO os
#define WIOS os::serialport

class RtSerialInterface : public RtInterface
{
	Q_OBJECT
	Q_PROPERTY(QString name READ name)
	Q_PROPERTY(BaudRate baud READ baud WRITE setBaud)
	Q_PROPERTY(Parity parity READ parity WRITE setParity)
	Q_PROPERTY(DataBits databits READ databits WRITE setDatabits)
	Q_PROPERTY(StopBits stopbits READ stopbits WRITE setStopbits)
	Q_PROPERTY(Handshake handshake READ handshake WRITE setHandshake)
	Q_ENUMS(BaudRate)
	Q_ENUMS(Parity)
	Q_ENUMS(DataBits)
	Q_ENUMS(StopBits)
	Q_ENUMS(Handshake)

public:

	enum BaudRate
	{
		bps110     = CBR_110,		// 110 bits/sec
		bps300     = CBR_300,		// 300 bits/sec
		bps600     = CBR_600,		// 600 bits/sec
		bps1200    = CBR_1200,		// 1200 bits/sec
		bps2400    = CBR_2400,		// 2400 bits/sec
		bps4800    = CBR_4800,		// 4800 bits/sec
		bps9600    = CBR_9600,		// 9600 bits/sec
		bps14400   = CBR_14400,		// 14400 bits/sec
		bps19200   = CBR_19200,		// 19200 bits/sec (default)
		bps38400   = CBR_38400,		// 38400 bits/sec
		bps56000   = CBR_56000,		// 56000 bits/sec
		bps57600   = CBR_57600,		// 57600 bits/sec
		bps115200  = CBR_115200,	// 115200 bits/sec
		bps128000  = CBR_128000,	// 128000 bits/sec
		bps256000  = CBR_256000,	// 256000 bits/sec
	};

	// Data bits (5-8)
	enum DataBits
	{
		DataBits5       =  5,			// 5 bits per byte
		DataBits6       =  6,			// 6 bits per byte
		DataBits7       =  7,			// 7 bits per byte
		DataBits8       =  8			// 8 bits per byte (default)
	};

	// Parity scheme
	enum Parity
	{
		None    = NOPARITY,		// No parity (default)
		Odd     = ODDPARITY,	// Odd parity
		Even    = EVENPARITY,	// Even parity
		Mark    = MARKPARITY,	// Mark parity
		Space   = SPACEPARITY	// Space parity
	};

	// Stop bits
	enum StopBits
	{
		StopBits1       = ONESTOPBIT,	// 1 stopbit (default)
		StopBits1_5     = ONE5STOPBITS,// 1.5 stopbit
		StopBits2       = TWOSTOPBITS	// 2 stopbits
	};

	// Handshaking
	enum Handshake
	{
		No				=  0,	// No handshaking
		Hardware		=  1,	// Hardware handshaking (RTS/CTS)
		Software		=  2	// Software handshaking (XON/XOFF)
	};

protected:
	WIO::serialport port_;

	BaudRate baud_;
	Parity parity_;
	DataBits databits_;
	StopBits stopbits_;
	Handshake handshake_;

public:
	RtSerialInterface(const QString& name, const QString& descr, RtObject* parent, uint addr=1);
	virtual ~RtSerialInterface(void);

	virtual void registerTypes(QScriptEngine* eng);

	// getters
	QString name() const;
	BaudRate baud() const { return baud_; }
	Parity parity() const { return parity_; }
	DataBits databits() const { return databits_; }
	StopBits stopbits() const { return stopbits_; }
	Handshake handshake() const { return handshake_; }

	// setters
	void setBaud(BaudRate v);
	void setParity(Parity v);
	void setDatabits(DataBits v);
	void setStopbits(StopBits v);
	void setHandshake(Handshake v);

	// io
	virtual int read(uint port, char* buff, int len, int eos = 0);
	virtual int write(uint port, const char* buff, int len, int eos = 0);

protected:
	virtual bool open_();
	virtual void close_();
	virtual void clear_();
};

class RtRS232 : public RtSerialInterface
{
	Q_OBJECT
public:
	RtRS232(const QString& name, RtObject* parent, uint addr=1);
	virtual ~RtRS232();

	virtual bool isValidPort(uint i) { return i==0; }

};

class RtTcpip : public RtInterface
{
	Q_OBJECT
	Q_PROPERTY(uint port READ port WRITE setPort)
	Q_PROPERTY(QString host READ host WRITE setHost)

protected:
	uint port_;
	QHostAddress host_;
	QTcpSocket socket_;
public:
	RtTcpip(const QString& name, RtObject* parent, const QString& ahost = QString(), uint portno = 0);
	virtual ~RtTcpip();

	// getters
	uint port() const { return port_; }
	QString host() const { return host_.toString(); }

	// setters
	void setPort(uint p);
	void setHost(const QString& h);

	virtual bool isValidPort(uint i) { return i==0; }

	// io
	virtual int read(uint port, char* buff, int len, int eos = 0);
	virtual int write(uint port, const char* buff, int len, int eos = 0);

protected:
	virtual bool open_();
	virtual void close_();
};

#endif


