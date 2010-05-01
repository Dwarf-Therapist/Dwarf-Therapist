#ifndef CREATURE_H
#define CREATURE_H

#include <QtGui>
#include "basegraphicsobject.h"

/* Graphics overhaul of dwarves for the data grid

   we need to be able to show any creature along with several supplimentary
   data fields attached to or somewhere around the core graphic

   The hierarcy of drawables should be build up before polluting it with too
   much specific creature data.

   BaseGraphicsObject -> MultiStateGraphicsObject


*/

class DisplayCell;

class Creature : public BaseGraphicsObject {
    Q_OBJECT
public:
    Creature(const QString &name, const QString &prof, int higest_skill_level,
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
    QString m_prof;
    int m_highest_skill_level;
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

#endif // Creature_H
