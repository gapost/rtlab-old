#include "RtBinaryChannelOp.h"
#include "linefit.h"

#include <cfloat>

#include <muParser.h>

#include "RtEnumHelper.h"
Q_SCRIPT_ENUM(OpType, RtBinaryChannelOp)

RtBinaryChannelOp::RtBinaryChannelOp(const QString& name, RtObject* parent) :
RtDataChannel(name,"binary operation channel",parent), 
op_(Add), v1p_(0.0), v2p_(0.0)
{
}

RtBinaryChannelOp::~RtBinaryChannelOp(void)
{
}

void RtBinaryChannelOp::registerTypes(QScriptEngine* e)
{
	qScriptRegisterOpType(e);	
	RtDataChannel::registerTypes(e);
}

void RtBinaryChannelOp::run()
{
	double v(0.); 

	if (left_ && right_ && left_->dataReady() && right_->dataReady())
	{
		double v1 = left_->value();
		double v2 = right_->value();
		switch(op_)
		{
		case Add: v = v1+v2; break;
		case Sub: v = v1-v2; break;
		case Mul: v = v1*v2; break;
		case Div: v = v1/v2; break;
		case Diff: v = (v2-v2p_)/(v1-v1p_); break;
		}
		v1p_ = v1;
		v2p_ = v2;

		push(v);
	}

	RtDataChannel::postRun();
}

bool RtBinaryChannelOp::setLeftChannel(RtDataChannel* ch)
{
    os::auto_lock L(comm_lock);
	left_ = ch;
	return true;
}

bool RtBinaryChannelOp::setRightChannel(RtDataChannel* ch)
{
    os::auto_lock L(comm_lock);
	right_ = ch;
	return true;
}
//////////////////////////////////////////////////////////////
RtLinearCorrelator::RtLinearCorrelator(const QString& name, RtObject* parent) :
RtDataChannel(name,"linear correlator channel",parent), buff_x_(1), k(0)
{
}

RtLinearCorrelator::~RtLinearCorrelator(void)
{
}
void RtLinearCorrelator::clear()
{
    os::auto_lock L(comm_lock);
	av.clear();
	buff_x_.clear();
	k=0;
}
bool RtLinearCorrelator::setXChannel(RtDataChannel* ch)
{
    os::auto_lock L(comm_lock);
	x_ = ch;
	clear();
	return true;
}

bool RtLinearCorrelator::setYChannel(RtDataChannel* ch)
{
    os::auto_lock L(comm_lock);
	y_ = ch;
	clear();
	return true;
}
void RtLinearCorrelator::setDepth(uint d)
{
	if ((d!=depth()) && d>0)
	{
		{
            os::auto_lock L(comm_lock);
			av.setN(d);
			buff_x_.setN(d);
			clear();
		}
		emit propertiesChanged();
	}
}
void RtLinearCorrelator::run()
{
	dataReady_ = false;

	if (y_ && y_->dataReady())
	{
		if (x_ && x_->dataReady())
		{
			av << (y_->value());
			buff_x_ << (x_->value());
            if (k < (int)depth()) k++;
			if (k>1)
			{
				linefit<double> fitobj(buff_x_.buff(),av.buff(),k);
				v_ = fitobj.b;
				dv_ = k>2 ? fitobj.sigb : 0;
				dataReady_ = true;
			}
		}
		else
		{
			av << (y_->value());
            if (k < (int)depth()) k++;
			if (k>1)
			{
				linefit<double> fitobj(av.buff(),k);
				v_ = fitobj.b;
				dv_ = k>2 ? fitobj.sigb : 0;
				dataReady_ = true;
			}
		}
	}


	if (dataReady_ )
	{
		if (parser_)
		{
			try
			{
			  v_ = parser_->Eval();
			}
			catch (mu::Parser::exception_type &e)
			{
			  //std::cout << e.GetMsg() << endl;
                //const mu::string_type& e.GetMsg();
                pushError("muParser",e.GetMsg().c_str());
                dataReady_ = false;
			}
		}
		else
		{
			v_ = multiplier_*v_ + offset_;
			dv_ = multiplier_*dv_ + offset_;
		}
		// check limits
                dataReady_ = dataReady_ && finite(v_) && (v_>range_[0]) && (v_<range_[1]);
		updateWidgets();
	}
	RtJob::run();
}
