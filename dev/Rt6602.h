#ifndef _RT6602_H_
#define _RT6602_H_

#include "RtDevice.h"

namespace pci6602 {
	class device;
	class counter;
	class digital_io;
}

class Rt6602 : public RtInterface
{
	Q_OBJECT

protected:
	pci6602::device* dev_;

public:
	Rt6602(const QString& name, RtObject* parent, uint addr = 1);
	virtual ~Rt6602(void);

	pci6602::device* dev() { return dev_; }

protected:
	virtual bool open_();
	virtual void clear_();

public: // slots
	Q_SLOT void newCounter(const QString& name, int no);
	Q_SLOT void newPulseGenerator(const QString& name, int no);
	Q_SLOT void newDio(const QString& name, int no);
};

class Rt6602CounterBase : public RtDevice
{
	Q_OBJECT
protected:
	pci6602::counter* ctr_;
public:
	Rt6602CounterBase(const QString& name, Rt6602* parent, uint no);
	virtual ~Rt6602CounterBase(void);

	Rt6602* parent() { return (Rt6602*)(RtObject::parent()); }
	pci6602::device* dev() { return parent()->dev(); }

protected:
	// on-off implementation
	virtual bool setOnline_(bool on);
	// application dependent config
	virtual bool configCtr_() = 0;
	// job implementation
	virtual bool arm_();
	virtual void disarm_();
};

class Rt6602Counter : public Rt6602CounterBase
{
	Q_OBJECT
protected:
	uint count0;
public:
	Rt6602Counter(const QString& name, Rt6602* parent, uint no);
	virtual ~Rt6602Counter(void);

	void takeReading_();

protected:
	// on-off implementation
	virtual bool configCtr_();
	// job implementation
	virtual void run();

public:
	Q_SLOT void takeReading();
	Q_SLOT bool filterSource(int fid);

};

class Rt6602PulseGenerator : public Rt6602CounterBase
{
	Q_OBJECT
	Q_PROPERTY(uint tlow READ tlow WRITE setTlow)
	Q_PROPERTY(uint thigh READ thigh WRITE setThigh)
protected:
	uint tlow_, thigh_;
public:
	Rt6602PulseGenerator(const QString& name, Rt6602* parent, uint no);
	virtual ~Rt6602PulseGenerator(void);

	//getters
	/// pulse low time in ms
	uint tlow() const { return tlow_; }
	/// pulse high time in ms
	uint thigh() const { return thigh_; }

	// setters
	void setTlow(uint t);
	void setThigh(uint t);

protected:
	// on-off implementation
	virtual bool configCtr_();
};

class Rt6602Dio : public RtDevice
{
	Q_OBJECT
	Q_PROPERTY(bool input READ input WRITE setInput)
	Q_PROPERTY(bool state READ state WRITE setState)
protected:
	pci6602::digital_io* dio_;
	bool input_;
public:
	Rt6602Dio(const QString& name, Rt6602* parent, uint no);
	virtual ~Rt6602Dio(void);

	Rt6602* parent() { return (Rt6602*)(RtObject::parent()); }
	pci6602::device* dev() { return parent()->dev(); }

	bool input() const { return input_; }
	bool state() const;
	void setInput(bool v);
	void setState(bool v);

protected:
	// on-off implementation
	virtual bool setOnline_(bool on);

};

#endif


