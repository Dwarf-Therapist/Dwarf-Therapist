#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "graphicsthing.h"

static const int box_w = 16;
static const int box_h = 16;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_scene(new QGraphicsScene(0, 0, 800, 400, this))
{
    ui->setupUi(this);
    srand(QTime(0,0,0).secsTo(QTime::currentTime()));
    resize(850, 550);

    ui->gv_main->setScene(m_scene);
    ui->gv_main->setInteractive(true);
    ui->gv_main->setMouseTracking(true);
    ui->gv_main->setBackgroundBrush(QBrush(QColor(48, 48, 48)));
    ui->gv_main->setRenderHint(QPainter::Antialiasing, true);
    //ui->gv_main->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

    m_things.clear();
    m_things << new GraphicsThing("Urist McFoobizzle");
    m_things << new GraphicsThing("Cog Zanderpither");
    m_things << new GraphicsThing("Wank WaggleFoot");
    foreach(GraphicsThing *t, m_things) {
        m_scene->addItem(t);
    }
    layout_things();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::changeEvent(QEvent *e) {
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::layout_things() {
    qStableSort(m_things);
    int y = 0;
    int max_width = 0;
    foreach(GraphicsThing *t, m_things) {
        QRectF r = t->boundingRect();
        //qDebug() << "Y:" << y << "RECT:" << r;
        t->setPos(0, y);
        y += r.height();
        if (r.width() > max_width)
            max_width = r.width();
    }
    qDebug() << "max width" << max_width;
    foreach(GraphicsThing *t, m_things) {
        t->set_min_width(max_width);
        qDebug() << t << "rect:" << t->boundingRect();
    }
    ui->gv_main->ensureVisible(m_things.at(0));;
}

void MainWindow::set_scale(double new_scale) {
    if (ui->gv_main) {
        ui->gv_main->resetMatrix();
        ui->gv_main->scale(new_scale, new_scale);
        ui->gv_main->centerOn(0, 0);
    }
}
