#ifndef UBER_DELEGATE_H
#define UBER_DELEGATE_H

#include "dwarfmodel.h"
#include <QtGui>

class UberDelegate : public QStyledItemDelegate {
	Q_OBJECT
public:
	UberDelegate(QObject *parent = 0);
	void paint(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &idx) const;
	void paint_skill(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &idx) const;
	void paint_aggregate(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &idx) const;
	//QSize sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &idx) const;

	QColor color_active_labor;
	QColor color_active_group;
	QColor color_inactive_group;
	QColor color_partial_group;
	QColor color_guides;
	QColor color_border;
	QColor color_dirty_border;
	QColor color_cursor;
};

#endif