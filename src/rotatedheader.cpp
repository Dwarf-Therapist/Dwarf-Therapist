#include "rotatedheader.h"
#include "dwarfmodel.h"

RotatedHeader::RotatedHeader(Qt::Orientation orientation, QWidget *parent)
	: QHeaderView(orientation, parent)
{
}

void RotatedHeader::paintSection(QPainter *p, const QRect &rect, int idx) const {
	if (!rect.isValid()) return;

	QStyleOptionHeader opt;
	opt.rect = rect;
	opt.orientation = Qt::Horizontal;
	opt.section = idx;
	opt.sortIndicator = QStyleOptionHeader::None;
	

	QStyle::State state = QStyle::State_None;
	if (isEnabled())
		state |= QStyle::State_Enabled;
	if (window()->isActiveWindow())
		state |= QStyle::State_Active;
	if (rect.contains(m_p))
		state |= QStyle::State_MouseOver;
	if (sortIndicatorSection() == idx)
		state |= QStyle::State_Sunken;
	opt.state = state;
	style()->drawControl(QStyle::CE_HeaderSection, &opt, p, this);

	QString data = m_model->headerData(idx, Qt::Horizontal).toString();
	p->save();
	p->setPen(Qt::black);
	p->setRenderHint(QPainter::Antialiasing);
	p->translate(rect.x(), rect.y());
	p->rotate(90);
	p->setFont(QFont("Verdana", 8));
	p->drawText(8, -4, data);
	p->restore();
}

QSize RotatedHeader::sizeHint() const {
	return QSize(32, 120);
}

void RotatedHeader::mouseMoveEvent(QMouseEvent *e) {
	m_p = e->pos();
	QHeaderView::mouseMoveEvent(e);
}

void RotatedHeader::leaveEvent(QEvent *e) {
	m_p = QPoint(-1, -1);
	QHeaderView::leaveEvent(e);
}