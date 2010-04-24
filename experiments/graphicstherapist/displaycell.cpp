#include "displaycell.h"
#include <QtGui>

DisplayCell::DisplayCell(QGraphicsItem *parent)
    : BaseGraphicsObject(parent)
    , m_tooltip(new ToolTip("My Tooltip"))
    , m_show_tooltip(false)
    , m_labor_on(false)
    , m_bg(QBrush(Qt::gray))
    , m_bg_selected(QBrush(Qt::yellow))
{
    setAcceptHoverEvents(true);
    m_tooltip->setZValue(1);
    m_tooltip->hide();
    this->setFlag(QGraphicsItem::ItemIsFocusable, true);

    connect(this, SIGNAL(hover_start()), SLOT(on_hover_start()));
    connect(this, SIGNAL(hover_stop()), SLOT(on_hover_stop()));
    connect(this, SIGNAL(single_click()), SLOT(toggle()));
}

DisplayCell::~DisplayCell() {
    if (m_tooltip)
        m_tooltip->deleteLater();
}

QRectF DisplayCell::boundingRect() const {
    return QRectF(-box_w / 2.0f, -box_h / 2.0f, box_w, box_h);
}

void DisplayCell::paint(QPainter *p, const QStyleOptionGraphicsItem *opt,
                        QWidget *widget) {
    p->save();
    if (scene()->focusItem() == this) {
        p->setPen(QPen(Qt::white, 0.5));
    } else {
        p->setPen(QPen(Qt::black, 0.5));
    }
    if (isEnabled() && m_labor_on) {
        p->setBrush(m_bg_selected);
    } else {
        p->setBrush(m_bg);
    }
    p->drawRect(boundingRect().adjusted(0.5f, 0.5f, -0.5f, -0.5f));
    p->restore();
}

void DisplayCell::on_hover_start() {
    scene()->setFocusItem(this, Qt::MouseFocusReason);
    //qDebug() << "Hover";
    m_show_tooltip = true;
    QTimer::singleShot(100, this, SLOT(maybe_show_tooltip()));
}

void DisplayCell::on_hover_stop() {
    //qDebug() << "Leave";
    m_show_tooltip = false;
    m_tooltip->hide();
}

void DisplayCell::on_added_to_scene(QGraphicsScene *scene) {
    scene->addItem(m_tooltip);
}

void DisplayCell::maybe_show_tooltip() {
    if (m_show_tooltip) {
        m_tooltip->setPos(mapToScene(boundingRect().bottomRight()) +
                          QPoint(4, 4));
        m_tooltip->show();

        /*
        QPropertyAnimation *ani = new QPropertyAnimation(m_tooltip, "scale");
        connect(ani, SIGNAL(finished()), ani, SLOT(deleteLater()));
        ani->setDuration(200);
        ani->setStartValue(0.4f);
        ani->setEndValue(1.0f);
        ani->setEasingCurve(QEasingCurve::InExpo);
        */

        QPropertyAnimation *ani2 = new QPropertyAnimation(m_tooltip, "opacity");
        connect(ani2, SIGNAL(finished()), ani2, SLOT(deleteLater()));
        ani2->setDuration(500);
        ani2->setStartValue(0);
        ani2->setEndValue(1.0f);

        //ani->start();
        ani2->start();
    }
    m_show_tooltip = false;
}

void DisplayCell::toggle() {
    qDebug() << this << "toggled!";
    m_labor_on = !m_labor_on;
    maybe_update();
    //update();
}
