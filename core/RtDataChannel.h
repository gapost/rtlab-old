#ifndef _RTDATACHANNEL_H_
#define _RTDATACHANNEL_H_

#include "RtJob.h"
#include "filters.h"
#include "RtTypes.h"

class RtChannelWidget;
namespace mu
{
	class Parser;
}

/** Objects that represent a stream of numerical data, a signal.

Real time data is handeled in RtLab through the RtDataChannel class.
An instrument provides measurement data through RtDataChannel objects.
On-line filtering and all types of data processing is also done with RtDataChannel and its
descendants.

\ingroup RtBase
*/
class RtDataChannel : public RtJob
{
	Q_OBJECT
	/** User supplied name.
	Used when displaying the channel.
	*/
	Q_PROPERTY(QString signalName READ signalName WRITE setSignalName)
	/** User supplied unit of measurement.
	Used when displaying the channel.
	*/	
	Q_PROPERTY(QString unit READ unit WRITE setUnit)
	/** User supplied unit of measurement.
	Used when displaying the channel.
	*/	
	Q_PROPERTY(RtDoubleVector range READ range WRITE setRange)
	/** Channel offset.
	Applied imediately when data is inserted in the channel by the transformation y=a*x+b.
	b is the offset and a is the multiplier.
	*/	
	Q_PROPERTY(double offset READ offset WRITE setOffset)
	/** Channel multiplier.
	Applied imediately when data is inserted in the channel by the transformation y=a*x+b.
	b is the offset and a is the multiplier.
	*/	
	Q_PROPERTY(double multiplier READ multiplier WRITE setMultiplier)
	/** Type of on-line averaging.
	*/	
	Q_PROPERTY(AveragingType averaging READ averaging WRITE setAveraging)
	/** Forgetting factor value.
	Only used when this type of averaging is active.
	*/	
	Q_PROPERTY(double forgettingFactor READ forgettingFactor WRITE setForgettingFactor)
	/** Averaging depth.
	Number of past data values used in averaging.
	*/	
	Q_PROPERTY(uint depth READ depth WRITE setDepth)
	/** Channel memory used.
	Number of values stored in internal channel memory.
	*/	
	Q_PROPERTY(uint memsize READ memsize)
	/** True if valid data exist on the channel.
	If dataReady is true, then value() & std() return valid numbers.
	*/	
	Q_PROPERTY(bool dataReady READ dataReady)
	/** muParser Expression.
	If set the expression is executed on the channel data.
	*/
	Q_PROPERTY(QString parserExpression READ parserExpression WRITE setParserExpression)
    /** Display format.
     * One of the chars 'e', 'E', 'f', 'g', 'G' defining the number format
     * according to QString::number(double,format,precision)
     */
    Q_PROPERTY(QString format READ format WRITE setFormat)
    /** Display precision.
     * Set the number display precision
     * according to QString::number(double,format,precision)
     */
    Q_PROPERTY(int precision READ precision WRITE setPrecision)

public:
	/** Type of channel averaging.	
	*/	
	enum AveragingType {
		None, /**< No averaging. */
		Running, /**< Running (box) average. */
		Delta, /**< Running average for signals of alternating sign. */
		ForgettingFactor /**< Running average with forgetting (exponential weighting). */
	};

protected:
	QString signalName_, unit_;

	typedef averager<double> averager_t;
	AveragingType type_;
	averager_t av;
	double v_, dv_, offset_, multiplier_;
	RtDoubleVector range_;
    QString format_;
    int precision_;

	bool time_channel_, dataReady_;

	QPointer<RtChannelWidget> widget_;
	QPointer<RtDataChannel> inputChannel_;

	mu::Parser* parser_;

	virtual bool arm_();
	virtual void run();
	virtual void postRun();

	virtual void registerTypes(QScriptEngine* e);

public:
	RtDataChannel(const QString& name, const QString& desc, RtObject* parent);
	virtual ~RtDataChannel(void);

	virtual void detach();

	// getters
	QString signalName() const { return signalName_; }
	QString unit() const { return unit_; }
	RtDoubleVector range() const { return range_; }
	AveragingType averaging() const { return type_; }
	double forgettingFactor() const { return av.ff(); }
	double offset() const { return offset_; }
	double multiplier() const { return multiplier_; }
	uint memsize() const { return av.capacity(); }
	uint depth() const { return av.N(); }
	bool dataReady() const { return dataReady_; }
	QString parserExpression() const;
    QString format() const { return format_; }
    int precision() const { return precision_; }

	// setters
	void setSignalName(QString v);
	void setUnit(QString v);
	void setRange(const RtDoubleVector& v);
	void setOffset(double v);
	void setMultiplier(double v);
	void setAveraging(AveragingType t); 
	void setForgettingFactor(double v) { av.setff(v); } 
	virtual void setDepth(uint d); 
	void setParserExpression(const QString& s);
    void setFormat(const QString& v);
    void setPrecision(int v);


	bool isTimeChannel() const { return time_channel_; }
	void setTimeChannel(bool on) { time_channel_ = on; }


	void forceProcces();

	double last() const { return av.last(); }

	virtual QString formatedValue();

public slots:
	/** Insert a value into the channel. */
	void push(double v) { av << v;	}
	/** Clear internal channel memory.*/
	void clear();
	/** Hide channel widget from Channel Viewer.*/
	void hide();
	/** Show channel widget in Channel Viewer.*/
	void show();
	/** Get the current channel value. */
	double value() const { return v_; }
	/** Get the current channel value standard deviation. */
	double std() const { return dv_; }
	/** Set another channel to be the input for this channel */
	void setInputChannel(RtDataChannel*);
	/** Get the input channel */
	RtDataChannel* getInputChannel();
};

class RtTimeChannel : public RtDataChannel
{
	Q_OBJECT
protected:
	virtual void run();
public:
	RtTimeChannel(const QString& name, RtObject* parent);
	virtual ~RtTimeChannel(void);
};

class RtTestChannel : public RtDataChannel
{
	Q_OBJECT
	Q_PROPERTY(RtDoubleVector vector READ vector WRITE setVector)
protected:
	enum TestType { Random, Inc, Dec, Sin, Pulse };
	TestType type_;
	RtDoubleVector vec_;

	double v;

	virtual void run();
public:
	RtTestChannel(const QString& name, RtObject* parent);
	virtual ~RtTestChannel(void);
	RtDoubleVector vector() const { return vec_; }
	void setVector(const RtDoubleVector& v) { vec_ = v; }
public: // slots:
	Q_SLOT void setTestType(const QString& type);
	Q_SLOT void reset() { v=0; }
};


#endif
