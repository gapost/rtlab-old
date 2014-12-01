#ifndef _RTVECTOR_H_
#define _RTVECTOR_H_

#include "RtObject.h"
#include "RtDataBuffer.h"
#include "ivector.h"
#include "RtTypes.h"

#include <QStringList>
#include <QPointer>

class RtTableWidget;

class RtDataContainer : public RtObject
{
	Q_OBJECT
	Q_PROPERTY(VectorType vectorType READ vectorType WRITE setVectorType)
	Q_ENUMS(VectorType)

public:
	enum VectorType {
		Open,
		Fixed,
		Circular
	};

	virtual void registerTypes(QScriptEngine* e);

protected:
	VectorType type_;

	friend class DataBufferReceiver;
	DataBufferReceiver* receiver_;

	friend class RtVector;
	QVector<RtVector*> vectors_;
	void addVector(RtVector* v);
	void removeVector(RtVector* v);

	QPointer<RtTableWidget> widget_;

public:
	RtDataContainer(const QString& name, const QString& desc, RtObject* parent);
	virtual ~RtDataContainer(void);
	virtual void detach();

	VectorType vectorType() const { return type_; }
	void setVectorType(VectorType t);

	const QVector<RtVector*>& vectors() const { return vectors_; }
	QVector<RtVector*>& vectors() { return vectors_; }
	
public slots:
	void newVector(const QString& name, uint sz = 0);
	void newVector(const QStringList& names, uint sz = 0);
	void newDataFolder(const QString& name);
	bool isMatrix() const;
	bool setSourceBuffer(RtDataBuffer* b = 0);
	void textSave(const QString& fname) const;
	void setCapacity(unsigned int sz);
	void clear();
	void show();
	void hide();

signals:
	void vectorAdded(int);
	void vectorRemoved(int);

};

class RtVector : public RtObject
{
	Q_OBJECT
	Q_PROPERTY(uint capacity READ capacity WRITE setCapacity)
	Q_PROPERTY(uint size READ size)
	Q_PROPERTY(uint memsize READ memsize)
	Q_PROPERTY(RtDataContainer::VectorType type READ type WRITE setType)
	Q_PROPERTY(bool time READ time WRITE setTime)

protected:
	ivector<double> v_;
	bool time_;

	static ivector<double> fromStringList(const QStringList& S, bool* ok_=0);

	bool boundsValid_;
	double vmin_, vmax_;
	void calcBounds_();

	void contentsChanged();

public:
	RtVector(const QString& name, RtDataContainer* parent, unsigned int cp = 0);
	virtual ~RtVector(void);
	virtual void detach();

	// getters
	unsigned int capacity() const { return v_.cap(); }
	unsigned int size() const { return v_.size(); }
	unsigned int memsize() const { return v_.mem(); }
	RtDataContainer::VectorType type() const { return (RtDataContainer::VectorType)v_.type(); }
	bool time() const { return time_; }
	bool boundsValid() const { return boundsValid_; }

	// setters
	void setCapacity(unsigned int sz) 
	{
		if (sz<size()) contentsChanged();
		v_.setCapacity(sz); 
	}
	void setType(RtDataContainer::VectorType t);
	void setTime(bool on) { time_ = on; }

	double operator[](unsigned int i) const { return v_.get(i); }
	void calcBounds(double& vmin, double& vmax) const;

	void push(const ivector<double>& v);
	void push(const double* p, int m);

	const ivector<double>& data() const { return v_; }
	const double* data_ptr() const { return v_.data(); }

public slots:
	QString toStringList(QChar sep = ' ') const;
	RtDoubleVector toArray() const;
	void fromArray(const RtDoubleVector& S);
	bool isFull() const { return size()==capacity(); }
	double get(unsigned int i) const 
	{ 
		if (i>=0 && i<v_.size()) return v_.get(i);
		else throwScriptError("Vector index out of range");
		return 0.;
	}
	void push(double v);
	void linearfill(double v0, double v1, int n);
	void clear() { v_.clear(); updateWidgets(); }
	double vmin() const;
	double vmax() const;
	double mean() const;
	double std() const;

signals:
	void full();
};

class DataBufferReceiver : public QObject, public RtDataBuffer::DataListener
{
	Q_OBJECT
protected:
	virtual void dataReady_(int);
public:
	DataBufferReceiver(QObject* parent);
	bool setSource(RtDataBuffer* src);
	QPointer<RtDataBuffer> buff;
protected slots:
	void getData();
signals:
	void dataReady();
};



#endif


