#ifndef _RTDELEGATES_H_
#define _RTDELEGATES_H_

#include "RtObject.h"
#include <QPointer>
#include <QVariant>
#include <QWidget>
#include <QMetaProperty>

class RtDataChannel;

class WidgetVariant
{
public:
	enum WidgetType { qNone, qUnsupported, 
		qAbstractButton, qAbstractSlider, qSpinBox, qDoubleSpinBox, qLineEdit, qComboBox, 
		qLabel, qLCDNumber, qwtThermo };	

protected:
	QPointer<QWidget> widget_;
	WidgetType type_;

public:
	WidgetVariant(QWidget* w = 0);

	bool isReadable() const { return (type_>=qAbstractButton) && (type_<=qComboBox); }
	bool isText() const { return (type_==qLabel) || (type_==qLineEdit); }
	bool isValid() const { return (type_>qUnsupported); }
	bool isNull() const { return widget_.isNull(); }
 	bool canConvert(QVariant::Type t) const;
	WidgetType type() const { return type_; }
	QWidget* widget() const { return widget_.data(); }
	const char* widgetSignal() const;

	QVariant read();
	void write(const QVariant& v);

};

class DisplayDelegate : public QObject, public RtUpdateFlag
{
public:
	DisplayDelegate(QObject* parent, RtDataChannel* ch, QWidget* w);
	virtual ~DisplayDelegate();
protected:
	virtual void customEvent(QEvent*);
	void updateDisplay();

	WidgetVariant widget_;
	QPointer<RtDataChannel> channel_;
};

class PropertyDelegate : public QObject
{
	Q_OBJECT
public:
	PropertyDelegate(QObject* parent, RtObject* obj, QMetaProperty p, QWidget* w, bool readOnly);
	virtual ~PropertyDelegate();

protected:
	QPointer<RtObject> obj_;
	QMetaProperty property_;
	WidgetVariant widget_;
	bool readOnly_;

	QVariant getObjectValue();
	void setWidgetValue(const QVariant& v);
	void setObjectValue(const QVariant& v);

public slots:
	void objectPropertyChange();
	void widgetPropertyChange();
};

#endif

