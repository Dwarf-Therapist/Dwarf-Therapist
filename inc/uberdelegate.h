/*
Dwarf Therapist
Copyright (c) 2009 Trey Stout (chmod)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
#ifndef UBER_DELEGATE_H
#define UBER_DELEGATE_H

#include <QtGui>
#include "gridview.h"

class DwarfModel;
class DwarfModelProxy;

class UberDelegate : public QStyledItemDelegate {
	Q_OBJECT
public:
	UberDelegate(QObject *parent = 0);
	void paint(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &proxy_idx) const;
	
	//QSize sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &idx) const;

	void set_model(DwarfModel *model) {m_model = model;}
	void set_proxy(DwarfModelProxy *proxy) {m_proxy = proxy;}
	QRect adjust_rect(QRect r) const;

	QColor color_active_labor;
	QColor color_active_group;
	QColor color_inactive_group;
	QColor color_partial_group;
	QColor color_guides;
	QColor color_border;
	QColor color_dirty_border;
	QColor color_cursor;
	int cell_padding;

private:
	DwarfModel *m_model;
	DwarfModelProxy *m_proxy;

	void paint_cell(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &proxy_idx) const;
	
	void paint_grid(bool dirty, QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &proxy_idx) const;
	//! return the bg color that was painted
	QColor paint_bg(bool active, QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &proxy_idx) const;

	void paint_skill(int rating, QColor bg, QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &proxy_idx) const;
	void paint_labor(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &proxy_idx) const;
	void paint_aggregate(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &proxy_idx) const;
	
	private slots:
		void read_settings();
};

#endif