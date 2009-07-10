#include <QMessageBox>
#include <QLabel>
#include <QProgressDialog>

#include <QtDebug>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dwarfmodel.h"

MainWindow::MainWindow(QWidget *parent)
    :QMainWindow(parent)
    ,ui(new Ui::MainWindow)
	,m_lbl_status(0)
    ,m_df(0)
{
    ui->setupUi(this);

	m_lbl_status = new QLabel(tr("not connected"), statusBar());
	statusBar()->addPermanentWidget(m_lbl_status, 0);
    set_interface_enabled(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::connect_to_df() {
    if (m_df) {
        delete m_df;
        set_interface_enabled(false);
        m_df = 0;
    }
    m_df = DFInstance::find_running_copy(this);
    if (!m_df) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("Unable to locate a running copy of Dwarf "
                                "Fortress, are you sure it's running?"));
        return;
    }
    m_lbl_status->setText(tr("connected"));
    set_interface_enabled(true);


	/*
    QProgressDialog *pd = new QProgressDialog(tr("Scanning for Creature Vector"), tr("Cancel"), 0, 1, this);
    connect(m_df, SIGNAL(scan_total_steps(int)), pd, SLOT(setMaximum(int)));
    connect(m_df, SIGNAL(scan_progress(int)), pd, SLOT(setValue(int)));
    connect(pd, SIGNAL(canceled()), m_df, SLOT(cancel_scan()));
    pd->show();

    uint creature_addr = m_df->find_creature_vector();
	*/
}

void MainWindow::read_dwarves() {
	QVector<Dwarf*> dwarves = m_df->load_dwarves();

	DwarfModel *m = new DwarfModel(this);
	for (int i=0; i<dwarves.size(); ++i) {
		QStandardItem *item = new QStandardItem(dwarves[i]->to_string());
		m->setItem(i, 0, item);
	}
	ui->tbl_main->setModel(m);

}

void MainWindow::set_interface_enabled(bool enabled) {
    /*
	ui->btn_read->setEnabled(enabled);
    ui->btn_dump->setEnabled(enabled);
    ui->edit_s_address->setEnabled(enabled);
    ui->edit_e_address->setEnabled(enabled);
    */
}
