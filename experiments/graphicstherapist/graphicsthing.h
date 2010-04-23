#ifndef GRAPHICSTHING_H
#define GRAPHICSTHING_H

#include <QtGui>
#include "basegraphicsobject.h"

class DisplayCell;

class GraphicsThing : public BaseGraphicsObject {
    Q_OBJECT
public:
    GraphicsThing(const QString &name, QGraphicsItem *parent = 0);

    /*QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget = 0);
               */

public slots:
    void toggle_expand();
    void collapse();
    void expand_down() {}
    void expand_right();
    void set_min_width(int min_width);
    void double_clicked(QPointF pos);
    void on_hover_start();
    void on_hover_stop();

private:
    QString m_name;
    bool m_expanded;
    bool m_hovering;
    QGraphicsTextItem *m_text;
    QList<DisplayCell*> m_items;
    int m_min_width;
    QBrush m_bg_brush;
    QBrush m_bg_hover_brush;
    QParallelAnimationGroup *m_animation;

    void on_added_to_scene(QGraphicsScene *scene);
};

#endif // GRAPHICSTHING_H
