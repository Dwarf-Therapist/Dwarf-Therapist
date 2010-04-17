#ifndef DISPLAYCELL_H
#define DISPLAYCELL_H

#include <QGraphicsObject>
static const int box_w = 16;
static const int box_h = 16;

class DisplayCell : public QGraphicsObject {
    Q_OBJECT
public:
    DisplayCell(QGraphicsItem *parent = 0);

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget);

public slots:
    void hide_me() {hide();}
    void show_me() {show();}

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

private:
};

#endif // DISPLAYCELL_H
