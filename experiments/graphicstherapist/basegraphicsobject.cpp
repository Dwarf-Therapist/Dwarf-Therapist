#include "basegraphicsobject.h"

BaseGraphicsObject::BaseGraphicsObject(QGraphicsItem *parent)
    : QGraphicsObject(parent)
    , m_is_click(false)
    , m_is_hovering(false)
    , m_double_clicked(false)
{
    setAcceptHoverEvents(true);
    this->setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton | Qt::MidButton);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
}

QRectF BaseGraphicsObject::boundingRect() const {
    return QRectF(0, 0, 100, 20);
}
void BaseGraphicsObject::paint(QPainter *p, const QStyleOptionGraphicsItem *opt,
                               QWidget *widget) {
    p->drawText(0, 0, "NOT IMPLEMENTED");
}

QVariant BaseGraphicsObject::itemChange(GraphicsItemChange change,
                                        const QVariant &value) {
    QVariant ret_val = QGraphicsItem::itemChange(change, value);
    if (change == ItemSceneChange) {
        QGraphicsScene *new_scene = value.value<QGraphicsScene*>();
        if (!scene() && new_scene) {
            on_added_to_scene(new_scene);
        }
    }
    return ret_val;
}

void BaseGraphicsObject::maybe_single_click() {
    if (!m_double_clicked) {
        //qDebug() << "single click for sure";
        emit single_click();
        emit single_click(m_last_click_position);
    }
    m_double_clicked = false;
}

void BaseGraphicsObject::maybe_update() {
    prepareGeometryChange();
    //QGraphicsObject::update();
}


void BaseGraphicsObject::on_added_to_scene(QGraphicsScene *scene) {
    Q_UNUSED(scene);
    // subclasses may want to do things once they are added to the scene
}

//! Mouse Handlers
void BaseGraphicsObject::hoverEnterEvent(QGraphicsSceneHoverEvent *e) {
    //qDebug() << "mouse enter" << e->pos();
    m_is_hovering = true;
    m_is_click = false;
    emit hover_start();
    emit hover_start(e->pos());
    QGraphicsObject::hoverEnterEvent(e);
}

void BaseGraphicsObject::hoverLeaveEvent(QGraphicsSceneHoverEvent *e) {
    //qDebug() << "mouse exit";
    m_is_hovering = false;
    m_is_click = false;
    emit hover_stop();
    emit hover_stop(e->pos());
    QGraphicsObject::hoverLeaveEvent(e);
}

void BaseGraphicsObject::mousePressEvent(QGraphicsSceneMouseEvent *e) {
    //qDebug() << "PRESS";
    m_is_click = true;
    m_last_click_position = e->pos();
    QGraphicsObject::mousePressEvent(e);
}

void BaseGraphicsObject::mouseReleaseEvent(QGraphicsSceneMouseEvent *e) {
    //qDebug() << "RELEASE";
    if (m_is_click) {
        m_is_click = false;
        m_last_click_position = e->pos();
        QTimer::singleShot(100, this, SLOT(maybe_single_click()));
    }
    QGraphicsObject::mouseReleaseEvent(e);
}

void BaseGraphicsObject::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *e) {
    //qDebug() << "DOUBLE CLICK";
    m_is_click = false;
    m_double_clicked = true;
    emit double_click();
    emit double_click(e->pos());
    QGraphicsObject::mouseDoubleClickEvent(e);
}
