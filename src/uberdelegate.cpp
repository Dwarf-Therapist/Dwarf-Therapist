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
#include <QtGui>
#include "uberdelegate.h"
#include "dwarfmodel.h"
#include "dwarf.h"

UberDelegate::UberDelegate(QObject *parent)
	: QStyledItemDelegate(parent)
	, m_allow_grid_focus(false)
{
}

void UberDelegate::paint(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &proxy_idx) const {
	if (!proxy_idx.isValid()) {
		return;
	}
	QModelIndex idx = m_proxy->mapToSource(proxy_idx);
	if (proxy_idx.column() == 0) { // we never do anything with the 0 col...
		QStyledItemDelegate::paint(p, opt, idx); // always lay the "base coat"
		return;
	}

	if (m_model->current_grouping() == DwarfModel::GB_NOTHING) {
		paint_skill(p, opt, proxy_idx);
	} else {
		//QModelIndex idx = m_proxy->mapToSource(proxy_idx);
		QModelIndex first_col = m_model->index(idx.row(), 0, idx.parent());
		if (m_model->hasChildren(first_col)) { // skill item (under a group header)
			paint_aggregate(p, opt, idx);
		} else {
			paint_skill(p, opt, idx);
		}
	}

	if (idx.column() == m_model->selected_col()) {
		p->save();
		p->setPen(QPen(color_guides));
		p->drawLine(opt.rect.topLeft(), opt.rect.bottomLeft());
		p->drawLine(opt.rect.topRight(), opt.rect.bottomRight());
		p->restore();
	}

	if (m_allow_grid_focus && opt.state & QStyle::State_HasFocus) { // cursor
		p->save(); // border last
		p->setBrush(Qt::NoBrush);
		p->setPen(QPen(color_cursor, 2));
		QRect r = opt.rect.adjusted(1, 1, -1 ,-1);
		p->drawLine(r.topLeft(), r.bottomRight());
		p->drawLine(r.topRight(), r.bottomLeft());
		//p->drawRect(opt.rect.adjusted(0, 0, -1, -1));
		p->restore();
	}
}

void UberDelegate::paint_skill(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &idx) const {
	GameDataReader *gdr = GameDataReader::ptr();
	short rating = idx.data(DwarfModel::DR_RATING).toInt();
	
	Dwarf *d = m_model->get_dwarf_by_id(idx.data(DwarfModel::DR_ID).toInt());
	Q_ASSERT(d);

	bool skip_border = false;
	int labor_id = idx.data(DwarfModel::DR_LABOR_ID).toInt();
	bool enabled = d->is_labor_enabled(labor_id);
	bool dirty = d->is_labor_state_dirty(labor_id);

	p->save();
	if (enabled) {
		p->fillRect(opt.rect, QBrush(color_active_labor));
	} else {
		p->fillRect(opt.rect, QBrush(gdr->get_color(QString("labors/%1/color").arg(idx.column() - 1))));
	}
	p->restore();
	
	// draw rating
	if (rating == 15) {
		// draw diamond
		p->save();
		p->setRenderHint(QPainter::Antialiasing);
		p->setPen(Qt::gray);
		p->setBrush(QBrush(Qt::red));

		QPolygonF shape;
		shape << QPointF(0.5, 0.1) //top
			<< QPointF(0.75, 0.5) // right
			<< QPointF(0.5, 0.9) //bottom
			<< QPointF(0.25, 0.5); // left

		p->translate(opt.rect.x() + 2, opt.rect.y() + 2);
		p->scale(opt.rect.width()-4, opt.rect.height()-4);
		p->drawPolygon(shape);
		p->restore();

	} else if (rating < 15 && rating > 10) {
		// TODO: try drawing the square of increasing size...
		float size = 0.65f * (rating / 10.0f);
		float inset = (1.0f - size) / 2.0f;

		p->save();
		p->translate(opt.rect.x(), opt.rect.y());
		p->scale(opt.rect.width(), opt.rect.height());
		p->fillRect(QRectF(inset, inset, size, size), QBrush(QColor(0x888888)));
		p->restore();
	} else if (rating > 0) {
		float size = 0.65f * (rating / 10.0f);
		float inset = (1.0f - size) / 2.0f;

		p->save();
		p->translate(opt.rect.x(), opt.rect.y());
		p->scale(opt.rect.width(), opt.rect.height());
		p->fillRect(QRectF(inset, inset, size, size), QBrush(QColor(0xAAAAAA)));
		p->restore();
	}

	if (dirty || !skip_border) {
		p->save(); // border last
		p->setBrush(Qt::NoBrush);
		if (dirty) {
			p->setPen(QPen(QColor(color_dirty_border), 1));
			p->drawRect(opt.rect.adjusted(0, 0, -1, -1));
		} else if (opt.state & QStyle::State_Selected) {
			p->setPen(color_border);
			p->drawRect(opt.rect);
			p->setPen(color_guides);
			p->drawLine(opt.rect.topLeft(), opt.rect.topRight());
			p->drawLine(opt.rect.bottomLeft(), opt.rect.bottomRight());
			//p->drawRect(opt.rect.adjusted(0,0,-1,-1));
		} else {
			p->setPen(color_border);
			p->drawRect(opt.rect);
		}
		p->restore();
	}
}

void UberDelegate::paint_aggregate(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &proxy_idx) const {
	if (!proxy_idx.isValid()) {
		return;
	}
	QModelIndex idx = m_proxy->mapFromSource(proxy_idx);
	QModelIndex first_col = m_model->index(idx.row(), 0, idx.parent());
	if (!first_col.isValid()) {
		return;
	}
	QStandardItem *item = m_model->itemFromIndex(first_col);

	QString group_name = idx.data(DwarfModel::DR_GROUP_NAME).toString();
	int labor_id = idx.data(DwarfModel::DR_LABOR_ID).toInt();

	int dirty_count = 0;
	int enabled_count = 0;
	const QMap<QString, QVector<Dwarf*>> *groups = m_model->get_dwarf_groups();
	foreach(Dwarf *d, groups->value(group_name)) {
		if (d->is_labor_enabled(labor_id))
			enabled_count++;
		if (d->is_labor_state_dirty(labor_id))
			dirty_count++;
	}
	
	QStyledItemDelegate::paint(p, opt, idx); // always lay the "base coat"
	
	p->save();
	QRect adj = opt.rect.adjusted(1, 1, 0, 0);
	if (enabled_count == item->rowCount()) {
		p->fillRect(adj, QBrush(color_active_group));
	} else if (enabled_count > 0) {
		p->fillRect(adj, QBrush(color_partial_group));
	} else {
		p->fillRect(adj, QBrush(color_inactive_group));
	}
	p->restore();

	p->save(); // border last
	p->setBrush(Qt::NoBrush);
	if (dirty_count) {
		p->setPen(QPen(color_dirty_border, 1));
		p->drawRect(opt.rect.adjusted(0, 0, -1, -1));
	} else {
		p->setPen(QPen(color_border));
		p->drawRect(opt.rect);
	}
	p->restore();
}

/* If grid-sizing ever comes back...
QSize UberDelegate::sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &idx) const {
	if (idx.column() == 0)
		return QStyledItemDelegate::sizeHint(opt, idx);
	return QSize(24, 24);
}
*/