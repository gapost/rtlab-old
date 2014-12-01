#ifndef _RTPLOTWIDGET_H_
#define _RTPLOTWIDGET_H_

#include "RtEvent.h"

#include <qwt_plot.h>

class RtVector;
class QwtPlotCurve;
class QwtPlotGrid;
class QwtPlotZoomer;
class QwtPlotPanner;
class QwtPlotPicker;

class RtPlotWidget : public QwtPlot, public RtUpdateFlag
{
public:
	RtPlotWidget(QWidget* parent = 0);
	virtual ~RtPlotWidget();

	void plot(const RtVector& x, const RtVector& y);
	void setTimeAxis(bool on);
	void setGrid(bool on);
	void clear();

	virtual QSize 	sizeHint () const	{ return QSize(10,10); }
	virtual QSize 	minimumSizeHint () const { return QSize(10,10); }

protected:
	virtual void closeEvent ( QCloseEvent * e );
	virtual void customEvent ( QEvent * e );
	void onVectorUpdated(const RtVector* v);
	void onVectorDeleted(const RtVector* v);

	QMap<const RtVector*, QwtPlotCurve*> vec2curve;

	QwtPlotGrid* grid;
	QwtPlotZoomer* zoomer;
	QwtPlotPanner* panner;
	QwtPlotPicker* picker;


	uint id_;
};

#endif
