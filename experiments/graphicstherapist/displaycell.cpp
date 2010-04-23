#include "displaycell.h"
#include <QtGui>

DisplayCell::DisplayCell(QGraphicsItem *parent)
    : BaseGraphicsObject(parent)
    , m_tooltip(new ToolTip("My Tooltip"))
    , m_show_tooltip(false)
{
    setAcceptHoverEvents(true);
    m_tooltip->setZValue(1);
    m_tooltip->hide();

    connect(this, SIGNAL(hover_start()), SLOT(on_hover_start()));
    connect(this, SIGNAL(hover_stop()), SLOT(on_hover_stop()));
}

DisplayCell::~DisplayCell() {
    if (m_tooltip)
        m_tooltip->deleteLater();
}

QRectF DisplayCell::boundingRect() const {
    return QRectF(0, 0, box_w, box_h);
}

void DisplayCell::paint(QPainter *p, const QStyleOptionGraphicsItem *opt,
                        QWidget *widget) {
    p->save();
    p->setPen(QPen(Qt::black, 0.5));
    p->setBrush(Qt::gray);
    p->drawRect(boundingRect().adjusted(0.5f, 0.5f, -0.5f, -0.5f));
    p->restore();
}

void DisplayCell::on_hover_start() {
    qDebug() << "Hover";
    m_show_tooltip = true;
    QTimer::singleShot(100, this, SLOT(maybe_show_tooltip()));
}

void DisplayCell::on_hover_stop() {
    m_show_tooltip = false;
    qDebug() << "Leave";
    prepareGeometryChange();
    m_tooltip->hide();
}

void DisplayCell::on_added_to_scene(QGraphicsScene *scene) {
    scene->addItem(m_tooltip);
}

void DisplayCell::maybe_show_tooltip() {
    if (m_show_tooltip) {
        qDebug() << "show it!";
        m_tooltip->setPos(mapToScene(boundingRect().bottomRight()) +
                          QPoint(4, 4));
        m_tooltip->show();
        QPropertyAnimation *ani = new QPropertyAnimation(m_tooltip, "scale");
        QPropertyAnimation *ani2 = new QPropertyAnimation(m_tooltip, "opacity");

        ani->setDuration(200);
        ani2->setDuration(500);
        ani->setStartValue(0.4f);
        ani2->setStartValue(0);
        ani->setEndValue(1.0f);
        ani2->setEndValue(1.0f);
        ani->setEasingCurve(QEasingCurve::InExpo);

        ani->start();
        ani2->start();
        connect(ani, SIGNAL(finished()), ani, SLOT(deleteLater()));
        connect(ani2, SIGNAL(finished()), ani2, SLOT(deleteLater()));
    }
    m_show_tooltip = false;
}
