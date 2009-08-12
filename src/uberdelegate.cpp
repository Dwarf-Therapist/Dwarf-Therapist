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
#include "dwarfmodelproxy.h"
#include "dwarf.h"
#include "defines.h"
#include "gridview.h"
#include "columntypes.h"
#include "utils.h"
#include "dwarftherapist.h"

UberDelegate::UberDelegate(QObject *parent)
	: QStyledItemDelegate(parent)
{
	read_settings();
	connect(DT, SIGNAL(settings_changed()), this, SLOT(read_settings()));
}

void UberDelegate::read_settings() {
	QSettings *s = DT->user_settings();
	s->beginGroup("options");
	s->beginGroup("colors");
	color_skill = s->value("skill").value<QColor>();
	color_dirty_border = s->value("dirty_border").value<QColor>();
	color_active_labor = s->value("active_labor").value<QColor>();
	color_active_group = s->value("active_group").value<QColor>();
	color_inactive_group= s->value("inactive_group").value<QColor>();
	color_partial_group = s->value("partial_group").value<QColor>();
	color_guides = s->value("guides").value<QColor>();
	color_border = s->value("border").value<QColor>();
	s->endGroup();
	s->endGroup();
	cell_size = s->value("options/grid/cell_size", DEFAULT_CELL_SIZE).toInt();
	cell_padding = s->value("options/grid/cell_padding", 0).toInt();
	auto_contrast = s->value("options/auto_contrast", true).toBool();
	draw_aggregates = s->value("options/show_aggregates", true).toBool();
}

void UberDelegate::paint(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &proxy_idx) const {
	if (!proxy_idx.isValid()) {
		return;
	}
	if (proxy_idx.column() == 0) { // we never do anything with the 0 col...
		QStyledItemDelegate::paint(p, opt, proxy_idx);
		return;
	}

	paint_cell(p, opt, proxy_idx);

	if (proxy_idx.column() == m_model->selected_col()) {
		p->save();
		p->setPen(QPen(color_guides));
		p->drawLine(opt.rect.topLeft(), opt.rect.bottomLeft());
		p->drawLine(opt.rect.topRight(), opt.rect.bottomRight());
		p->restore();
	}
}

void UberDelegate::paint_cell(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &idx) const {
	QModelIndex model_idx = m_proxy->mapToSource(idx);
	COLUMN_TYPE type = static_cast<COLUMN_TYPE>(model_idx.data(DwarfModel::DR_COL_TYPE).toInt());
	QRect adjusted = opt.rect.adjusted(cell_padding, cell_padding, (cell_padding * -2) - 1, (cell_padding * -2) - 1);
	switch (type) {
		case CT_SKILL:
			{
				short rating = model_idx.data(DwarfModel::DR_RATING).toInt();
				QColor bg = paint_bg(adjusted, false, p, opt, idx);
				paint_skill(rating, bg, p, opt, idx);
				paint_grid(adjusted, false, p, opt, idx);
			}
			break;
		case CT_LABOR:
			{
				bool agg = model_idx.data(DwarfModel::DR_IS_AGGREGATE).toBool();
				if (m_model->current_grouping() == DwarfModel::GB_NOTHING || !agg) {
					paint_labor(adjusted, p, opt, idx);
				} else {
					if (draw_aggregates)
						paint_aggregate(adjusted, p, opt, idx);
				}
			}
			break;
		case CT_HAPPINESS:
			paint_bg(adjusted, false, p, opt, idx);
			QStyledItemDelegate::paint(p, opt, idx);
			paint_grid(adjusted, false, p, opt, idx);
			break;
		case CT_DEFAULT:
		case CT_SPACER:
		default:
			paint_bg(adjusted, false, p, opt, idx);
			//QStyledItemDelegate::paint(p, opt, idx);
			if (opt.state & QStyle::State_Selected) {
				p->save();
				p->setPen(color_guides);
				p->drawLine(opt.rect.topLeft(), opt.rect.topRight());
				p->drawLine(opt.rect.bottomLeft(), opt.rect.bottomRight());
				p->restore();
			}
			break;
	}
}

QColor UberDelegate::paint_bg(const QRect &adjusted, bool active, QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &proxy_idx) const {
	QModelIndex idx = m_proxy->mapToSource(proxy_idx);
	QColor bg = idx.data(DwarfModel::DR_DEFAULT_BG_COLOR).value<QColor>();
	p->save();
	p->fillRect(opt.rect, QBrush(bg));
	if (active) {
		bg = color_active_labor;
		p->fillRect(adjusted, QBrush(bg));
	}
	p->restore();
	return bg;
}

void UberDelegate::paint_skill(int rating, QColor bg, QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &) const {
	QColor c = color_skill; 
	if (auto_contrast)
		c = compliment(bg);
	QRect adjusted = opt.rect.adjusted(cell_padding, cell_padding, (cell_padding * -2) - 1, (cell_padding * -2) - 1);
	p->save();
	bool growing_box = false;
	if (growing_box) {
		if (rating == 15) {
			// draw diamond
			p->setRenderHint(QPainter::Antialiasing);
			p->setPen(Qt::gray);
			p->setBrush(QBrush(c));
			QPolygonF shape;
			shape << QPointF(0.5, 0.1) //top
				<< QPointF(0.75, 0.5) // right
				<< QPointF(0.5, 0.9) //bottom
				<< QPointF(0.25, 0.5); // left

			p->translate(opt.rect.x() + 2, opt.rect.y() + 2);
			p->scale(opt.rect.width()-4, opt.rect.height()-4);
			p->drawPolygon(shape);
		} else if (rating < 15 && rating > 10) {
			float size = 0.75f * (rating / 14.0f);
			float inset = (1.0f - size) / 2.0f;
			p->translate(opt.rect.x(), opt.rect.y());
			p->scale(opt.rect.width(), opt.rect.height());
			p->fillRect(QRectF(inset, inset, size, size), QBrush(c));
		} else if (rating > 0) {
			float size = 0.65f * (rating / 10.0f);
			float inset = (1.0f - size) / 2.0f;
			p->translate(opt.rect.x(), opt.rect.y());
			p->scale(opt.rect.width(), opt.rect.height());
			p->fillRect(QRectF(inset, inset, size, size), QBrush(c));
		}
	} else {
		p->setBrush(QBrush(c));
		p->setPen(c);
		p->translate(adjusted.x(), adjusted.y());
		p->scale(adjusted.width(), adjusted.height());
		QVector<QLineF> lines;
		switch (rating) {
			case 15:
				{
					//http://doc.trolltech.com/4.5/itemviews-stardelegate-starrating-cpp.html
					QPolygonF poly;
					double pi = 3.14; // nobody cares, these are tiny
					//poly << QPointF(0.5, 0.9);
					for (int i = 0; i < 5; ++i)
						poly << QPointF(
									0.5 + 0.4 * sin(i * ((pi * 2)/5.0)), // x
									0.5 + 0.4 * cos(i * ((pi * 2)/5.0))  // y
								);
					p->drawPolygon(poly, Qt::WindingFill);
				}
				break;
			case 14:
				p->drawEllipse(QPointF(0.5, 0.5), 0.35, 0.35);
				break;
			case 13:
				{
					QPolygonF poly;
					poly << QPointF(0.5, 0.1)
						<< QPointF(0.5, 0.5)
						<< QPointF(0.9, 0.5);
					p->drawPolygon(poly);
				}
			case 12:
				{
					QPolygonF poly;
					poly << QPointF(0.1, 0.5)
						<< QPointF(0.5, 0.5)
						<< QPointF(0.5, 0.9);
					p->drawPolygon(poly);
				}
			case 11:
				{
					QPolygonF poly;
					poly << QPointF(0.9, 0.5)
						 << QPointF(0.5, 0.5)
						 << QPointF(0.5, 0.9);
					p->drawPolygon(poly);
				}
			case 10:
				{
					QPolygonF poly;
					poly << QPointF(0.1, 0.5)
						 << QPointF(0.5, 0.5)
						 << QPointF(0.5, 0.1);
					p->drawPolygon(poly);
				}
			case 9:
				lines << QLineF(QPointF(0.1, 0.5), QPointF(0.5, 0.1));
			case 8:
				lines << QLineF(QPointF(0.5, 0.9), QPointF(0.1, 0.5));
			case 7:
				lines << QLineF(QPointF(0.5, 0.9), QPointF(0.9, 0.5));
			case 6:
				lines << QLineF(QPointF(0.5, 0.1), QPointF(0.9, 0.5));
			case 5:
				lines << QLineF(QPointF(0.5, 0.9), QPointF(0.5, 0.5));
			case 4:
				lines << QLineF(QPointF(0.5, 0.1), QPointF(0.5, 0.5));
			case 3:
				lines << QLineF(QPointF(0.1, 0.5), QPointF(0.5, 0.5));
			case 2:
				lines << QLineF(QPointF(0.5, 0.5), QPointF(0.9, 0.5));
			case 1:
				lines << QLineF(QPointF(0.5, 0.5), QPointF(0.5, 0.5));
				break;
		}
		p->drawLines(lines);
	}
	p->restore();
}

void UberDelegate::paint_labor(const QRect &adjusted, QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &proxy_idx) const {
	QModelIndex idx = m_proxy->mapToSource(proxy_idx);
	short rating = idx.data(DwarfModel::DR_RATING).toInt();
	
	Dwarf *d = m_model->get_dwarf_by_id(idx.data(DwarfModel::DR_ID).toInt());
	if (!d) {
		return QStyledItemDelegate::paint(p, opt, idx);
	}

	int labor_id = idx.data(DwarfModel::DR_LABOR_ID).toInt();
	bool enabled = d->is_labor_enabled(labor_id);
	bool dirty = d->is_labor_state_dirty(labor_id);

	QColor bg = paint_bg(adjusted, enabled, p, opt, proxy_idx);
	paint_skill(rating, bg, p, opt, proxy_idx);
	paint_grid(adjusted, dirty, p, opt, proxy_idx);
}

void UberDelegate::paint_aggregate(const QRect &adjusted, QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &proxy_idx) const {
	if (!proxy_idx.isValid()) {
		return;
	}
	QModelIndex model_idx = m_proxy->mapToSource(proxy_idx);
	QModelIndex first_col = m_proxy->index(proxy_idx.row(), 0, proxy_idx.parent());
	if (!first_col.isValid()) {
		return;
	}
	
	QString group_name = proxy_idx.data(DwarfModel::DR_GROUP_NAME).toString();
	int labor_id = proxy_idx.data(DwarfModel::DR_LABOR_ID).toInt();

	int dirty_count = 0;
	int enabled_count = 0;
	for (int i = 0; i < m_proxy->rowCount(first_col); ++i) {
		int dwarf_id = m_proxy->data(m_proxy->index(i, 0, first_col), DwarfModel::DR_ID).toInt();
		Dwarf *d = m_model->get_dwarf_by_id(dwarf_id);
		if (!d)
			continue;
		if (d->is_labor_enabled(labor_id))
			enabled_count++;
		if (d->is_labor_state_dirty(labor_id))
			dirty_count++;
	}

	QStyledItemDelegate::paint(p, opt, proxy_idx); // slap on the main bg
	
	p->save();
	if (enabled_count == m_proxy->rowCount(first_col)) {
		p->fillRect(adjusted, QBrush(color_active_group));
	} else if (enabled_count > 0) {
		p->fillRect(adjusted, QBrush(color_partial_group));
	} else {
		p->fillRect(adjusted, QBrush(color_inactive_group));
	}
	p->restore();

	paint_grid(adjusted, dirty_count > 0, p, opt, proxy_idx);
}

void UberDelegate::paint_grid(const QRect &adjusted, bool dirty, QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &) const {
	p->save(); // border last
	p->setBrush(Qt::NoBrush);
	if (dirty) {
		p->setPen(QPen(color_dirty_border, 1));
		p->drawRect(adjusted);
	} else if (opt.state.testFlag(QStyle::State_Selected)) {
		p->setPen(color_guides);
		p->drawLine(opt.rect.topLeft(), opt.rect.topRight());
		p->drawLine(opt.rect.bottomLeft(), opt.rect.bottomRight());
	} else {
		p->setPen(color_border);
		p->drawRect(adjusted);
	}
	p->restore();
}

QSize UberDelegate::sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &idx) const {
	if (idx.column() == 0)
		return QStyledItemDelegate::sizeHint(opt, idx);
	return QSize(cell_size, cell_size);
}
