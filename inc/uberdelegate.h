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
	void paint_aggregate(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &idx) const;

	void set_active_bg_color(QColor c) {m_active_bg_color = c;}
private:
	QColor m_active_bg_color;
};

#endif