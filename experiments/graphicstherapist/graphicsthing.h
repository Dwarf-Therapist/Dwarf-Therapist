#ifndef GRAPHICSTHING_H
#define GRAPHICSTHING_H

#include <QtGui>
#include "basegraphicsobject.h"

class DisplayCell;

class GraphicsThing : public BaseGraphicsObject {
    Q_OBJECT
public:
    GraphicsThing(const QString &name, Qt::Orientation = Qt::Horizontal,
                  QGraphicsItem *parent = 0);

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget);

public slots:
    void toggle_expand();
    void expand();
    void collapse();
    void set_min_width(int min_width);
    void on_hover_start();
    void on_hover_stop();
    void show_details();
    void hide_details();

private:
    QString m_name;
    Qt::Orientation m_orientation;
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
