#include "statetableview.h"
#include "dwarfmodel.h"
#include <QLineEdit>
#include <QVariant>
#include <QColor>
#include "qmath.h"

HeaderDelegate::HeaderDelegate(QObject *parent /* = 0 */)
: QStyledItemDelegate(parent)
{}

void HeaderDelegate::paint(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &idx) const {
	p->save();
	p->translate(opt.rect.left(), opt.rect.top());
	p->rotate(90);
	//p->setBrush(QBrush(QColor(200, 200, 200)));
	p->setFont(QFont("Arial", 7));
	p->drawText(4, -4, idx.data().toString());
	p->restore();
}

SkillDelegate::SkillDelegate(QObject *parent /* = 0 */)
	: QStyledItemDelegate(parent)
{}


void SkillDelegate::paint(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &idx) const {
	if (idx.column() == 0) {
		QStyledItemDelegate::paint(p, opt, idx);
		return;
	}
	bool enabled = idx.data(DwarfModel::DR_ENABLED).toBool();
	short rating = idx.data(DwarfModel::DR_RATING).toInt();

	
	if (opt.state & QStyle::State_Selected) {
		p->save();
		p->setPen(QColor(0x66EEFF));
		//p->translate(opt.rect.topLeft());
		p->drawLine(opt.rect.topLeft(), opt.rect.topRight());
		p->drawLine(opt.rect.bottomLeft(), opt.rect.bottomRight());
		p->restore();
	}
	if (enabled) {
		p->save();
		p->fillRect(opt.rect, QBrush(QColor(0x99FF55)));
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
		
	} else if (rating < 15 && rating > 9) {
		int offset = 14 - rating;
		int color = 0xFFFFFF - 0x001100 * offset;
		int sq_size = opt.rect.width() / 1.2;
		p->save();
		p->fillRect(opt.rect.adjusted(sq_size, sq_size, -sq_size, -sq_size), QBrush(QColor(color)));
		p->restore();
	} else if (rating > 0) {
		int color = 0xFFFFFF - 0x111111 * rating;
		int sq_size = opt.rect.width() / 3;
		p->save();
		p->fillRect(opt.rect.adjusted(sq_size, sq_size, -sq_size, -sq_size), QBrush(QColor(color)));
		p->restore();
	}
}

StateTableView::StateTableView(QWidget *parent)
	: QTableView(parent)
{
	ui.setupUi(this);
	setItemDelegate(new SkillDelegate);
	setItemDelegateForRow(0, new HeaderDelegate);
}

StateTableView::~StateTableView()
{
}

void StateTableView::setModel(QAbstractItemModel *model) {
	QTableView::setModel(model);
	resizeColumnToContents(0);
	setRowHeight(0, 100);
}

void StateTableView::set_grid_size(int new_size) {
	if (model()->rowCount() < 1) {
		return;
	}
	for (int i=1; i < model()->rowCount(); ++i) {
		setRowHeight(i, new_size);
	}
	for (int i=1; i < model()->columnCount(); ++i) {
		setColumnWidth(i, new_size);
	}
}

void StateTableView::filter_dwarves(QString text) {
	//model()-

}