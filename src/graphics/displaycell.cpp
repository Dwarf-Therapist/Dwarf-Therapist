/*
Dwarf Therapist
Copyright (c) 2010 Trey Stout (chmod)

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
#include "displaycell.h"

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
