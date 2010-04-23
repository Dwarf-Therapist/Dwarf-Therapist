#ifndef DISPLAYCELL_H
#define DISPLAYCELL_H

#include <QtGui>
#include "basegraphicsobject.h"
static const int box_w = 16;
static const int box_h = 16;

class ToolTip : public QGraphicsWidget {
    Q_OBJECT
public:
    ToolTip(const QString &title, QGraphicsItem *parent = 0)
        : QGraphicsWidget(parent)
        , m_title(title)
    {
        setOpacity(0.5);
    }

    void on_added_to_scene(QGraphicsScene *scene) {
        QGraphicsWidget *label = scene->addWidget(new QLabel(m_title));
        QGraphicsWidget *btn = scene->addWidget(new QPushButton(m_title));
        QGraphicsLinearLayout *layout = new QGraphicsLinearLayout;
        layout->setOrientation(Qt::Vertical);
        layout->addItem(label);
        layout->addItem(btn);
        setLayout(layout);
    }

    QVariant itemChange(GraphicsItemChange change, const QVariant &value) {
        QVariant ret_val = QGraphicsItem::itemChange(change, value);
        if (change == ItemSceneChange) {
            QGraphicsScene *new_scene = value.value<QGraphicsScene*>();
            if (!scene() && new_scene) { // this was freshly added to new_scene
                on_added_to_scene(new_scene);
            }
        }
        return ret_val;
    }
private:
    QString m_title;
};

class DisplayCell : public BaseGraphicsObject {
    Q_OBJECT
public:
    DisplayCell(QGraphicsItem *parent = 0);
    ~DisplayCell();

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget);

private:
    ToolTip *m_tooltip;
    bool m_show_tooltip;
    void on_added_to_scene(QGraphicsScene *scene);

private slots:
    void on_hover_start();
    void on_hover_stop();
    void maybe_show_tooltip();
};

#endif // DISPLAYCELL_H
