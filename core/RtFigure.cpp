#include "RtFigure.h"
#include "RtRoot.h"
#include "RtPlotWidget.h"
#include "RtVector.h"
#include "RtDataChannel.h"
#include "RtMainWindow.h"
#include "RtDelegates.h"

#include <QGridLayout>
#include <QCloseEvent>
#include <QtUiTools>
#include <QFile>
#include <QSet>

#include <qwt_text.h>

RtFigureItem::RtFigureItem(const QString& name, const QString& descr, RtObject* parent) :
RtObject(name,descr,parent)
{
}
RtFigureItem::~RtFigureItem(void)
{
}
QString RtFigureItem::pluginPaths()
{
	QUiLoader l;
	return l.pluginPaths().join("\n");
}
QString RtFigureItem::availableWidgets()
{
	QUiLoader l;
	return l.availableWidgets().join("\n");
}
//************************************************************************************//
RtFigureFolder::RtFigureFolder(const QString& name, RtObject* parent)
: RtFigureItem(name,"Figure folder",parent)
{
	canBeKilled_ = parent != (RtObject*)&root_;
}
RtFigureFolder::~RtFigureFolder(void)
{
}

void RtFigureFolder::show_()
{
	foreach(QObject* obj, children())
	{
		RtFigureItem* fig = qobject_cast<RtFigureItem*>(obj);
		if (fig) fig->show();
	}
}
void RtFigureFolder::hide_()
{
	foreach(QObject* obj, children())
	{
		RtFigureItem* fig = qobject_cast<RtFigureItem*>(obj);
		if (fig) fig->hide();
	}
}
void RtFigureFolder::newFigurePane(const QString& name, int rows, int cols)
{
	if (checkName(name))
	{
		RtFigurePane* fig = new RtFigurePane(name,this,rows,cols);
		createScriptObject(fig);
	}
}
void RtFigureFolder::newWidgetPane(const QString& name, const QString& uifile)
{
	if (!checkName(name)) return;

	QWidget* w;
	QFile file(uifile);
	file.open(QFile::ReadOnly);
	if (!file.isOpen())
	{
		throwScriptError(QString("The ui definition file %1 cannot be opened").arg(uifile));
		return;		
	}
	else 
	{
		QUiLoader loader;
		w = loader.load(&file);
		file.close();
		if (!w)
		{
			throwScriptError(QString("The file %1 is not a valid ui definition").arg(uifile));
			return;		
		}
	}

	RtWidgetPane* fig = new RtWidgetPane(name,w,this);
	createScriptObject(fig);
}

bool RtFigureFolder::visible() const
{
	foreach(QObject* obj, children())
	{
		RtFigureItem* fig = qobject_cast<RtFigureItem*>(obj);
		if (!fig->visible()) return false;
	}
	return true;
}

//************************************************************************************//
class NoCloseWidget : public QWidget
{
public:
	NoCloseWidget(QWidget* parent = 0) : QWidget(parent)
	{}
protected:
	virtual void closeEvent ( QCloseEvent * e )
	{
		parentWidget()->hide();
		e->ignore();
	}
};

RtFigurePane::RtFigurePane(const QString& name, RtFigureFolder* parent, int rows, int cols) :
RtFigureItem(name,"Rt Figure",parent)
{
	widget_ = new NoCloseWidget;
	QGridLayout* layout = new QGridLayout();

	int k = 1;
	for(int i=0; i<rows; ++i)
	{
		for(int j=0; j<cols; ++j)
		{
			RtPlotWidget* plotWidget = new RtPlotWidget(widget_);
			RtFigure* fig = new RtFigure(QString("fig%1").arg(k++),this,plotWidget);
			layout->addWidget(plotWidget,i,j);
		}
	}
	widget_->setLayout(layout);
	root_.mainWindow()->addFigureWindow(widget_);
	widget_->setWindowTitle(name);
	widget_->show();
}

RtFigurePane::~RtFigurePane(void)
{
}

void RtFigurePane::detach()
{
	if (widget_)
	{
		QWidget* w = widget_->parentWidget();
		if (w) delete w;
		else delete widget_;
	}
	RtFigureItem::detach();
}

void RtFigurePane::setTitle(const QString& str)
{
	if (widget_) widget_->setWindowTitle(str);
}

void RtFigurePane::show_()
{
	if (widget_)
	{
		QWidget* p = widget_->parentWidget();
		if (p) p->show();
	}
}

void RtFigurePane::hide_()
{
	if (widget_)
	{
		QWidget* p = widget_->parentWidget();
		if (p) p->hide();
	}
}

bool RtFigurePane::visible() const
{
	return widget_ && !(widget_->isHidden());
}


//************************************************************************************//
RtFigure::RtFigure(const QString& name, RtFigurePane* parent, RtPlotWidget* w) :
RtFigureItem(name,"Rt Figure",parent), timeScaleX_(false), widget_(w)
{
	canBeKilled_ = false;
}

RtFigure::~RtFigure(void)
{
}
void RtFigure::detach()
{
	RtFigureItem::detach();
}

bool RtFigure::visible() const
{
	return ((RtFigurePane*)parent())->visible();
}

void RtFigure::clear()
{
	if (widget_) widget_->clear();
}

void RtFigure::plot(const RtVector* x, const RtVector* y)
{
	if (x->size() != y->size())
	{
		throwScriptError("Vector must have equal size.");
		return;
	}
	widget_->plot(*x, *y);
}

void RtFigure::replot()
{
	widget_->replot();
}

//getters
QString RtFigure::title() const
{
	return widget_->title().text();
}
QString RtFigure::xlabel() const
{
	return widget_->axisTitle(QwtPlot::xBottom).text();
}
QString RtFigure::ylabel() const
{
	return widget_->axisTitle(QwtPlot::yLeft).text();
}
bool RtFigure::autoScaleX() const
{
	return widget_->axisAutoScale(QwtPlot::xBottom);
}
bool RtFigure::autoScaleY() const
{
	return widget_->axisAutoScale(QwtPlot::yLeft);
}

//setters
void RtFigure::setTitle(const QString& s)
{
	widget_->setTitle(s);
}
void RtFigure::setXLabel(const QString& s)
{
	widget_->setAxisTitle(QwtPlot::xBottom,s);
}
void RtFigure::setYLabel(const QString& s)
{
	widget_->setAxisTitle(QwtPlot::yLeft,s);
}
void RtFigure::setAutoScaleX(bool on)
{
	if (on) widget_->setAxisAutoScale(QwtPlot::xBottom);
}
void RtFigure::setAutoScaleY(bool on)
{
	if (on) widget_->setAxisAutoScale(QwtPlot::yLeft);
}
void RtFigure::setTimeScaleX(bool on)
{
	if (on==timeScaleX_) return;
	widget_->setTimeAxis(on);
	timeScaleX_ = on;
}
void RtFigure::setGrid(bool on)
{
	widget_->setGrid(on);
}

//slots
void RtFigure::xlim(double x1, double x2, double dx)
{
	widget_->setAxisScale(QwtPlot::xBottom,x1,x2,dx);
}
void RtFigure::ylim(double x1, double x2, double dx)
{
	widget_->setAxisScale(QwtPlot::yLeft,x1,x2,dx);
}
//************************************************************************************//

RtWidgetPane::RtWidgetPane(const QString& name, QWidget* w, RtFigureFolder* parent) :
RtFigureItem(name,"Rt widget pane",parent)
{
	widget_ = w;
	root_.mainWindow()->addDockPane(widget_);
	widget_->window()->setWindowTitle(name);
	widget_->window()->show();
}

RtWidgetPane::~RtWidgetPane(void)
{
}

void RtWidgetPane::detach()
{
	//foreach(DisplayDelegate* d, displayDelegates) { delete d; }
	if (widget_)
	{
		QWidget* w = widget_->parentWidget();
		if (w) delete w;
		else delete widget_;
	}
	RtFigureItem::detach();
}

void RtWidgetPane::setTitle(const QString& str)
{
	if (widget_ && (widget_->window())) 
		widget_->window()->setWindowTitle(str);
}

void RtWidgetPane::show_()
{
	if (widget_ && (widget_->window())) 
		widget_->window()->show();
}

void RtWidgetPane::hide_()
{
	if (widget_ && (widget_->window())) 
		widget_->window()->hide();
}

bool RtWidgetPane::visible() const
{
	return widget_ && (widget_->window()) && !(widget_->window()->isHidden());
}

QWidget* RtWidgetPane::widget()
{ 
	return (QWidget*)widget_; 
}

void RtWidgetPane::bind(RtDataChannel* ch, QWidget* w)
{
	if (ch && w)
	{
		DisplayDelegate* d = new DisplayDelegate(this,ch,w);
		//displayDelegates << d;
	}
}
void RtWidgetPane::bind(RtObject* obj, const QString& propertyName, QWidget* w, bool readOnly)
{
	if (!obj) 
	{
		return;
	}
	const QMetaObject* metaObj = obj->metaObject();
	int idx = metaObj->indexOfProperty(propertyName.toLatin1());
	if (idx<0) 
	{
		throwScriptError(
			QString("%1 is not a property of %2").arg(propertyName).arg(obj->objectName())
			);
		return;
	}
	QMetaProperty p = metaObj->property(idx);
	if(!w || !w->isWidgetType())
	{
		throwScriptError(
			QString("Invalid widget")
			);
		return;
	}
	WidgetVariant wv(w);
	if (!wv.canConvert(p.type()))
	{
		throwScriptError(
			QString("Property %1 (%2) is not compatible with widget %3 (%4)")
			.arg(propertyName)
			.arg(p.typeName())
			.arg(w->objectName())
			.arg(w->metaObject()->className())
			);
		return;
	}

	PropertyDelegate* d = new PropertyDelegate(this,obj,p,w,readOnly);


}



/*void RtWidgetPane::rtEvent(RtEvent* rte)
{
	RtFigureItem::rtEvent(rte);
}*/



//************************************************************************************//


