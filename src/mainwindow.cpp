#include <QMessageBox>
#include <QLabel>
#include <QProgressDialog>
#include <QSettings>

#include <QtDebug>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dwarfmodel.h"

MainWindow::MainWindow(QWidget *parent)
    :QMainWindow(parent)
    ,ui(new Ui::MainWindow)
    ,m_df(0)
    ,m_lbl_status(0)
	,m_settings(0)
{
    ui->setupUi(this);

	m_settings = new QSettings(QSettings::IniFormat, QSettings::UserScope, "UDP Software", "Dwarf Therapist", this);
	read_settings();

	m_lbl_status = new QLabel(tr("not connected"), statusBar());
	statusBar()->addPermanentWidget(m_lbl_status, 0);
    set_interface_enabled(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::read_settings() {
	// Geometry
	m_settings->beginGroup("window");
	QByteArray geom = m_settings->value("geometry").toByteArray();
	if (!geom.isEmpty()) {
		restoreGeometry(geom);
	}
	// Toolbars etc... 
	QByteArray state = m_settings->value("state").toByteArray();
	if (!state.isEmpty()) {
		restoreState(state);
	}
	m_settings->endGroup();

}

void MainWindow::write_settings() {
	if (m_settings) {
		QByteArray geom = saveGeometry();
		QByteArray state = saveState();
		m_settings->beginGroup("window");
		m_settings->setValue("geometry", QVariant(geom));
		m_settings->setValue("state", QVariant(state));
		m_settings->endGroup();
	}
}

void MainWindow::closeEvent(QCloseEvent *evt) {
	write_settings();
	QWidget::closeEvent(evt);
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
}

void MainWindow::read_dwarves() {
	GameDataReader *gdr = GameDataReader::ptr();
	QVector<Dwarf*> dwarves = m_df->load_dwarves();

	DwarfModel *m = new DwarfModel(this);
	for (int i=0; i<dwarves.size(); ++i) {
		QStandardItem *name = new QStandardItem(dwarves[i]->to_string());
		QStandardItem *skills = new QStandardItem();
		QString out;
		foreach(Skill s, *dwarves.at(i)->get_skills()) {
			out += s.to_string();
			out += "\n";
		}
		skills->setText(out);
		m->setItem(i, 0, name);
		m->setItem(i, 1, skills);
		for (int j=0; j < 102; ++j) {
			bool enabled = dwarves[i]->is_labor_enabled(j);
			QStandardItem *tmp = new QStandardItem(QString::number(dwarves[i]->get_rating_for_skill(j)));
			tmp->setToolTip(gdr->get_skill_name(j));
			tmp->setCheckable(true);
			tmp->setCheckState(enabled ? Qt::Checked : Qt::Unchecked);
			m->setItem(i, j+2, tmp);
		}
	}
	ui->tbl_main->setModel(m);

}

void MainWindow::set_interface_enabled(bool enabled) {
	ui->act_connect_to_DF->setEnabled(!enabled);
	ui->act_read_dwarves->setEnabled(enabled);
	ui->act_scan_memory->setEnabled(enabled);
}

void MainWindow::scan_memory() {
    QProgressDialog *pd = new QProgressDialog(tr("Scanning Memory"), tr("Cancel"), 0, 1, this);
    connect(m_df, SIGNAL(scan_total_steps(int)), pd, SLOT(setMaximum(int)));
    connect(m_df, SIGNAL(scan_progress(int)), pd, SLOT(setValue(int)));
    connect(m_df, SIGNAL(scan_message(QString)), pd, SLOT(setLabelText(QString)));
    connect(pd, SIGNAL(canceled()), m_df, SLOT(cancel_scan()));
    pd->show();

    int language_addr = m_df->find_language_vector();
    int translation_addr = m_df->find_translation_vector();
    int creature_addr = m_df->find_creature_vector();
	pd->deleteLater();

    qDebug() << "LANGUAGE VECTOR:   " << hex << language_addr;
    qDebug() << "TRANSLATION VECTOR:" << hex << translation_addr;
    qDebug() << "CREATURE VECTOR:   " << hex << creature_addr;
}


