#ifndef _RTAXIS_H_
#define _RTAXIS_H_

#include "RtDevice.h"

class RtAxis : public RtDevice
{
	Q_OBJECT
	Q_PROPERTY(int pos READ pos)
	Q_PROPERTY(int setPos READ setPos WRITE setSetPos)
	Q_PROPERTY(int limitUp READ limitUp WRITE setLimitUp)
	Q_PROPERTY(int limitDown READ limitDown WRITE setLimitDown)
	Q_PROPERTY(AxisMode mode READ mode WRITE setMode)
	Q_PROPERTY(int limitFlag READ limitFlag)
	Q_ENUMS(AxisMode)

public:
	enum AxisMode { 
		Idle = 0, 
		Positioning = 1,
		MoveUp = 2,
		MoveDown = 3
	};

protected:

	struct tAxis
	{
		int x,xset; // position in steps 
		short v; // velocity in Hz

		char inFlags;
		char outFlags;
		short command;
		short unused;

	};

    //tAxis ax_;
	AxisMode mode_;
	int lup_, ldown_, lflag_;

    virtual int rdstate_(tAxis& ax);
	virtual void home_();

    // RtJob implementation
    virtual bool arm_();
    virtual void run();

public:
	RtAxis(const QString& name, const QString& desc, RtObject* parent, 
		RtInterface* aifc,  int addr);
	virtual ~RtAxis(void);

	virtual void registerTypes(QScriptEngine* e);


	// getters
    int pos();
    int setPos();
	int limitUp() { return lup_; } 
	int limitDown() { return ldown_; } 
    AxisMode mode();
    int limitFlag();

	// setters
	void setSetPos(int v);
	void setLimitUp(int v);
	void setLimitDown(int v);
	void setMode(AxisMode v);

public slots:
	void home() { home_(); }
	void abort() { setMode(Idle); }
};

#endif

