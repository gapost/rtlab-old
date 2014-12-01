#include "RtVector.h"
#include "RtDataBuffer.h"
#include "RtEnumHelper.h"
#include "RtTableWidget.h"
#include "RtRoot.h"
#include "RtMainWindow.h"
#include <QFile>
#include <QTextStream>
#include <math.h>
#include <QDebug>

Q_SCRIPT_ENUM(VectorType,RtDataContainer)

RtDataContainer::RtDataContainer(const QString& name, const QString& desc, RtObject* parent) : 
RtObject(name,desc,parent), type_(Fixed)
{
	canBeKilled_ = (parent != (RtObject*)&root_);
	receiver_ = new DataBufferReceiver(this);
}
RtDataContainer::~RtDataContainer(void)
{
}
void RtDataContainer::detach()
{
	hide();
	receiver_->setSource(0);
	RtObject::detach();
}
void RtDataContainer::registerTypes(QScriptEngine* e)
{
	qScriptRegisterVectorType(e);
	RtObject::registerTypes(e);
}
void RtDataContainer::addVector(RtVector* v)
{
	vectors_.push_back(v);
	emit vectorAdded(vectors_.size()-1);
	//qDebug() << "Added vector" << v;
	//qDebug() << "Total vectors:" << vectors_.size();
}
void RtDataContainer::removeVector(RtVector* v)
{	
	//qDebug() << "Removing vector" << v;
	//qDebug() << "Total vectors:" << vectors_.size();
	int i = vectors_.indexOf(v);
	//Q_ASSERT(i!=-1);
	if (i!=-1) {
		vectors_.remove(i);
		emit vectorRemoved(i);
	}
}
void RtDataContainer::newVector(const QString& name, uint sz)
{
	if (!checkName(name)) return;
	RtVector* obj = new RtVector(name,this,sz);
	obj->setType(type_);
	createScriptObject(obj);
	if (widget_) obj->add_widget(widget_,widget_);
}
void RtDataContainer::setVectorType(VectorType t)
{
	type_ = t;
	foreach(RtVector* v, vectors_) v->setType(t);
}
void RtDataContainer::newVector(const QStringList& names, uint sz)
{
	foreach(const QString& name, names)
	{
		if (!checkName(name)) return;
	}
	foreach(const QString& name, names)
	{
		RtVector* obj = new RtVector(name,this,sz);
		obj->setType(type_);
		createScriptObject(obj);
		if (widget_) obj->add_widget(widget_,widget_);
	}
}
void RtDataContainer::newDataFolder(const QString& name)
{
	if (!checkName(name)) return;
	RtDataContainer* obj = new RtDataContainer(name,"",this);
	createScriptObject(obj);
}
bool RtDataContainer::isMatrix() const
{
	if (vectors_.isEmpty()) return false;
	unsigned int n;
	bool not_initialized = true;
	
	foreach(RtVector* v, vectors_)
	{
		if (not_initialized) 
		{
			n = v->size();
			not_initialized = false;
		}
		else if (n!=v->size()) return false;
	}

	return true;
}
bool RtDataContainer::setSourceBuffer(RtDataBuffer* b)
{
	return receiver_->setSource(b); 
}
void RtDataContainer::textSave(const QString& fname) const
{
	if (!isMatrix()) 
	{
		throwScriptError("Data has not matrix shape.");
		return;
	}

	QFile f(fname);
	if (f.open(QFile::WriteOnly | QFile::Truncate)) 
	{
		QTextStream out(&f);

		// write the names
		int i = 0, n;
		foreach(RtVector* v, vectors_)
		{
			if (i++) out << '\t';
			else n = v->size();
			out << v->objectName();
		}
		out << endl;

		for(int j=0; j<n; ++j)
		{
			i=0;
			foreach(RtVector* v, vectors_)
			{
				if (i++) out << '\t';
				double x = v->get(j);
				if (v->time()) x -= v->get(0);
				out << x;
			}
			out << endl;
		}
	}
	else throwScriptError("Cannot open file for writing.");
}
void RtDataContainer::setCapacity(unsigned int sz)
{
	foreach(RtVector* v, vectors_) v->setCapacity(sz);
}
void RtDataContainer::clear()
{
	foreach(RtVector* v, vectors_) v->clear();
}
void RtDataContainer::show()
{
	hide();
	widget_ = new RtTableWidget(this); //root_.mainWindow()->channelViewer()->addChannel(this);
	root_.mainWindow()->addFigureWindow(widget_);
	widget_->setWindowTitle(fullName());
	widget_->show();
}
void RtDataContainer::hide()
{
	if (widget_) 
	{
		QWidget* w = widget_->parentWidget();
		if (w) delete w;
		else delete widget_;
	}
	widget_ = 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
RtVector::RtVector(const QString& name, RtDataContainer* parent, unsigned int sz) : 
RtObject(name,"",parent), v_(sz), time_(false), boundsValid_(false)
{
	Q_ASSERT(parent);
	parent->addVector(this);
}
RtVector::~RtVector(void)
{
}
void RtVector::detach()
{
	((RtDataContainer*)parent())->removeVector(this);
	RtObject::detach();
}
void RtVector::setType(RtDataContainer::VectorType t)
{
	if ((int)t==-1)
	{
		QString msg(
			"Invalid vector type specification. Availiable options: "
			"Open, Fixed, Circular"
			);
		return;
	}
	
	v_.setType((ivector<double>::iVectorType)t);
	emit propertiesChanged();
}
void RtVector::contentsChanged()
{
	boundsValid_ = false;
	updateWidgets();
	emit propertiesChanged();
}
ivector<double> RtVector::fromStringList(const QStringList& S, bool* ok_)
{
	ivector<double> V(S.size());
	bool ok;
	foreach(const QString& s, S)
	{
		double v = s.toDouble(&ok);
		if (ok) V.push(v);
		else break;
	}
	if (ok_) *ok_ = ok;
	return V;
}
void RtVector::calcBounds_()
{
	if (int n = v_.size())
	{
		vmin_ = vmax_ = v_[0];
		if (time_)
		{
			if (v_[0]<v_[n-1]) vmax_ = v_[n-1];
			else vmin_ = v_[n-1]; 
		}
		else
		{
			for(int i=1; i<n; ++i)
			{
				if (v_[i]>vmax_) vmax_=v_[i];
				if (v_[i]<vmin_) vmin_=v_[i];
			}
		}
	}
	else vmin_=vmax_=0;
	boundsValid_ = true;
}
void RtVector::calcBounds(double& v0, double& v1) const
{
	if (!boundsValid_) const_cast<RtVector*>(this)->calcBounds_();
	v0 = vmin_; v1 = vmax_; 
	return; 
}
QString RtVector::toStringList(QChar sep) const
{
	QString S;
	int n=v_.size(), i=0;
	if (n) S += QString::number(v_.get(i++));
	while(i<n) 
	{
		S += sep;
		S += QString("%1").arg(v_.get(i++));
	}
	return S;
}
RtDoubleVector RtVector::toArray() const
{
	RtDoubleVector S(v_.size());
	for(int i=0; i<v_.size(); ++i) S[i] = v_[i];
	return S;
}
void RtVector::push(double v) 
{
	v_.push(v); 
	contentsChanged();
	if (isFull()) emit full();
}
void RtVector::fromArray(const RtDoubleVector& v)
{
	int n = v.size();
	v_.setCapacity(n);
	v_.clear();
	for(int i=0; i<n; ++i) v_.push(v[i]);
	contentsChanged();
	if (isFull()) emit full();
}
void RtVector::push(const double* p, int m)
{
	for(int i=0; i<m; ++i) v_.push(*p++);
	contentsChanged();
	if (isFull()) emit full();
}
/*void RtVector::push(const QStringList& S)
{
	bool ok;
	ivector<double> s = fromStringList(S,&ok);
	if (ok) push(s);
	else throwScriptError("Conversion from script array to vector not possible");
}*/
void RtVector::linearfill(double v0, double v1, int n)
{
	if (n<2) throwScriptError("# of points must be > 2");
	v_.setCapacity(n);
	v_.clear();
	double dv = (v1-v0)/(n-1);
	for(int i=0; i<n; ++i) v_.push(v0+i*dv);
	contentsChanged();
}
double RtVector::vmin() const
{
	double x1,x2;
	calcBounds(x1,x2);
	return x1;
}
double RtVector::vmax() const
{
	double x1,x2;
	calcBounds(x1,x2);
	return x2;
}
double RtVector::mean() const
{
	double s(0.0);
	int n(v_.size());
	for(int i=0; i<n; ++i) s += v_[i];
	return s/n;
}
double RtVector::std() const
{
	double s1(0.0), s2(0.0);
	int n(v_.size());
	for(int i=0; i<n; ++i) { s1 += v_[i]; s2 += v_[i]*v_[i]; }
	s1 /= n;
	s2 /= n;
	s1 = s2 - s1*s1;
	if (s1<=0.0) return 0.0;
	else return sqrt(s1);
}
//////////////////////////////////////////////////////////////////////////////
DataBufferReceiver::DataBufferReceiver(QObject* parent) : QObject(parent)
{
}

void DataBufferReceiver::dataReady_(int n)
{
	if (n==1) emit dataReady();
}

bool DataBufferReceiver::setSource(RtDataBuffer* src)
{
	if (!buff.isNull())
	{
		buff->remove_listener(this);
		buff = 0;
		QObject::disconnect(this, SIGNAL(dataReady()), this, SLOT(getData()));
	}
	if (src)
	{
		buff = src;
		buff->add_listener(this);
		QObject::connect(this, SIGNAL(dataReady()), this, SLOT(getData()), Qt::QueuedConnection);
	}
	return true;

}

void DataBufferReceiver::getData()
{
	QVector<RtVector*> vecs = qobject_cast<RtDataContainer*>(parent())->vectors_;

	while(!isEmpty())
	{
		PacketReader<double> pckt = pop();
		int m = pckt.depth();
		int n = qMin(vecs.size(),pckt.width());
		for(int i=0; i<n; ++i) vecs[i]->push(pckt[i], m);
	}
}


