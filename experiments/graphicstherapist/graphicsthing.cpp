#include "graphicsthing.h"
#include "displaycell.h"
#include <QtGui>

GraphicsThing::GraphicsThing(const QString &name, QGraphicsItem *parent)
    : BaseGraphicsObject(parent)
    , m_name(name)
    , m_expanded(false)
    , m_hovering(false)
    , m_text(new QGraphicsTextItem(name, this))
    , m_min_width(0) // boundingRect() relies on this being init'd to 0
    , m_bg_brush(QBrush(QColor(0, 120, 0, 255)))
    , m_bg_hover_brush(QBrush(QColor(64, 64, 64, 128)))
    , m_animation(new QParallelAnimationGroup(this))
{
    setFlag(ItemIsSelectable, true);
    setAcceptsHoverEvents(true);
    m_text->setFont(QFont("Helvetica", 7));
    m_text->setDefaultTextColor(Qt::white);
    m_text->setPos(1, 1);
    //this->setGraphicsEffect(new QGraphicsDropShadowEffect(this));

    connect(this, SIGNAL(hover_start()), SLOT(on_hover_start()));
    connect(this, SIGNAL(hover_stop()), SLOT(on_hover_stop()));
    connect(this, SIGNAL(single_click()), SLOT(toggle_expand()));

    m_items.clear();
    /*
    for (int i = 0; i < 25; ++i) {
        DisplayCell *cell = new DisplayCell(this);
        cell->hide();
        m_items << cell;
    }
    */
}

void GraphicsThing::on_added_to_scene(QGraphicsScene *scene) {
    qDebug() << "added to scene:" << scene;
    update();
}
/*
QRectF GraphicsThing::boundingRect() const {
    int w = m_min_width;
    if (w == 0) {
        w = m_text->boundingRect().width();
    }
    QRectF bounds(0, 0, w, m_text->boundingRect().height() + 2);
    foreach(QGraphicsItem *child, childItems()) {
        QPointF child_pos = child->pos();
        QRectF child_rect = child->boundingRect();
        if (child_pos.x() + child_rect.width() > bounds.width()) {
            bounds.setWidth(child_pos.x() + child_rect.width() + 2);
        }
        if ((child_pos.y() + child_rect.height()) > bounds.height()) {
            bounds.setHeight(child_pos.y() + child_rect.height() + 2);
        }
    }
    return bounds;
}

void GraphicsThing::paint(QPainter *p, const QStyleOptionGraphicsItem *opt,
                          QWidget *w) {
    Q_UNUSED(opt);
    Q_UNUSED(w);
    p->save();
    p->setPen(QPen(QColor(192, 192, 192, 255), 1));
    p->drawEllipse(10, 10, 20, 50);
    //p->setBrush(Qt::NoBrush);
    p->setBrush(m_hovering ? m_bg_hover_brush : m_bg_brush);
    QRectF r = boundingRect();
    p->drawRect(1, 1, r.width() - 1, r.height() -1);
    p->drawLine(r.topLeft(), r.topRight());
    p->drawLine(r.bottomLeft(), r.bottomRight());
    p->restore();
}
*/
void GraphicsThing::toggle_expand() {
    if (m_expanded) {
        collapse();
    } else {
        expand_right();
    }
}

void GraphicsThing::collapse() {
    m_expanded = false;
    int x = 0;
    m_animation->stop();
    m_animation->clear();
    prepareGeometryChange();
    foreach(DisplayCell *cell, m_items) {
        int y = (boundingRect().height() - cell->boundingRect().height()) / 2;
        QPropertyAnimation *ani = new QPropertyAnimation(cell, "pos",
                                                         m_animation);
        ani->setDuration(450);
        ani->setStartValue(cell->pos());
        ani->setEndValue(QPointF(x, y));
        QEasingCurve curve(QEasingCurve::InCubic);
        curve.setAmplitude(.25);
        curve.setPeriod(.25);
        ani->setEasingCurve(curve);
        connect(ani, SIGNAL(finished()), cell, SLOT(hide_me()));
    }
    m_animation->start();
}

void GraphicsThing::expand_right() {
    m_expanded = true;
    prepareGeometryChange();
    int x = m_min_width + 4;
    m_animation->stop();
    m_animation->clear();
    foreach(DisplayCell *cell, m_items) {
        cell->setPos(boundingRect().width() - cell->boundingRect().width(),
                     (boundingRect().height() -
                      cell->boundingRect().height()) / 2.0f);
        QPropertyAnimation *ani = new QPropertyAnimation(cell, "pos",
                                                         m_animation);
        ani->setDuration(500);
        ani->setStartValue(cell->pos());
        ani->setEndValue(QPointF(x, cell->pos().y()));
        QEasingCurve curve(QEasingCurve::OutBounce);
        curve.setAmplitude(1.0);
        curve.setPeriod(1.25);
        ani->setEasingCurve(curve);

        cell->show();
        m_animation->addAnimation(ani);
        x += cell->boundingRect().width() + 4;
    }
    m_animation->start();
}

void GraphicsThing::set_min_width(int min_width) {
    if (min_width != m_min_width) {
        prepareGeometryChange();
        m_min_width = min_width;
    }
}

void GraphicsThing::double_clicked(QPointF pos) {
    Q_UNUSED(pos);
    if (m_expanded)
        collapse();
    else
        expand_right();
}

void GraphicsThing::on_hover_start() {
    //m_hovering = true;
    //update();
}

void GraphicsThing::on_hover_stop() {
    //m_hovering = false;
    //update();
}
