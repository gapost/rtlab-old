#ifndef _RTTABLEWIDGET_H_
#define _RTTABLEWIDGET_H_

#include "RtEvent.h"

#include <QTableView>

class RtDataContainer;
class RtDataModel;

class RtTableWidget : public QTableView, public RtUpdateFlag
{
	Q_OBJECT
protected:
	RtDataModel* model_;
public:
	RtTableWidget(RtDataContainer* dc, QWidget* parent = 0);
	virtual ~RtTableWidget(void);
protected:
	virtual void customEvent ( QEvent * e );
public slots:
	void replot();
	//void insertColumn(int i);
	//void removeColumn(int i);
};

#endif



