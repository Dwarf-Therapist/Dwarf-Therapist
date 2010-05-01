#include "creature.h"
#include "displaycell.h"
#include <QtGui>

Creature::Creature(const QString &name, const QString &prof,
                   int highest_skill_level, QGraphicsItem *parent)
    : BaseGraphicsObject(parent)
    , m_name(name)
    , m_prof(prof)
    , m_highest_skill_level(highest_skill_level)
    , m_expanded(false)
    , m_hovering(false)
    , m_text(new QGraphicsTextItem(name, this))
    , m_min_width(0) // boundingRect() relies on this being init'd to 0
    , m_bg_brush(QBrush(QColor(0, 120, 0, 128)))
    , m_bg_hover_brush(QBrush(QColor(64, 64, 64, 128)))
    , m_animation(new QParallelAnimationGroup(this))
{
    setFlag(ItemIsSelectable, true);
    setAcceptsHoverEvents(true);
    m_text->setFont(QFont("Helvetica", 7));
    m_text->setDefaultTextColor(Qt::white);
    m_text->setPos(1, 1);
    //this->setGraphicsEffect(new QGraphicsDropShadowEffect(this));

    connect(this, SIGNAL(hover_start()), SLOT(on_hover_start()));
    connect(this, SIGNAL(hover_stop()), SLOT(on_hover_stop()));
    //connect(this, SIGNAL(single_click()), SLOT(show_details()));
    connect(this, SIGNAL(double_click()), SLOT(toggle_expand()));

    m_items.clear();
    for (int i = 0; i < 32; ++i) {
        DisplayCell *cell = new DisplayCell(this);
        //cell->setPos(100, 21);
        cell->hide();
        m_items << cell;
    }
}

void Creature::show_details() {
    QPushButton *btn = new QPushButton("close");
    QGraphicsWidget *w = new QGraphicsWidget(this);
    w->setPos(0, 30);
    QGraphicsWidget *label = scene()->addWidget(new QLabel(m_name));
    QGraphicsWidget *btn_w = scene()->addWidget(btn);
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout;
    layout->setOrientation(Qt::Vertical);
    layout->addItem(label);
    layout->addItem(btn_w);
    w->setLayout(layout);
    connect(btn, SIGNAL(pressed()), this, SLOT(hide_details()));

}

void Creature::hide_details() {
    qDebug() << "would hide";
}

void Creature::on_added_to_scene(QGraphicsScene *scene) {
    Q_UNUSED(scene);
    //qDebug() << "added to scene:" << scene;
}

QRectF Creature::boundingRect() const {
    int w = m_min_width;
    if (w == 0) {
        w = m_text->boundingRect().width();
    }
    QRectF bounds(0, 0, w, m_text->boundingRect().height() + 2);

    foreach(QGraphicsItem *child, childItems()) {
        QPointF p = child->pos();
        QRectF r = child->boundingRect();

        float x = mapFromItem(child, r.topRight()).x();
        float y = mapFromItem(child, r.bottomRight()).y();
        if (x > bounds.width()) {
            bounds.setWidth(x + 2);
        }
        if (y > bounds.height()) {
            bounds.setHeight(y + 2);
        }
    }
    return bounds;
}

void Creature::paint(QPainter *p, const QStyleOptionGraphicsItem *opt,
                          QWidget *w) {
    Q_UNUSED(w);
    p->save();
    p->setPen(QPen(QColor(192, 192, 192), 0.5));
    p->setBrush(m_hovering ? m_bg_hover_brush : m_bg_brush);
    QRectF r = opt->rect.adjusted(0.5f, 0.5f, -0.5f, -0.5f);
    p->fillRect(r, p->brush());
    p->drawLine(r.topLeft(), r.topRight());
    p->drawLine(r.bottomLeft(), r.bottomRight());
    p->restore();
}

void Creature::toggle_expand() {
    if (m_expanded) {
        collapse();
    } else {
        expand();
    }
}

void Creature::collapse() {
    m_expanded = false;
    int x = 0;
    int y = boundingRect().height() / 2.0f;
    m_animation->stop();
    m_animation->clear();
    prepareGeometryChange();
    foreach(DisplayCell *cell, m_items) {
        QPropertyAnimation *ani = new QPropertyAnimation(cell, "pos",
                                                         m_animation);
        ani->setDuration(450);
        ani->setStartValue(cell->pos());
        ani->setEndValue(QPointF(x, y));
        QEasingCurve curve(QEasingCurve::InCubic);
        curve.setAmplitude(.25);
        curve.setPeriod(.25);
        ani->setEasingCurve(curve);
        connect(ani, SIGNAL(finished()), cell, SLOT(hide_me()));
        // if this is the first item, update ourselves as it moves
        if (cell == m_items.at(0))
            connect(ani, SIGNAL(valueChanged(QVariant)), SLOT(maybe_update()));
    }
    m_animation->start();
}

void Creature::expand() {
    m_expanded = true;
    prepareGeometryChange();

    // base position for a cell (home-base)
    int x = m_min_width + 4;
    int y = boundingRect().height() / 2.0f;

    // stop and destroy any existing animations
    m_animation->stop();
    m_animation->clear();

    // setup property animations for each member cell
    foreach(DisplayCell *cell, m_items) {
        QPropertyAnimation *ani = new QPropertyAnimation(cell, "pos",
                                                         m_animation);
        ani->setDuration(500);
        ani->setStartValue(cell->pos());
        ani->setEndValue(QPointF(x, y));
        QEasingCurve curve(QEasingCurve::OutCubic);
        curve.setAmplitude(1.0);
        curve.setPeriod(1.25);
        ani->setEasingCurve(curve);

        cell->show();
        m_animation->addAnimation(ani);
        x += cell->boundingRect().width() + 4;
        // if this is the first item, update ourselves as it moves
        if (cell == m_items.at(0))
            connect(ani, SIGNAL(valueChanged(QVariant)), SLOT(maybe_update()));
    }
    m_animation->start();
}

void Creature::set_min_width(int min_width) {
    if (min_width != m_min_width) {
        prepareGeometryChange();
        m_min_width = min_width;
    }
}

void Creature::on_hover_start() {
    m_hovering = true;
    maybe_update();
}

void Creature::on_hover_stop() {
    m_hovering = false;
    maybe_update();
}
