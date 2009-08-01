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
#include "rotatedheader.h"
#include "dwarfmodel.h"
#include "dwarfmodelproxy.h"

RotatedHeader::RotatedHeader(Qt::Orientation orientation, QWidget *parent)
	: QHeaderView(orientation, parent)
{
	setClickable(true);
	setSortIndicatorShown(true);
	setMouseTracking(true);
}

void RotatedHeader::paintSection(QPainter *p, const QRect &rect, int idx) const {
	if (!rect.isValid() || idx == 0) 
		return QHeaderView::paintSection(p, rect, idx);

	QColor bg = model()->headerData(idx, Qt::Horizontal, Qt::BackgroundColorRole).value<QColor>();
	if (m_spacer_indexes.contains(idx)) {
		p->save();
		p->fillRect(rect, QBrush(bg));
		p->restore();
		return;
	}

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
	if (sortIndicatorSection() == idx) {
		//state |= QStyle::State_Sunken;
		if (sortIndicatorOrder() == Qt::AscendingOrder) {
			opt.sortIndicator = QStyleOptionHeader::SortDown;
		} else {
			opt.sortIndicator = QStyleOptionHeader::SortUp;
		}
	}

	opt.state = state;
	style()->drawControl(QStyle::CE_HeaderSection, &opt, p);
	
	QLinearGradient g(rect.topLeft(), rect.bottomLeft());
    g.setColorAt(0.25, QColor(255, 255, 255, 10));
	g.setColorAt(1.0, bg);
	if (idx > 0)
		p->fillRect(rect.adjusted(1,8,-1,-2), QBrush(g));

	if (sortIndicatorSection() == idx) {
		opt.rect = QRect(opt.rect.x() + opt.rect.width()/2 - 5, opt.rect.y(), 10, 8);
		style()->drawPrimitive(QStyle::PE_IndicatorHeaderArrow, &opt, p);
	}

	/* Draw a border around header if column has guides applied
	DwarfModelProxy *prox = static_cast<DwarfModelProxy*>(model());
	DwarfModel *dm = prox->get_dwarf_model();
	int col = dm->selected_col();
	if (dm->selected_col() == idx) {
		p->save();
		p->setPen(Qt::red);
		p->setBrush(Qt::NoBrush);
		p->drawRect(rect);
		p->restore();
	}
	*/
	
	QString data = this->model()->headerData(idx, Qt::Horizontal).toString();
	p->save();
	p->setPen(Qt::black);
	p->setRenderHint(QPainter::Antialiasing);
	p->translate(rect.x(), rect.y());
	p->rotate(90);
	p->setFont(QFont("Verdana", 8));
	p->drawText(14, -4, data);
	p->restore();
}

void RotatedHeader::resizeSection(int logicalIndex, int size) {
	QHeaderView::resizeSection(logicalIndex, size);
}

void RotatedHeader::set_index_as_spacer(int idx) {
	m_spacer_indexes << idx;
}

void RotatedHeader::clear_spacers() {
	m_spacer_indexes.clear();
}

QSize RotatedHeader::sizeHint() const {
	return QSize(32, 120);
}

void RotatedHeader::mouseMoveEvent(QMouseEvent *e) {
	m_p = e->pos();
	QHeaderView::mouseMoveEvent(e);
}

void RotatedHeader::mousePressEvent(QMouseEvent *e) {
	m_p = e->pos();
	int idx = logicalIndexAt(e->pos());
	if (idx < count() && e->button() == Qt::RightButton) {
		emit section_right_clicked(idx);
	}
	QHeaderView::mousePressEvent(e);
}

void RotatedHeader::leaveEvent(QEvent *e) {
	m_p = QPoint(-1, -1);
	QHeaderView::leaveEvent(e);
}