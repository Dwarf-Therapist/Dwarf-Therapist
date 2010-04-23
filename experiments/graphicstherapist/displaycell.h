#ifndef DISPLAYCELL_H
#define DISPLAYCELL_H

#include <QtGui>
static const int box_w = 16;
static const int box_h = 16;


class ToolTip : public QGraphicsWidget {
    Q_OBJECT
public:
    ToolTip(const QString &title, QGraphicsItem *parent = 0)
        : QGraphicsWidget(parent)
        , m_title(title)
    {
        //setOpacity(0.5);
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
        QVariant ret_val;
        if (change == ItemSceneChange) {
            QGraphicsScene *new_scene = value.value<QGraphicsScene*>();
            if (!scene() && new_scene) { // this was freshly added to new_scene
                on_added_to_scene(new_scene);
            }
        } else {
            ret_val = QGraphicsItem::itemChange(change, value);
        }
        return ret_val;
    }
private:
    QString m_title;
};

class DisplayCell : public QGraphicsObject {
    Q_OBJECT
public:
    DisplayCell(QGraphicsItem *parent = 0);
    ~DisplayCell();

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
    ToolTip *m_tooltip;
};

#endif // DISPLAYCELL_H
