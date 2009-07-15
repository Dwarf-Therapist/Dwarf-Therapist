#include "rotatedheader.h"
#include "dwarfmodel.h"

RotatedHeader::RotatedHeader(Qt::Orientation orientation, QWidget *parent)
	: QHeaderView(orientation, parent)
{}

void RotatedHeader::paintSection(QPainter *p, const QRect &rect, int idx) const {
	QString data = m_model->headerData(idx, Qt::Horizontal).toString();
	m_model->setHeaderData(idx, Qt::Horizontal, "");
	QHeaderView::paintSection(p, rect, idx);

	m_model->setHeaderData(idx, Qt::Horizontal, data);


	//p->save();
	p->setPen(Qt::black);
	p->setRenderHint(QPainter::Antialiasing);
	p->translate(rect.x(), rect.y());
	p->rotate(90);
	p->setFont(QFont("Verdana", 8, QFont::Bold));
	p->drawText(4, -4, data);
	//p->restore();

	//p->save();
	//p->setPen(QColor(0xd9d9d9));
	p->setPen(Qt::red);
	p->setBrush(Qt::NoBrush);
	p->drawRect(rect);
	//p->restore();
}

QSize RotatedHeader::sizeHint() const {
	return QSize(16, 120);
}