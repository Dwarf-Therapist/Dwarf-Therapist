#include <QtGui>
#include "dwarfmodel.h"
#include "uberdelegate.h"

UberDelegate::UberDelegate(QObject *parent)
	: QStyledItemDelegate(parent)
{
}

void UberDelegate::paint(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &idx) const {
	if (idx.column() == 0) { // we never do anything with the 0 col...
		QStyledItemDelegate::paint(p, opt, idx); // always lay the "base coat"
		return;
	}
	
	const DwarfModel *model = dynamic_cast<const DwarfModel*>(idx.model());
	if (model->current_grouping() == DwarfModel::GB_NOTHING) {
		paint_skill(p, opt, idx);
	} else {
		QModelIndex first_col = model->index(idx.row(), 0, idx.parent());
		if (model->hasChildren(first_col)) { // skill item (under a group header)
			//QStyledItemDelegate::paint(p, opt, idx); // always lay the "base coat"
			paint_aggregate(p, opt, idx);
		} else {
			paint_skill(p, opt, idx);
			
		}
	}
}

void UberDelegate::paint_skill(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &idx) const {
	//const DwarfModel *m = dynamic_cast<const DwarfModel*>(idx.model());
	//QStandardItem *item = m->itemFromIndex(idx);
	
	bool enabled = idx.data(DwarfModel::DR_ENABLED).toBool();
	short rating = idx.data(DwarfModel::DR_RATING).toInt();
	bool dirty = idx.data(DwarfModel::DR_DIRTY).toBool();
	bool skip_border = false;

	if (enabled) {
		p->save();
		p->fillRect(opt.rect, QBrush(m_active_bg_color));
		p->restore();
	}
	// draw rating
	if (rating == 15) {
		// draw diamond
		p->save();
		p->setRenderHint(QPainter::Antialiasing);
		p->setPen(Qt::gray);
		p->setBrush(QBrush(Qt::black));

		QPolygonF shape;
		shape << QPointF(0.5, 0.1) //top
			<< QPointF(0.75, 0.5) // right
			<< QPointF(0.5, 0.9) //bottom
			<< QPointF(0.25, 0.5); // left

		p->translate(opt.rect.x() + 2, opt.rect.y() + 2);
		p->scale(opt.rect.width()-4, opt.rect.height()-4);
		p->drawPolygon(shape);
		p->restore();

		p->save();
		p->setPen(QColor(0x111111));
		p->drawRect(opt.rect.adjusted(0,0,-1,-1));
		p->restore();
		skip_border = true;

	} else if (rating < 15 && rating > 9) {
		// TODO: try drawing the square of increasing size...
		int offset = 14 - rating;
		int color = 0xFFFFFF - 0x111100 * offset;
		p->save();
		p->translate(opt.rect.x(), opt.rect.y());
		p->scale(opt.rect.width(), opt.rect.height());
		p->fillRect(QRectF(0.15, 0.15, 0.7, 0.7), QBrush(QColor(color)));
		p->restore();
	} else if (rating > 0) {
		int color = 0xAAAAAA - 0x090909 * rating;
		p->save();
		p->translate(opt.rect.x(), opt.rect.y());
		p->scale(opt.rect.width(), opt.rect.height());
		p->fillRect(QRectF(0.25, 0.25, 0.5, 0.5), QBrush(QColor(color)));
		p->restore();
	}

	if (dirty || !skip_border) {
		p->save(); // border last
		p->setBrush(Qt::NoBrush);
		if (dirty) {
			p->setPen(QPen(QColor(0xFF6600), 1));
			p->drawRect(opt.rect.adjusted(0, 0, -1, -1));
		} else {
			p->setPen(QColor(0xd9d9d9));
			p->drawRect(opt.rect);
		}
		p->restore();
	}
}

void UberDelegate::paint_aggregate(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &idx) const {
	const DwarfModel *m = dynamic_cast<const DwarfModel*>(idx.model());
	QModelIndex first_col = m->index(idx.row(), 0, idx.parent());
	QStandardItem *item = m->itemFromIndex(first_col);

	int enabled = idx.data(DwarfModel::DR_ENABLED).toInt();
	bool dirty = idx.data(DwarfModel::DR_DIRTY).toBool();
	int children = item->rowCount();

	Q_ASSERT(children > 0);
	QStyledItemDelegate::paint(p, opt, idx); // always lay the "base coat"
	p->save();
	QRect adj = opt.rect.adjusted(1, 1, -1, 1);
	if (enabled >= children) {
		p->fillRect(adj, m_active_bg_color);
	} else if (enabled > 0) {
		p->fillRect(adj, QBrush(0xAAAAAA));
	} else {
		p->fillRect(adj, QBrush(0xCCCCCC));
	}
	p->restore();

	

	p->save(); // border last
	p->setBrush(Qt::NoBrush);
	if (dirty) {
		p->setPen(QPen(QColor(0xFF6600), 1));
		p->drawRect(opt.rect.adjusted(0, 0, -1, -1));
	} else {
		p->setPen(QColor(0xd9d9d9));
		p->drawRect(opt.rect);
	}
	p->restore();
}

QSize UberDelegate::sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &idx) const {
	/*if (idx.row() == 0 && !idx.parent().isValid()) { //top row, unparented (MAIN HEADER)
		return QSize(16, 100);
	}*/
	return QStyledItemDelegate::sizeHint(opt, idx);
}