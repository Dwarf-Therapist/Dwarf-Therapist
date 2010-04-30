#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>
#include "creature.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void set_scale(double new_scale);
    void expand_all();
    void collapse_all();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::MainWindow *ui;
    QGraphicsScene *m_scene;
    QList<Creature*> m_creatures;

    void layout_things();
};

#endif // MAINWINDOW_H
