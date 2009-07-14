#ifndef UBER_DELEGATE_H
#define UBER_DELEGATE_H

#include "dwarfmodel.h"
#include <QtGui>

class UberDelegate : public QStyledItemDelegate {
	Q_OBJECT
public:
	UberDelegate(QObject *parent = 0);
	QSize sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &idx) const;
	void paint(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &idx) const;

	void paint_header(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &idx) const;
	void paint_skill(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &idx) const;
private:
	DwarfModel::GROUP_BY m_group_by;
};

#endif