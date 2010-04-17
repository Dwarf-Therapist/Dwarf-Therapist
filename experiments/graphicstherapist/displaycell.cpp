#include "displaycell.h"
#include <QtGui>

DisplayCell::DisplayCell(QGraphicsItem *parent)
    : QGraphicsObject(parent)
{
    setAcceptHoverEvents(true);
}

QRectF DisplayCell::boundingRect() const {
    return QRectF(0, 0, box_w, box_h);
}

void DisplayCell::paint(QPainter *p, const QStyleOptionGraphicsItem *opt,
                        QWidget *widget) {
    p->save();
    p->setPen(QPen(Qt::black, 0.5));
    p->setBrush(Qt::gray);
    p->drawRect(boundingRect());
    p->restore();
}

void DisplayCell::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {

}

void DisplayCell::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {

}

void DisplayCell::mousePressEvent(QGraphicsSceneMouseEvent *event) {

}

void DisplayCell::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {

}

void DisplayCell::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {

}
