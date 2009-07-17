#include <QtGui>
#include <QtDebug>

#include "mainwindow.h"
#include "ui_about.h"
#include "ui_mainwindow.h"
#include "ui_pendingchanges.h"
#include "dwarfmodel.h"
#include "statetableview.h"
#include "uberdelegate.h"
#include "customprofession.h"

MainWindow::MainWindow(QWidget *parent)
    :QMainWindow(parent)
    ,ui(new Ui::MainWindow)
    ,m_df(0)
    ,m_lbl_status(0)
	,m_settings(0)
	,m_model(new DwarfModel(this))
	,m_reading_settings(false)
{
    ui->setupUi(this);
	ui->stv->setModel(m_model);
	connect(m_model, SIGNAL(new_pending_changes(int)), this, SLOT(new_pending_changes(int)));
	connect(ui->act_clear_pending_changes, SIGNAL(triggered()), m_model, SLOT(clear_pending()));
	connect(ui->act_commit_pending_changes, SIGNAL(triggered()), m_model, SLOT(commit_pending()));
	connect(ui->act_list_pending_changes, SIGNAL(triggered()), this, SLOT(list_pending()));

	m_settings = new QSettings(QSettings::IniFormat, QSettings::UserScope, "UDP Software", "Dwarf Therapist", this);

	m_lbl_status = new QLabel(tr("not connected"), statusBar());
	statusBar()->addPermanentWidget(m_lbl_status, 0);
    set_interface_enabled(false);

	ui->cb_group_by->setItemData(0, DwarfModel::GB_NOTHING);
	ui->cb_group_by->addItem("Profession", DwarfModel::GB_PROFESSION);
	read_settings();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::read_settings() {
	m_reading_settings = true;
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
	
	m_settings->beginGroup("gui_options");
	// toolbutton text
	bool show_text = m_settings->value("show_toolbutton_text", true).toBool();
	ui->act_show_toolbutton_text->setChecked(show_text);
	show_toolbutton_text(show_text);

	// group by
	int group_by = m_settings->value("group_by", 0).toInt();
	ui->cb_group_by->setCurrentIndex(group_by);
	m_model->set_group_by(group_by);

	// delegate active color
	QColor c = m_settings->value("labors/active_bg_color", QColor(0xE0FFE0)).value<QColor>();
	ui->stv->get_delegate()->set_active_bg_color(c);

	m_settings->endGroup();
	m_reading_settings = false;
}

void MainWindow::write_settings() {
	if (m_settings && !m_reading_settings) {
		QByteArray geom = saveGeometry();
		QByteArray state = saveState();
		m_settings->beginGroup("window");
		m_settings->setValue("geometry", QVariant(geom));
		m_settings->setValue("state", QVariant(state));
		m_settings->endGroup();
		m_settings->beginGroup("gui_options");
		m_settings->setValue("show_toolbutton_text", ui->act_show_toolbutton_text->isChecked());
		m_settings->setValue("group_by", m_model->current_grouping());
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
	m_model->set_instance(m_df);
	m_model->load_dwarves();
	//ui->stv->setModel(m_model);
}

void MainWindow::set_interface_enabled(bool enabled) {
	ui->act_connect_to_DF->setEnabled(!enabled);
	ui->act_read_dwarves->setEnabled(enabled);
	ui->act_scan_memory->setEnabled(enabled);
	ui->act_expand_all->setEnabled(enabled);
	ui->act_collapse_all->setEnabled(enabled);
	ui->cb_group_by->setEnabled(enabled);
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

void MainWindow::filter_dwarves() {
	QString filter_text = ui->le_filter->text();
	//ui->stv->filter_dwarves(filter_text);
}

void MainWindow::show_toolbutton_text(bool enabled) {
	if (enabled)
		ui->mainToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	else
		ui->mainToolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
	write_settings();
}

void MainWindow::set_group_by(int group_by) {
	m_model->set_group_by(group_by);
	write_settings();
}

void MainWindow::show_about() {
	QDialog *d = new QDialog(this);
	Ui::form_about fa;
	fa.setupUi(d);
	d->show();
}

void MainWindow::add_custom_profession() {
	CustomProfession cp;
	int accepted = cp.show_builder_dialog(this);
	if (accepted)
		qDebug() << "new profession accepted!";
	else
		qDebug() << "cancelled new profession";
}

void MainWindow::new_pending_changes(int cnt) {
	ui->lbl_pending_changes->setNum(cnt);
	ui->act_clear_pending_changes->setEnabled(cnt > 0);
	ui->act_commit_pending_changes->setEnabled(cnt > 0);
	ui->act_list_pending_changes->setEnabled(cnt > 0);
	list_pending();
}

void MainWindow::list_pending() {
	ui->list_pending->clear();
	foreach(Dwarf *d, m_model->get_dirty_dwarves()) {
		foreach(int labor_id, d->get_dirty_labors()) {
			QString text = d->nice_name();
			text += " " + GameDataReader::ptr()->get_string_for_key(QString("labor_names/%1").arg(labor_id));
			QListWidgetItem *i = new QListWidgetItem(text, ui->list_pending);
			if (d->is_labor_enabled(labor_id)) {
				i->setIcon(QIcon(":img/add.png"));
			} else {
				i->setIcon(QIcon(":img/delete.png"));
			}
			i->setData(Qt::UserRole, d->id());
			ui->list_pending->addItem(i);
		}
	}
}