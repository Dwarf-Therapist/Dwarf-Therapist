#include <QtGui>
#include <QtDebug>

#include "mainwindow.h"
#include "ui_about.h"
#include "ui_mainwindow.h"
#include "ui_pendingchanges.h"
#include "optionsmenu.h"
#include "dwarfmodel.h"
#include "dfinstance.h"
#include "statetableview.h"
#include "uberdelegate.h"
#include "customprofession.h"
#include "defines.h"

MainWindow::MainWindow(QWidget *parent)
    :QMainWindow(parent)
    ,ui(new Ui::MainWindow)
	,m_options_menu(new OptionsMenu(this))
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

	connect(ui->stv, SIGNAL(new_custom_profession(Dwarf*)), this, SLOT(new_custom_profession(Dwarf *)));

	connect(m_options_menu, 
			SIGNAL(picker_changed(MainWindow::CONFIGURABLE_COLORS, const QColor&)),
			this, 
			SLOT(color_changed(MainWindow::CONFIGURABLE_COLORS, const QColor &)));

	m_settings = new QSettings(QSettings::IniFormat, QSettings::UserScope, COMPANY, PRODUCT, this);

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
	m_settings->endGroup();


	// options menu settings
	m_options_menu->read_settings(m_settings);

	// delegate active color
	//QColor c = m_settings->value("labors/active_bg_color", QColor(0xE0FFE0)).value<QColor>();
	//ui->stv->get_delegate()->set_active_bg_color(c);

	
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

		// options menu settings
		m_options_menu->write_settings(m_settings);
	}
}

void MainWindow::closeEvent(QCloseEvent *evt) {
	write_settings();
	evt->accept();
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
    m_lbl_status->setText(tr("Connected"));
    set_interface_enabled(true);
}

void MainWindow::read_dwarves() {
	m_model->set_instance(m_df);
	m_model->load_dwarves();
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
		ui->main_toolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	else
		ui->main_toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
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
	bool on_off = cnt > 0;
	ui->lbl_pending_changes->setNum(cnt);
	ui->btn_clear->setEnabled(on_off);
	ui->btn_commit->setEnabled(on_off);
	ui->act_clear_pending_changes->setEnabled(on_off);
	ui->act_commit_pending_changes->setEnabled(on_off);
	ui->act_list_pending_changes->setEnabled(on_off);
	list_pending();
}

void MainWindow::list_pending() {
	ui->tree_pending->clear();
	foreach(Dwarf *d, m_model->get_dirty_dwarves()) {
		QVector<int> labors = d->get_dirty_labors();
		QTreeWidgetItem *d_item = new QTreeWidgetItem(ui->tree_pending);
		d_item->setText(0, d->nice_name() + "(" + QString::number(labors.size()) + ")");
		d_item->setData(0, Qt::UserRole, d->id());
		foreach(int labor_id, labors) {
			QString labor_name = GameDataReader::ptr()->get_string_for_key(QString("labor_names/%1").arg(labor_id));
			QTreeWidgetItem *i = new QTreeWidgetItem(d_item);
			i->setText(0, labor_name);
			if (d->is_labor_enabled(labor_id)) {
				i->setIcon(0, QIcon(":img/add.png"));
			} else {
				i->setIcon(0, QIcon(":img/delete.png"));
			}
			i->setData(0, Qt::UserRole, d->id());
			//ui->list_pending->addItem(i);
		}
	}
	ui->tree_pending->expandAll();
}

void MainWindow::open_options_menu() {
	m_options_menu->show();
}

void MainWindow::color_changed(MainWindow::CONFIGURABLE_COLORS picker, const QColor &c) {
	UberDelegate *d = ui->stv->get_delegate();
	switch (picker) {
		case CC_CURSOR:			d->color_cursor = c;		break;
		case CC_ACTIVE_LABOR:	d->color_active_labor = c;	break;
		case CC_ACTIVE_GROUP:	d->color_active_group = c;	break;
		case CC_DIRTY_BORDER:	d->color_dirty_border = c;	break;
		default:
			qWarning() << "some color changed and I don't know what it is.";
	}
	
}

void MainWindow::new_custom_profession(Dwarf *d) {
	CustomProfession cp(d, this);
	int accepted = cp.show_builder_dialog(this);
}