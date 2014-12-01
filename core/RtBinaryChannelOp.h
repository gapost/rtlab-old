#ifndef _RTBINARYCHANNELOP_H_
#define _RTBINARYCHANNELOP_H_

#include "RtDataChannel.h"

#include <QPointer>

class RtBinaryChannelOp : public RtDataChannel
{
	Q_OBJECT
	Q_PROPERTY(OpType op READ op WRITE setOp)
	Q_ENUMS(OpType)

public:
	enum OpType {
		Add,
		Sub,
		Mul,
		Div,
		Diff
	};

protected:
	OpType op_;
	QPointer<RtDataChannel> left_, right_;
	double v1p_, v2p_;

protected:
	virtual void run();

public:
	RtBinaryChannelOp(const QString& name, RtObject* parent);
	virtual ~RtBinaryChannelOp(void);

	virtual void registerTypes(QScriptEngine* e);

	OpType op() const { return op_; }
	void setOp(OpType o) { op_ = o; }

public slots:
	bool setLeftChannel(RtDataChannel*);
	bool setRightChannel(RtDataChannel*);
};

class RtLinearCorrelator : public RtDataChannel
{
	Q_OBJECT

protected:
	averager_t buff_x_;
	int k;

	QPointer<RtDataChannel> x_, y_;

protected:
	virtual void run();

public:
	RtLinearCorrelator(const QString& name, RtObject* parent);
	virtual ~RtLinearCorrelator(void);

	virtual void setDepth(uint d); 

public slots:
	bool setXChannel(RtDataChannel*);
	bool setYChannel(RtDataChannel*);
	void clear();
};

#endif
