#ifndef _RTCHANNELWIDGET_H_
#define _RTCHANNELWIDGET_H_

#include "RtEvent.h"

#include "ui_ChannelWidget.h"

class RtDataChannel;

class RtChannelWidget : public QWidget, public RtUpdateFlag
{
	Q_OBJECT
public:
	RtChannelWidget(RtDataChannel* ch, QWidget* parent = 0);
	~RtChannelWidget(void);
protected:
	virtual void customEvent ( QEvent * e );

	QString getChannelName();

public:
	Q_SLOT void onPropertiesChanged();

private:
	Ui::ChannelWidget ui;
	RtDataChannel* ch;
};

class QVBoxLayout;

class RtChannelViewer : public QWidget
{
public:
	RtChannelViewer(QWidget* parent);
	~RtChannelViewer(void);

	RtChannelWidget* addChannel(RtDataChannel* ch);
private:
	QVBoxLayout* vbox;
};


#endif


