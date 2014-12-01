#include "RtDataChannel.h"
#include "RtChannelWidget.h"

#include <QVBoxLayout>

RtChannelWidget::RtChannelWidget(RtDataChannel* ach, QWidget* parent) :
QWidget(parent), ch(ach)
{
	ui.setupUi(this);
	ui.name->setText(getChannelName());
	ui.value->setText(ch->formatedValue());

	connect(ch,SIGNAL(propertiesChanged()),this,SLOT(onPropertiesChanged()));

	RtEvent ev(RtEvent::WidgetAdd,this);
	QCoreApplication::sendEvent(ch, &ev);
}

RtChannelWidget::~RtChannelWidget(void)
{
	if (ch)
	{
		RtEvent ev(RtEvent::WidgetRemove,this);
		QCoreApplication::sendEvent(ch, &ev);
	}
}

QString RtChannelWidget::getChannelName()
{
	QString name;
	if (ch->signalName().isEmpty()) name = ch->fullName();
	else
	{
		name = ch->signalName();
		if (!ch->unit().isEmpty())
		{
			name += " (";
			name += ch->unit();
			name += ")";
		}
	}
	return name;
}

void RtChannelWidget::onPropertiesChanged()
{
	ui.name->setText(getChannelName());
}

void RtChannelWidget::customEvent ( QEvent * e )
{
	if (RtEvent::isRtEvent(e))
	{
		RtEvent* rte = (RtEvent*)e;
		switch (rte->rtType())
		{
		case RtEvent::ObjectDeleted:
			ch = 0;
			deleteLater();
			break;
		case RtEvent::ObjectUpdated:
			ui.value->setText(ch->formatedValue());
			resetUpdateFlag();
			break;
		}
	}
}

RtChannelViewer::RtChannelViewer(QWidget* parent) : QWidget(parent)
{
	vbox = new QVBoxLayout(this);
	vbox->setContentsMargins ( 3, 3, 3, 3 );
	//vbox->setSpacing(3);
	vbox->addStretch();
}

RtChannelViewer::~RtChannelViewer(void)
{
}

RtChannelWidget* RtChannelViewer::addChannel(RtDataChannel* ch)
{
	RtChannelWidget* w = new RtChannelWidget(ch,this);
	vbox->addWidget(w);
	//vbox->insertWidget(0,w);
	return w;
}
