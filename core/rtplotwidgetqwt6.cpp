#include "RtPlotWidget.h"
#include "RtVector.h"
#include <QCloseEvent>
#include <QCoreApplication>
#include <QSet>
#include <QDateTime>

#include <qwt_plot_curve.h>
#include <qwt_series_data.h>
#include <qwt_scale_draw.h>
#include <qwt_plot_grid.h>
#include <qwt_scale_engine.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_picker.h>
#include <qwt_math.h>

#include <math.h>

// #define QwtDoublePoint QPointF

class FormattedPicker : public QwtPlotPicker
{
public:
    FormattedPicker(int xAxis, int yAxis,
        RubberBand rubberBand, DisplayMode trackerMode,
        QWidget *pc) : QwtPlotPicker(xAxis,yAxis,rubberBand,trackerMode,pc)
    {
    }
protected:
    virtual QwtText trackerTextF	(	const QPointF & 	pos	 ) 	 const
    {
        QwtText lx = plot()->axisScaleDraw(QwtPlot::xBottom)->label(pos.x());
        QwtText ly = plot()->axisScaleDraw(QwtPlot::yLeft)->label(pos.y());
        QString S( lx.text() );
        S += QChar(',');
        S += ly.text();
        return S;
    }
};

class Zoomer: public QwtPlotZoomer
{
public:
    Zoomer(int xAxis, int yAxis, QWidget *canvas):
        QwtPlotZoomer(xAxis, yAxis, canvas)
    {
        //setMaxStackDepth(1);
    }
protected:
    virtual void begin ()
    {
        //const QwtDoubleRect &rect = scaleRect();
        //const QwtDoubleRect &brect = zoomBase();
        if (zoomRectIndex()==0)
        {
            setZoomBase(false);

            const QRectF &rect = this->scaleRect();
            QwtPlot *plt = plot();
            if (plt->axisAutoScale(QwtPlot::xBottom))
            {
                plt->setAxisScale(QwtPlot::xBottom,rect.left(),rect.right());
            }
            if (plt->axisAutoScale(QwtPlot::yLeft))
            {
                plt->setAxisScale(QwtPlot::yLeft,rect.top(),rect.bottom());
            }
        }

        QwtPlotZoomer::begin();
    }
    virtual bool end (bool ok=true)
    {
        return QwtPlotZoomer::end(ok);
    }
    virtual void widgetMouseDoubleClickEvent (QMouseEvent *)
    {
        QwtPlot *plt = plot();
        if ( !plt ) return;
        plt->setAxisAutoScale(QwtPlot::xBottom);
        plt->setAxisAutoScale(QwtPlot::yLeft);

        setZoomStack(zoomStack(),0); //QStack<QwtDoubleRect>());
    }
};


class TimeScaleDraw: public QwtScaleDraw
{
public:
    TimeScaleDraw()
    {
    }
    virtual QwtText label(double v) const
    {
        uint s = (uint)v;
        return QDateTime::fromTime_t(s).time().toString("hh:mm:ss");
    }
};

class SciScaleDraw: public QwtScaleDraw
{
public:
    SciScaleDraw()
    {
    }
    virtual QwtText label(double v) const
    {
        return QString::number(v,'g');
    }
};

class TimeScaleEngine: public QwtLinearScaleEngine
{
protected:
    static double conversion_factor(double T)
    {
        double C = 1.; // conversion const
        if (T<=60) C = 1.; // secs up to 1 min
        else if (T<=3600) C = 60; // mins up to 1hr
        else if (T<=24*3600) C = 60*60; // hr up to 1 day
        else C = 24*60*60; // else in days
        return C;
    }
public:
    virtual void autoScale(int maxSteps,
        double &x1, double &x2, double &stepSize) const
    {
        double C = conversion_factor(fabs(x2-x1));
        x1 /= C; x2 /= C; // convert to  s,m,hr,d ...
        QwtLinearScaleEngine::autoScale(maxSteps,x1,x2,stepSize);
        x1 *= C; x2 *= C; stepSize *= C; // convert to s
    }
};


class RtPlotData : public QwtSeriesData< QPointF >
{
    const RtVector& vx;
    const RtVector& vy;
    size_t sz;
public:
    RtPlotData(const RtVector& x, const RtVector& y) : vx(x), vy(y)
    {
        sz = qMin(vx.size(),vy.size());
        double x1,x2,y1,y2;
        vx.calcBounds(x1,x2);
        vy.calcBounds(y1,y2);
    }
    RtPlotData(const RtPlotData& other) : vx(other.vx), vy(other.vy), sz(other.sz)
    {
    }
    virtual ~RtPlotData()
    {
    }

    RtPlotData *copy() const
    {
        RtPlotData* cc = new RtPlotData(*this);
        return cc;
    }

    virtual size_t size() const { return sz; }
    virtual QPointF sample( size_t i ) const { return QPointF(vx[i],vy[i]); }

    double x(size_t i) const { return vx[i]; }
    double y(size_t i) const { return vy[i]; }

    virtual QRectF boundingRect() const
    {
        double x1,x2,y1,y2;
        const_cast<RtPlotData*>(this)->sz = qMin(vx.size(),vy.size());
        vx.calcBounds(x1,x2);
        vy.calcBounds(y1,y2);
        return QRectF(x1,y1,x2-x1,y2-y1);
    }

    void update(const RtPlotWidget* w, const RtVector* v)
    {
        Q_UNUSED(v);
        sz = qMin(vx.size(),vy.size());
        //if (v==&vx && w->axisAutoScale(QwtPlot::xBottom)) vx.calcBounds(x1,x2);
        //else if (v==&vy && w->axisAutoScale(QwtPlot::yLeft)) vy.calcBounds(y1,y2);
        double x1,x2,y1,y2;
        if (w->axisAutoScale(QwtPlot::xBottom)) vx.calcBounds(x1,x2);
        if (w->axisAutoScale(QwtPlot::yLeft)) vy.calcBounds(y1,y2);

    }
};

RtPlotWidget::RtPlotWidget(QWidget* parent) : QwtPlot(parent), id_(0)
{
    setCanvasBackground(QColor(Qt::white));

    QFont font;
    font = axisFont(QwtPlot::xBottom);
    font.setPointSize(8);
    setAxisFont(QwtPlot::xBottom,font);
    setAxisFont(QwtPlot::yLeft,font);

    QwtText txt;
    txt = title();
    font = txt.font();
    font.setPointSize(10);
    txt.setFont(font);
    setTitle(txt);

    txt = axisTitle(QwtPlot::xBottom);
    font = txt.font();
    font.setPointSize(9);
    txt.setFont(font);
    setAxisTitle(QwtPlot::xBottom,txt);
    setAxisTitle(QwtPlot::yLeft,txt);


    grid = new QwtPlotGrid;
    grid->setMajorPen(QPen(Qt::gray, 0, Qt::DotLine));
    //grid->setMinPen(QPen(Qt::gray, 0 , Qt::DotLine));
    grid->attach(this);

    setAxisScaleDraw(QwtPlot::xBottom, new SciScaleDraw());
    setAxisScaleDraw(QwtPlot::yLeft, new SciScaleDraw());

    zoomer = new Zoomer( QwtPlot::xBottom, QwtPlot::yLeft, canvas());
    zoomer->setRubberBand(QwtPicker::RectRubberBand);
    zoomer->setRubberBandPen(QPen(Qt::darkGray,0,Qt::DashLine));

    //zoomer->setSelectionFlags(QwtPicker::DragSelection | QwtPicker::CornerToCorner);
    zoomer->setTrackerMode(QwtPicker::AlwaysOff);

        // RightButton: zoom out by 1
        // Ctrl+RightButton: zoom out to full size


    zoomer->setMousePattern(QwtEventPattern::MouseSelect2,
            Qt::RightButton, Qt::ControlModifier);
    zoomer->setMousePattern(QwtEventPattern::MouseSelect3,
            Qt::RightButton);

    panner = new QwtPlotPanner(canvas());
    panner->setMouseButton(Qt::LeftButton,Qt::ShiftModifier);

    picker = new FormattedPicker(QwtPlot::xBottom, QwtPlot::yLeft,
//        QwtPicker::PointSelection | QwtPicker::DragSelection,
        QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn,
        canvas());
    picker->setRubberBand(QwtPicker::CrossRubberBand);

    setAutoReplot(true);

}

RtPlotWidget::~RtPlotWidget(void)
{
    RtEvent ev(RtEvent::WidgetRemove,this);
    QList<const RtVector*> vecs = vec2curve.uniqueKeys();
    foreach(const RtVector* v, vecs)
    {
        QCoreApplication::sendEvent(const_cast<RtVector*>(v), &ev);
    }
}

void RtPlotWidget::setGrid(bool on)
{
    grid->enableX(on);
    grid->enableY(on);
}

void RtPlotWidget::closeEvent ( QCloseEvent * e )
{
    //QwtPlot::closeEvent(e);
    parentWidget()->hide();
    e->ignore();
}

void RtPlotWidget::plot(const RtVector& x, const RtVector& y)
{
    static const Qt::GlobalColor eight_colors[8] =
    {
        Qt::blue,
        Qt::red,
        Qt::darkGreen,
        Qt::magenta,
        Qt::darkBlue,
        Qt::darkMagenta,
        Qt::darkCyan,
        Qt::darkRed
    };

    QwtPlotCurve* curve = new QwtPlotCurve;
    curve->setData(new RtPlotData(x,y));

    curve->setPen(QPen(QColor(eight_colors[id_++ & 0x07])));

    curve->attach(this);

    vec2curve.insertMulti(&x,curve);
    vec2curve.insertMulti(&y,curve);
    RtEvent ev(RtEvent::WidgetAdd,this);
    QCoreApplication::sendEvent(const_cast<RtVector*>(&x), &ev);
    QCoreApplication::sendEvent(const_cast<RtVector*>(&y), &ev);

    replot();
}

void RtPlotWidget::customEvent (QEvent *e)
{
    if (RtEvent::isRtEvent(e))
    {
        RtEvent* rte = (RtEvent*)e;
        RtVector* v = (RtVector*)(rte->qObject());
        if (v)
        {
            switch (rte->rtType())
            {
            case RtEvent::ObjectDeleted:
                onVectorDeleted(v);
                break;
            case RtEvent::ObjectUpdated:
                onVectorUpdated(v);
                break;
            }
        }
    }
    else RtPlotWidget::customEvent(e);
}

void RtPlotWidget::onVectorUpdated(const RtVector* v)
{
    // get all curves with reference to v, no duplicates (this is why a Set is used)
    QSet<QwtPlotCurve*> curves = vec2curve.values(v).toSet();
    // check if this is a false message
    if (curves.empty()) return;
    // update curves
    foreach(QwtPlotCurve* curve, curves)
    {
        RtPlotData* data = (RtPlotData*)(curve->data());
        data->update(this,v);
        curve->itemChanged();
    }
    // reset flag & replot
    resetUpdateFlag();
    //replot();
}

void RtPlotWidget::onVectorDeleted(const RtVector* v)
{
    // get all curves with reference to v, no duplicates (this is why a Set is used)
    QSet<QwtPlotCurve*> curves = vec2curve.values(v).toSet();
    // check if this is a false message
    if (curves.empty()) return;
    // detach and delete curves
    foreach(QwtPlotCurve* curve, curves)
    {
        // remove reference to the curve
        QMap<const RtVector*, QwtPlotCurve*>::iterator i = vec2curve.begin();
        while(i!=vec2curve.end())
        {
            if (i.value()==curve) i = vec2curve.erase(i);
            else ++i;
        }
        curve->detach();
        id_--;
        delete curve;
    }
    // reset flag & replot
    resetUpdateFlag();
    replot();
}


void RtPlotWidget::setTimeAxis(bool on)
{
    if (on)
    {
        setAxisScaleEngine(QwtPlot::xBottom, new TimeScaleEngine());
        setAxisScaleDraw(QwtPlot::xBottom, new TimeScaleDraw());
    }
    else
    {
        setAxisScaleEngine(QwtPlot::xBottom, new QwtLinearScaleEngine());
        setAxisScaleDraw(QwtPlot::xBottom, new SciScaleDraw());
    }
}

void RtPlotWidget::clear()
{
    detachItems(QwtPlotItem::Rtti_PlotCurve,true);
    RtEvent ev(RtEvent::WidgetRemove,this);
    QList<const RtVector*> vecs = vec2curve.uniqueKeys();
    foreach(const RtVector* v, vecs)
    {
        QCoreApplication::sendEvent(const_cast<RtVector*>(v), &ev);
    }
    vec2curve.clear();
    id_ = 0;
    replot();
}

