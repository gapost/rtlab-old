#include "QConsoleHistory.h"

#include <QFile>
#include <QTextStream>

#define HISTORY_FILE "_command_history.lst"

QConsoleHistory::QConsoleHistory(void) : pos_(0), active_(false), maxsize_(10000)
{
	QFile f(HISTORY_FILE);
	if (f.open(QFile::ReadOnly)) {
		QTextStream is(&f);
		while(!is.atEnd()) add(is.readLine());
	}
}
QConsoleHistory::~QConsoleHistory(void)
{
	QFile f(HISTORY_FILE);
	if (f.open(QFile::WriteOnly | QFile::Truncate)) {
		QTextStream os(&f);
		int n = strings_.size();
		while(n>0) os << strings_.at(--n) << endl;
	}
}

void QConsoleHistory::add(const QString& str)
{
	if (strings_.size() == maxsize_) strings_.pop_back();
	strings_.push_front(str);
	active_ = false;
}

void QConsoleHistory::activate(const QString& tk)
{
	active_ = true;
	token_ = tk;
	pos_ = -1;
}

bool QConsoleHistory::move(bool dir)
{
	if (active_)
	{
		int next = indexOf ( dir, pos_ );
		if (pos_!=next) 
		{
			pos_=next;
			return true;
		}
		else return false;
	}
	else return false;
}

int QConsoleHistory::indexOf(bool dir, int from) const
{
	int i = from, to = from;
	if (dir)
	{
		while(i < strings_.size()-1)
		{
			const QString& si = strings_.at(++i);
			if (si.startsWith(token_)) { return i; }
		}				
	}
	else
	{
		while(i > 0)
		{
			const QString& si = strings_.at(--i);
			if (si.startsWith(token_)) { return i; }
		}	
		return -1;
	}
	return to;
}
