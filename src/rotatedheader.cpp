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
#include "gamedatareader.h"

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

	GameDataReader *gdr = GameDataReader::ptr();

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
	if (idx > 0)
		p->fillRect(rect.adjusted(2,2,-2,-2), QBrush(gdr->get_color(QString("labors/%1/color").arg(idx-1))));

	
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

void RotatedHeader::set_model(DwarfModel *dm) {
	m_model = dm;
	setResizeMode(QHeaderView::Fixed);
	setResizeMode(0, QHeaderView::ResizeToContents);
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
	if (idx < count()) {
		m_model->section_clicked(idx, e->button());
	}
	QHeaderView::mousePressEvent(e);
}

void RotatedHeader::leaveEvent(QEvent *e) {
	m_p = QPoint(-1, -1);
	QHeaderView::leaveEvent(e);
}