#ifndef _RTINTERFACE_H_
#define _RTINTERFACE_H_

#include "RtObject.h"

#include <QVector>
#include <QHostAddress>
#include <QTcpSocket>

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


