#pragma once

#include <QStringList>

class QConsoleHistory
{
	QStringList strings_;
	int pos_;
	QString token_;
	bool active_;
	int maxsize_;
public:
	QConsoleHistory(void);
	~QConsoleHistory(void);

	void add(const QString& str);

	const QString& currentValue() const
	{
		return pos_ == -1 ? token_ : strings_.at(pos_);
	}

	void activate(const QString& tk = QString());
	void deactivate() { active_ = false; }
	bool isActive() const { return active_; }

	bool move(bool dir);

	int indexOf(bool dir, int from) const;
};
