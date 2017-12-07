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
#include "dwarfmodelproxy.h"
#include "dwarftherapist.h"
#include "defines.h"
#include "defaultfonts.h"
#include "utils.h"
#include <QAction>
#include <QMenu>
#include <QMouseEvent>
#include <QSettings>
#include <QPainter>

RotatedHeader::RotatedHeader(Qt::Orientation orientation, QWidget *parent)
    : QHeaderView(orientation, parent)
    , m_hovered_column(-1)
    , m_last_sorted_idx(0)
    , m_preferred_height(150)
{
    setMinimumSectionSize(0);
    setSectionsClickable(true);
    setSortIndicatorShown(true);
    setMouseTracking(true);
    read_settings();
    connect(DT, SIGNAL(settings_changed()), this, SLOT(read_settings()));
}

void RotatedHeader::column_hover(int col) {
    updateSection(m_hovered_column);
    m_hovered_column = col;
    updateSection(col);
}

void RotatedHeader::read_settings() {
    QSettings *s = DT->user_settings();
    int pad = s->value("options/grid/cell_padding", 0).toInt();
    int cell_size = s->value("options/grid/cell_size", DEFAULT_CELL_SIZE).toInt();
    cell_size += (2 + 2*pad);
    for(int i=1; i < count(); ++i) {
        if (!m_spacer_indexes.contains(i)) {
            resizeSection(i, cell_size);
        }
    }
    m_shade_column_headers = s->value("options/grid/shade_column_headers", true).toBool();
    m_header_text_bottom = s->value("options/grid/header_text_bottom", false).toBool();
    m_font = s->value("options/grid/header_font", QFont(DefaultFonts::getRowFontName(), DefaultFonts::getRowFontSize())).value<QFont>();
}


void RotatedHeader::paintSection(QPainter *p, const QRect &rect, int idx) const {
    QColor bg = model()->headerData(idx, Qt::Horizontal,Qt::BackgroundColorRole).value<QColor>();

    QBrush grad_brush = QBrush(bg);
    if (m_shade_column_headers) {
        QLinearGradient g(rect.topLeft(), rect.bottomLeft());
        g.setColorAt(0.05, QColor(255, 255, 255, 10));
        g.setColorAt(0.65, bg);
        grad_brush = QBrush(g);
    }

    if (m_spacer_indexes.contains(idx)) {
        p->save();
        p->fillRect(rect.adjusted(0,8,0,0), grad_brush);
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
    if (m_last_sorted_idx == idx) {
        if (sortIndicatorOrder() == Qt::AscendingOrder) {
            opt.sortIndicator = QStyleOptionHeader::SortDown;
        } else {
            opt.sortIndicator = QStyleOptionHeader::SortUp;
        }
    }
    if (m_hovered_column == idx) {
        state |= QStyle::State_MouseOver;
    }

    opt.state = state;
    style()->drawControl(QStyle::CE_HeaderSection, &opt, p);

    if (idx > 0)
        p->fillRect(rect.adjusted(1,8,-1,-2), grad_brush);

    if (m_last_sorted_idx == idx) {
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
    p->setPen(complement(bg,0.25)); //Qt::black);
    p->setRenderHint(QPainter::TextAntialiasing);
    p->setFont(m_font);
    QFontMetrics fm = p->fontMetrics();

    if (m_header_text_bottom)
    {
        //flip column header text to read from bottom to top
        p->translate(rect.x() + rect.width(), rect.height());
        p->rotate(-90);
        p->drawText(4,-rect.width() + ((rect.width()-fm.height()) / 2),rect.height()-10,rect.width(),1,data);
    }
    else
    {
        p->translate(rect.x(), rect.y());
        p->rotate(90);
        p->drawText(9, -((rect.width()-fm.height()) / 2) - (fm.height()/4), data); //wtf.. i have no idea but it's centered so i'll take it
    }
    p->restore();
}

void RotatedHeader::resizeSection(int logicalIndex, int size) {
    QHeaderView::resizeSection(logicalIndex, size);
}

void RotatedHeader::set_header_height(QString max_title){
    QFontMetrics fm(m_font);
    m_preferred_height = fm.width(max_title)+15;
    if(m_preferred_height <= 0)
        m_preferred_height = 150;
}

void RotatedHeader::set_index_as_spacer(int idx) {
    m_spacer_indexes << idx;
}

void RotatedHeader::clear_spacers() {
    m_spacer_indexes.clear();
}

QSize RotatedHeader::sizeHint() const {
    return QSize(32, m_preferred_height);
}

void RotatedHeader::mouseMoveEvent(QMouseEvent *e) {
    m_p = e->pos();
    QHeaderView::mouseMoveEvent(e);
}

void RotatedHeader::mousePressEvent(QMouseEvent *e) {
    m_p = e->pos();
    int idx = logicalIndexAt(e->pos());
    if (idx > 0 && idx < count() && e->button() == Qt::RightButton) {
        emit section_right_clicked(idx);
    }
    QHeaderView::mousePressEvent(e);
}

void RotatedHeader::leaveEvent(QEvent *e) {
    m_p = QPoint(-1, -1);
    QHeaderView::leaveEvent(e);
}

void RotatedHeader::contextMenuEvent(QContextMenuEvent *evt) {
    int idx = logicalIndexAt(evt->pos());
    QMenu *m = new QMenu(this);

    if (idx == 0) { //name header
        m->addAction("Sort by..");
        m->addSeparator();
        m->addAction(QIcon(":img/sort-number.png"), tr("Age Ascending"), this, SLOT(sort_action()))->setData(DwarfModelProxy::DSR_AGE_ASC);
        m->addAction(QIcon(":img/sort-number-descending.png"),tr("Age Descending"), this, SLOT(sort_action()))->setData(DwarfModelProxy::DSR_AGE_DESC);
        m->addSeparator();
        m->addAction(QIcon(":img/sort-number.png"), tr("Body Size Ascending"), this, SLOT(sort_action()))->setData(DwarfModelProxy::DSR_SIZE_ASC);
        m->addAction(QIcon(":img/sort-number-descending.png"),tr("Body Size Descending"), this, SLOT(sort_action()))->setData(DwarfModelProxy::DSR_SIZE_DESC);
        m->addSeparator();
        m->addAction(QIcon(":img/sort-number.png"),tr("ID Ascending"), this, SLOT(sort_action()))->setData(DwarfModelProxy::DSR_ID_ASC);
        m->addAction(QIcon(":img/sort-number-descending.png"),tr("ID Descending"), this, SLOT(sort_action()))->setData(DwarfModelProxy::DSR_ID_DESC);
        m->addSeparator();
        m->addAction(QIcon(":img/sort-alphabet.png"),tr("Name Ascending"), this, SLOT(sort_action()))->setData(DwarfModelProxy::DSR_NAME_ASC);
        m->addAction(QIcon(":img/sort-alphabet-descending.png"),tr("Name Descending"), this, SLOT(sort_action()))->setData(DwarfModelProxy::DSR_NAME_DESC);
    }
    m->exec(viewport()->mapToGlobal(evt->pos()));
}

void RotatedHeader::sort_action() {
    QAction *sender = qobject_cast<QAction*>(QObject::sender());
    Qt::SortOrder order;
    DwarfModelProxy::DWARF_SORT_ROLE role = static_cast<DwarfModelProxy::DWARF_SORT_ROLE>(sender->data().toInt());
    if(sender->text().toLower().contains(tr("desc")))
        order = Qt::DescendingOrder;
    else
        order = Qt::AscendingOrder;
    emit sort(0, role, order);
}

void RotatedHeader::toggle_set_action() {
    QAction *sender = qobject_cast<QAction*>(QObject::sender());
    QString set_name = sender->data().toString();
    for(int i = 1; i < count(); ++i) {
        QString col_set_name = model()->headerData(i, Qt::Horizontal,
                                                   Qt::UserRole).toString();
        if (col_set_name == set_name) {
            hideSection(i);
        }
    }
}
