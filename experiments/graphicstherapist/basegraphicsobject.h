#ifndef BASEGRAPHICSOBJECT_H
#define BASEGRAPHICSOBJECT_H

#include <QtGui>

class BaseGraphicsObject : public QGraphicsObject
{
    Q_OBJECT
public:
    BaseGraphicsObject(QGraphicsItem *parent = 0);

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget = 0);

public slots:
    void maybe_update();

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private:

    bool m_is_click;
    bool m_is_hovering;
    bool m_double_clicked;
    QPointF m_last_click_position;
    //! called when an item is first added to a scene
    virtual void on_added_to_scene(QGraphicsScene *scene);

private slots:
    //! we have to manually detect single vs. double clicks
    void maybe_single_click();


signals:
    void hover_start();
    void hover_start(QPointF);
    void hover_stop();
    void hover_stop(QPointF);
    void single_click();
    void single_click(QPointF);
    void double_click();
    void double_click(QPointF);
};
#endif // BASEGRAPHICSOBJECT_H
