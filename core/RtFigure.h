#ifndef _RTFIGURE_H_
#define _RTFIGURE_H_

#include "RtObject.h"
#include <QPointer>

class RtVector;
class QWidget;
class RtPlotWidget;
class RtDataChannel;

class RtFigureItem : public RtObject
{
	Q_OBJECT
	Q_PROPERTY(bool visible READ visible WRITE setVisible)
public:
	RtFigureItem(const QString& name, const QString& descr, RtObject* parent);
	virtual ~RtFigureItem(void);

	virtual bool visible() const { return false; }
	void setVisible(bool v) { if (v) show_(); else hide_(); }

protected:
	virtual void show_() {}
	virtual void hide_() {}
public slots:
	void show() { show_(); }
	void hide() { hide_(); }
	QString pluginPaths();
	QString availableWidgets();
};

class RtFigureFolder : public RtFigureItem
{
	Q_OBJECT
public:
	RtFigureFolder(const QString& name, RtObject* parent);
	virtual ~RtFigureFolder(void);

	virtual bool visible() const;

protected:
	virtual void show_();
	virtual void hide_();
public slots:
	void newFigurePane(const QString& name, int rows=1, int cols=1);
	void newWidgetPane(const QString& name, const QString& uifile);
};

class RtFigurePane : public RtFigureItem
{
	Q_OBJECT

protected:
	QPointer<QWidget> widget_;

public:
	RtFigurePane(const QString& name, RtFigureFolder* parent, int rows = 1, int cols = 1);
	virtual ~RtFigurePane(void);
	virtual void detach();

	virtual bool visible() const;

protected:
	virtual void show_();
	virtual void hide_();

public slots:
	void setTitle(const QString& str);
};

class RtFigure : public RtFigureItem
{
	Q_OBJECT
	Q_PROPERTY(QString title READ title WRITE setTitle)
	Q_PROPERTY(QString xlabel READ xlabel WRITE setXLabel)
	Q_PROPERTY(QString ylabel READ ylabel WRITE setYLabel)
	Q_PROPERTY(bool autoScaleX READ autoScaleX WRITE setAutoScaleX)
	Q_PROPERTY(bool autoScaleY READ autoScaleY WRITE setAutoScaleY)
	Q_PROPERTY(bool timeScaleX READ timeScaleX WRITE setTimeScaleX)
	Q_PROPERTY(bool grid READ grid WRITE setGrid)

protected:
	QPointer<RtPlotWidget> widget_;
	bool timeScaleX_, grid_;

	//virtual void rtEvent(RtEvent* e);
public:
	RtFigure(const QString& name, RtFigurePane* parent, RtPlotWidget* widget);
	virtual ~RtFigure(void);
	virtual void detach();
	virtual bool visible() const;


	//getters
	QString title() const;
	QString xlabel() const;
	QString ylabel() const;
	bool autoScaleX() const;
	bool autoScaleY() const;
	bool timeScaleX() const { return timeScaleX_; }
	bool grid() const { return grid_; }

	//setters
	void setTitle(const QString& s);
	void setXLabel(const QString& s);
	void setYLabel(const QString& s);
	void setAutoScaleX(bool on);
	void setAutoScaleY(bool on);
	void setTimeScaleX(bool on);
	void setGrid(bool on);


public slots:
	void clear();
	void plot(const RtVector* x, const RtVector* y);
	void replot();
	void xlim(double x1, double x2, double dx=0);
	void ylim(double x1, double x2, double dx=0);
};

class RtWidgetPane : public RtFigureItem
{
	Q_OBJECT

protected:
	QPointer<QWidget> widget_;

public:
	RtWidgetPane(const QString& name, QWidget* w, RtFigureFolder* parent);
	virtual ~RtWidgetPane(void);
	virtual void detach();

	virtual bool visible() const;

protected:
	virtual void show_();
	virtual void hide_();

	//virtual void rtEvent(RtEvent* e);

	//QList<DisplayDelegate*> displayDelegates;

public slots:
	void setTitle(const QString& str);
	QWidget* widget(); 
	void bind(RtDataChannel* ch, QWidget* w);
	void bind(RtObject* obj, const QString& propertyName, QWidget* w, bool readOnly = false);
};




#endif
