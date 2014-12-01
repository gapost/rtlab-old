#include "RtTableWidget.h"
#include "RtVector.h"
#include "RtTimeValue.h"

#include <QCoreApplication>
#include <QAbstractTableModel>
#include <QHeaderView>

class RtDataModel : public QAbstractTableModel
{
protected:
	int nrow,ncol;

	int countRows (  ) const
	{
		int n = 0;
		if (dataContainer_)
		{
			foreach(const RtVector* v, dataContainer_->vectors())
			{
				int m = v->size();
				if (m>n) n=m;
			}
		}
		return n;
	}
	int countColumns ( ) const
	{
		return dataContainer_ ? dataContainer_->vectors().size() : 0;
	}

public:
	RtDataContainer* dataContainer_;


	RtDataModel(RtDataContainer* dc, QObject* parent=0) : QAbstractTableModel(parent), dataContainer_(dc)
	{
		nrow = countRows();
		ncol = countColumns();
	}

	void updateData()
	{
		int new_nrow = countRows(), drow = new_nrow-nrow;
		int new_ncol  = countColumns(), dcol = new_ncol - ncol;

		if (dcol!=0)
		{
			if (dcol>0) beginInsertColumns(QModelIndex(),ncol,ncol+dcol-1);
			else beginRemoveColumns(QModelIndex(),ncol+dcol,ncol-1);
			ncol = new_ncol;
			if (dcol>0) endInsertColumns();
			else endRemoveColumns();
		}
		if (drow!=0)
		{
			if (drow>0) beginInsertRows(QModelIndex(),nrow,nrow+drow-1);
			else beginRemoveRows(QModelIndex(),nrow+drow,nrow-1);
			nrow = new_nrow;
			if (drow>0) endInsertRows();
			else endRemoveRows();
		}

		emit dataChanged(index(0,0),index(nrow-1,ncol-1));
	}

	virtual int rowCount ( const QModelIndex & parent = QModelIndex() ) const
	{
		Q_UNUSED ( parent )		
		return nrow;
	}
	virtual int columnCount ( const QModelIndex & parent = QModelIndex() ) const
	{
		Q_UNUSED ( parent )
		return ncol;
	}
	virtual QVariant data ( const QModelIndex & idx, int role = Qt::DisplayRole ) const
	{
		QVariant retval;
		if (idx.isValid() && role==Qt::DisplayRole && dataContainer_)
		{
			int i(idx.row()), j(idx.column());
			const QVector<RtVector*>& V = dataContainer_->vectors();
			if (j<V.size()) 
			{
				const RtVector& v = *(V[j]);
				if (i<v.size())
				{
					retval.setValue(
						v.time() ? RtTimeValue(v[i]).toString() : QString::number(v[i],'f',12)
						);
				}
			}
		}
		return retval;
	}
	virtual QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const
	{
		Q_UNUSED ( role )
		QVariant retval;
		if (role==Qt::DisplayRole)
			if (orientation==Qt::Horizontal && section<dataContainer_->vectors().size())
				retval.setValue(dataContainer_->vectors().at(section)->objectName());
			else 
				retval.setValue(QString::number(section+1));
		return retval;
	}
	//virtual bool insertColumns ( int column, int count, const QModelIndex & parent = QModelIndex() )
	//{
//		Q_ASSERT(column==columnCount());
	//	int cc = columnCount();
	//	beginInsertColumns(parent,column,column);
	//	endInsertColumns();
	//	return true;
	//}
};

///////////////////////////////////////////////////////////////////////
RtTableWidget::RtTableWidget(RtDataContainer* dc, QWidget* parent) : QTableView(parent)
{
	model_ = new RtDataModel(dc,this);

	RtEvent ev(RtEvent::WidgetAdd,this);
	foreach(RtVector* v, model_->dataContainer_->vectors())
	{
		QCoreApplication::sendEvent(v, &ev);
	}

	
	setModel(model_);

	connect(dc,SIGNAL(vectorAdded(int)),this,SLOT(replot()));
	connect(dc,SIGNAL(vectorRemoved(int)),this,SLOT(replot()));

}

RtTableWidget::~RtTableWidget(void)
{
}

void RtTableWidget::replot()
{
	resetUpdateFlag();
	model_->updateData();
}

void RtTableWidget::customEvent (QEvent *e)
{
	if (RtEvent::isRtEvent(e))
	{
		RtEvent* rte = (RtEvent*)e;
		RtVector* v = (RtVector*)(rte->qObject());
		if (v && rte->rtType()==RtEvent::ObjectUpdated)
		{
			replot();
		}
	}
	else RtTableWidget::customEvent(e);
}



