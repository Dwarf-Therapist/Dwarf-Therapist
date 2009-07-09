#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include "dfinstance.h"

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    public slots:
        void connect_to_df();
        void read_memory();
        void dump_mem();
        void search_dump();

private:
    Ui::MainWindow *ui;
    DFInstance *m_df;

    private slots:
        void set_interface_enabled(bool);
};

#endif // MAINWINDOW_H
