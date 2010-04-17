#include "graphicsthing.h"
#include <QtGui>

GraphicsThing::GraphicsThing(const QString &name, QGraphicsItem *parent)
    : QGraphicsObject(parent)
    , m_name(name)
    , m_expanded(false)
    , m_text(new QGraphicsTextItem(name, this))
    , m_min_width(0)
    , m_bg_brush(QBrush(QColor(220, 220, 220, 255)))
    , m_bg_hover_brush(QBrush(Qt::yellow))
    , m_hovering(false)
    , m_animation(new QParallelAnimationGroup(this))
{
    setFlag(ItemIsSelectable, true);
    setAcceptsHoverEvents(true);
    m_text->setFont(QFont("Helvetica", 9));
    m_text->setDefaultTextColor(Qt::black);
    m_text->setPos(1, 1);
    //this->setGraphicsEffect(new QGraphicsDropShadowEffect(this));

    m_items.clear();
    for (int i = 0; i < 22; ++i) {
        QGraphicsRectItem *r = new QGraphicsRectItem(this);
        r->setPen(QPen(Qt::gray));
        r->setBrush(Qt::black);
        r->hide();
        r->setRect(0, 0, 16, 16);
        m_items << r;
    }
}

QRectF GraphicsThing::boundingRect() const {
    int w = m_min_width;
    if (w == 0) {
        w = m_text->boundingRect().width();
    }
    return QRectF(0, 0, w, m_text->boundingRect().height() + 2);
}

void GraphicsThing::paint(QPainter *p, const QStyleOptionGraphicsItem *opt, QWidget *w) {
    p->save();
    p->setPen(QPen(QColor(192, 192, 192, 255), 1));
    p->setBrush(m_hovering ? m_bg_hover_brush : m_bg_brush);
    QRectF r = boundingRect();
    p->drawRect(0, 0, r.width(), r.height());
    p->restore();
}

void GraphicsThing::collapse() {
    m_expanded = false;
    foreach(QGraphicsItem *r, m_items) {
        r->setPos(0, 0);
        r->hide();
    }
}

void GraphicsThing::expand_right() {
    m_expanded = true;
    int x = boundingRect().width() + 4;
    foreach(QGraphicsItem *item, m_items) {
        item->setPos(x, 0);
        item->show();
        x += item->boundingRect().width() + 4;
    }
}

void GraphicsThing::set_min_width(int min_width) {
    if (min_width != m_min_width) {
        prepareGeometryChange();
        m_min_width = min_width;
    }
}

/* Mouse Handlers */
void GraphicsThing::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
    qDebug() << "mouse enter";
    m_hovering = true;
    m_is_click = false;
    update();
    QGraphicsObject::hoverEnterEvent(event);
}
void GraphicsThing::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
    qDebug() << "mouse exit";
    m_hovering = false;
    m_is_click = false;
    update();
    QGraphicsObject::hoverLeaveEvent(event);
}
void GraphicsThing::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    m_is_click = true;
    QGraphicsObject::mousePressEvent(event);
}

void GraphicsThing::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    if (m_is_click) {
        qDebug() << "single click";
    }
    QGraphicsObject::mouseReleaseEvent(event);
}

void GraphicsThing::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
    m_is_click = false;
    qDebug() << "double click";
    if (m_expanded)
        collapse();
    else
        expand_right();
    QGraphicsObject::mouseDoubleClickEvent(event);
}
