#ifndef GRAPHICSTHING_H
#define GRAPHICSTHING_H

#include <QtGui>

class DisplayCell;

class GraphicsThing : public QGraphicsObject {
    Q_OBJECT
public:
    GraphicsThing(const QString &name, QGraphicsItem *parent = 0);

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget);

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

public slots:
    void collapse();
    void expand_down() {}
    void expand_right();
    void set_min_width(int min_width);
private:
    QString m_name;
    bool m_expanded;
    QGraphicsTextItem *m_text;
    QList<DisplayCell*> m_items;
    int m_min_width;
    QBrush m_bg_brush;
    QBrush m_bg_hover_brush;
    bool m_hovering;
    bool m_is_click;
    QParallelAnimationGroup *m_animation;
};

#endif // GRAPHICSTHING_H
