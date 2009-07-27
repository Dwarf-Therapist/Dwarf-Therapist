/*
Dwarf Therapist
Copyright (c) 2009 Trey Stout (chmod)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
#include <QtGui>
#include <QtNetwork>
#include <QtDebug>
#include <QxtLogger>

#include "mainwindow.h"
#include "ui_about.h"
#include "ui_mainwindow.h"
#include "ui_pendingchanges.h"
#include "optionsmenu.h"
#include "aboutdialog.h"
#include "dwarfmodel.h"
#include "dfinstance.h"
#include "memorylayout.h"
#include "statetableview.h"
#include "uberdelegate.h"
#include "customprofession.h"
#include "labor.h"
#include "defines.h"
#include "version.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
	, m_options_menu(new OptionsMenu(this))
	, m_about_dialog(new AboutDialog(this))
    , m_df(0)
    , m_lbl_status(0)
	, m_settings(0)
	, m_model(new DwarfModel(this))
	, m_proxy(new DwarfModelProxy(this))
	, m_custom_professions(QVector<CustomProfession*>())
	, m_http(new QHttp(this))
	, m_reading_settings(false)
	, m_temp_cp(0)
{
    ui->setupUi(this);
	m_proxy->setSourceModel(m_model);
	ui->stv->set_model(m_model, m_proxy);

	LOGD << "setting up connections for MainWindow";
	connect(m_model, SIGNAL(new_pending_changes(int)), this, SLOT(new_pending_changes(int)));
	connect(ui->act_clear_pending_changes, SIGNAL(triggered()), m_model, SLOT(clear_pending()));
	connect(ui->act_commit_pending_changes, SIGNAL(triggered()), m_model, SLOT(commit_pending()));

	connect(ui->stv, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(draw_grid_context_menu(const QPoint &)));

	connect(ui->list_custom_professions, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(edit_custom_profession(QListWidgetItem*)));
	connect(ui->list_custom_professions, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(draw_custom_profession_context_menu(const QPoint &)));

	connect(m_options_menu, SIGNAL(color_changed(const QString &, const QColor &)), this, SLOT(color_changed(const QString &, const QColor &)));

	m_settings = new QSettings(QSettings::IniFormat, QSettings::UserScope, COMPANY, PRODUCT, this);

	m_lbl_status = new QLabel(tr("not connected"), statusBar());
	statusBar()->addPermanentWidget(m_lbl_status, 0);
    set_interface_enabled(false);

	ui->cb_group_by->setItemData(0, DwarfModel::GB_NOTHING);
	ui->cb_group_by->addItem("Profession", DwarfModel::GB_PROFESSION);
	ui->cb_group_by->addItem("Legendary or not", DwarfModel::GB_LEGENDARY);
	read_settings();

	check_latest_version();
	//TODO: make this an option to connect on launch
	connect_to_df();
	if (m_df && m_df->is_ok())
		read_dwarves();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::read_settings() {
	LOGD << "beginning to read settings";
	m_reading_settings = true; // don't allow writes while we're reading...
	m_settings->beginGroup("window");
	{ // WINDOW SETTINGS
		QByteArray geom = m_settings->value("geometry").toByteArray();
		if (!geom.isEmpty()) {
			restoreGeometry(geom);
		}
		// Toolbars etc... 
		QByteArray state = m_settings->value("state").toByteArray();
		if (!state.isEmpty()) {
			restoreState(state);
		}
	}
	m_settings->endGroup();
	
	m_settings->beginGroup("gui_options");
	{ // GUI OPTIONS
		bool enabled;
		enabled = m_settings->value("show_toolbutton_text", true).toBool();
		ui->act_show_toolbutton_text->setChecked(enabled);
		show_toolbutton_text(enabled);

		enabled = m_settings->value("single_click_labor_changes", false).toBool();
		ui->act_single_click_labor_changes->setChecked(enabled);
		set_single_click_labor_changes(enabled);

		int group_by = m_settings->value("group_by", 0).toInt();
		ui->cb_group_by->setCurrentIndex(group_by);
		m_model->set_group_by(group_by);
	}
	m_settings->endGroup();

	m_settings->beginGroup("custom_professions");
	{
		QStringList profession_names = m_settings->childGroups();
		foreach(QString prof, profession_names) {
			CustomProfession *cp = new CustomProfession(this);
			cp->set_name(prof);
			m_settings->beginGroup(prof);
			int size = m_settings->beginReadArray("labors");
			for(int i = 0; i < size; ++i) {
				m_settings->setArrayIndex(i);
				int labor_id = m_settings->childKeys()[0].toInt();
				cp->add_labor(labor_id);
			}
			m_settings->endArray();
			m_settings->endGroup();
			m_custom_professions << cp;
		}
	}
	m_settings->endGroup();
	
	// options menu settings
	m_options_menu->read_settings(m_settings);

	m_reading_settings = false;
	LOGD << "finished reading settings";
	draw_professions();	
}

void MainWindow::write_settings() {
	if (m_settings && !m_reading_settings) {
		LOGD << "beginning to write settings";
		QByteArray geom = saveGeometry();
		QByteArray state = saveState();
		m_settings->beginGroup("window");
		m_settings->setValue("geometry", QVariant(geom));
		m_settings->setValue("state", QVariant(state));
		m_settings->endGroup();
		m_settings->beginGroup("gui_options");
		m_settings->setValue("show_toolbutton_text", ui->act_show_toolbutton_text->isChecked());
		m_settings->setValue("single_click_labor_changes", ui->act_single_click_labor_changes->isChecked());
		m_settings->setValue("group_by", m_model->current_grouping());
		m_settings->endGroup();

		// options menu settings
		m_options_menu->write_settings(m_settings);
		
		if (m_custom_professions.size() > 0) {
			m_settings->beginGroup("custom_professions");
			m_settings->remove(""); // clear all of them, so we can re-write

			foreach(CustomProfession *cp, m_custom_professions) {
				m_settings->beginGroup(cp->get_name());
				m_settings->beginWriteArray("labors");
				int i = 0;
				foreach(int labor_id, cp->get_enabled_labors()) {
					m_settings->setArrayIndex(i++);
					m_settings->setValue(QString::number(labor_id), true);
				}
				m_settings->endArray();
				m_settings->endGroup();
			}
			m_settings->endGroup();
			
		}
		LOGD << "finished writing settings";
	}
}

void MainWindow::closeEvent(QCloseEvent *evt) {
	LOG->info() << "Beginning shutdown";
	write_settings();
	evt->accept();
	LOG->info() << "Closing Dwarf Therapist normally";
}

void MainWindow::connect_to_df() {
	LOGD << "attempting connection to running DF game";
    if (m_df) {
		LOGD << "already connected, disconnecting";
        delete m_df;
        set_interface_enabled(false);
        m_df = 0;
    }
	// find_running_copy can fail for several reasons, and will take care of 
	// logging and notifying the user.
    m_df = DFInstance::find_running_copy(this);
	if (m_df && m_df->is_ok()) {
		m_lbl_status->setText(tr("Connected to ") + m_df->memory_layout()->game_version());
		set_interface_enabled(true);
	}
}

void MainWindow::read_dwarves() {
	m_model->set_instance(m_df);
	m_model->load_dwarves();
	m_proxy->setSourceModel(m_model);
	ui->stv->sortByColumn(0, Qt::AscendingOrder);
}

void MainWindow::set_interface_enabled(bool enabled) {
	ui->act_connect_to_DF->setEnabled(!enabled);
	ui->act_read_dwarves->setEnabled(enabled);
	ui->act_scan_memory->setEnabled(enabled);
	ui->act_expand_all->setEnabled(enabled);
	ui->act_collapse_all->setEnabled(enabled);
	ui->cb_group_by->setEnabled(enabled);
	ui->btn_import_professions->setEnabled(enabled);
	ui->act_import_existing_professions->setEnabled(enabled);
}

void MainWindow::check_latest_version() {
	//http://code.google.com/p/dwarftherapist/wiki/LatestVersion
	Version our_v(VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);

	QHttpRequestHeader header("GET", "/p/dwarftherapist/wiki/LatestVersion");
	header.setValue("Host", "code.google.com");
	header.setValue("User-Agent", QString("DwarfTherapist %1").arg(our_v.to_string()));
	header.setValue("Referer", QString("/DwarfTherapist %1").arg(our_v.to_string()));
	m_http->setHost("code.google.com");
	disconnect(m_http, SIGNAL(done(bool)));
	connect(m_http, SIGNAL(done(bool)), this, SLOT(version_check_finished(bool)));
	m_http->request(header);
}

void MainWindow::version_check_finished(bool error) {
	if (error) {
		qWarning() << m_http->errorString();
	}
	QString data = QString(m_http->readAll());
	QRegExp rx("###LATEST_VERSION###(\\d+)\\.(\\d+)\\.(\\d+)###END_LATEST_VERSION###");
	int pos = rx.indexIn(data);
	if (pos != -1) {
		Version our_v(VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
		QString major = rx.cap(1);
		QString minor = rx.cap(2);
		QString patch = rx.cap(3);
		Version newest_v(major.toInt(), minor.toInt(), patch.toInt());
		qDebug() << "OUR VERSION:" << our_v.to_string();
		qDebug() << "LATEST VERSION:" << newest_v.to_string();
		if (our_v < newest_v) {
			qDebug() << "LATEST VERSION IS NEWER!";
			QMessageBox *mb = new QMessageBox(this);
			mb->setWindowTitle(tr("Update Available"));
			mb->setText(tr("A newer version of this application is available."));
			QString link = "<a href=\"http://code.google.com/p/dwarftherapist/downloads/list\">Download v" + newest_v.to_string() + "</a>";
			mb->setInformativeText(QString("You are currently running v%1. %2").arg(our_v.to_string()).arg(link));
			mb->exec();
		}
		m_about_dialog->set_latest_version(newest_v);
	} else {
		m_about_dialog->version_check_failed();
		
	}
}

void MainWindow::scan_memory() {
    QProgressDialog *pd = new QProgressDialog(tr("Scanning Memory"), tr("Cancel"), 0, 1, this);
    connect(m_df, SIGNAL(scan_total_steps(int)), pd, SLOT(setMaximum(int)));
    connect(m_df, SIGNAL(scan_progress(int)), pd, SLOT(setValue(int)));
    connect(m_df, SIGNAL(scan_message(QString)), pd, SLOT(setLabelText(QString)));
    connect(pd, SIGNAL(canceled()), m_df, SLOT(cancel_scan()));
    pd->show();

    //int language_addr = m_df->find_language_vector();
    //int translation_addr = m_df->find_translation_vector();
    int creature_addr = m_df->find_creature_vector();
	pd->deleteLater();

    //qDebug() << "LANGUAGE VECTOR:   " << hex << language_addr;
    //qDebug() << "TRANSLATION VECTOR:" << hex << translation_addr;
    qDebug() << "CREATURE VECTOR:   " << hex << creature_addr;
}

void MainWindow::show_toolbutton_text(bool enabled) {
	if (enabled)
		ui->main_toolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	else
		ui->main_toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
	write_settings();
}

void MainWindow::set_single_click_labor_changes(bool enabled) {
	ui->stv->set_single_click_labor_changes(enabled);
	write_settings();
}

void MainWindow::set_group_by(int group_by) {
	m_model->set_group_by(group_by);
	write_settings();
}

void MainWindow::show_about() {
	m_about_dialog->show();
}

void MainWindow::new_pending_changes(int cnt) {
	bool on_off = cnt > 0;
	ui->lbl_pending_changes->setNum(cnt);
	ui->btn_clear->setEnabled(on_off);
	ui->btn_commit->setEnabled(on_off);
	ui->act_clear_pending_changes->setEnabled(on_off);
	ui->act_commit_pending_changes->setEnabled(on_off);
	list_pending();
}

void MainWindow::list_pending() {
	ui->tree_pending->clear();
	foreach(Dwarf *d, m_model->get_dirty_dwarves()) {
		QTreeWidgetItem *i = d->get_pending_changes_tree();
		ui->tree_pending->addTopLevelItem(i);
	}
	ui->tree_pending->expandAll();
}

void MainWindow::open_options_menu() {
	m_options_menu->show();
}

void MainWindow::color_changed(const QString &key, const QColor &c) {
	UberDelegate *d = ui->stv->get_delegate();
	if (key == "cursor")
		d->color_cursor = c;
	else if (key == "dirty_border")
		d->color_dirty_border = c;
	else if (key == "active_labor")
		d->color_active_labor = c;
	else if (key == "active_group")
		d->color_active_group = c;
	else if (key == "inactive_group")
		d->color_inactive_group = c;
	else if (key == "partial_group")
		d->color_partial_group = c;
	else if (key == "guides")
		d->color_guides = c;
	else if (key == "border")
		d->color_border = c;
	else
		qWarning() << "some color changed and I don't know what it is.";
}

void MainWindow::add_custom_profession() {
	Dwarf *d = 0;
	QModelIndex idx = ui->stv->currentIndex();
	if (idx.isValid()) {
		int id = idx.data(DwarfModel::DR_ID).toInt();
		d = m_model->get_dwarf_by_id(id);
	}

	CustomProfession *cp = new CustomProfession(d, this);
	int accepted = cp->show_builder_dialog(this);
	if (accepted) {
		m_custom_professions << cp;
		draw_professions();
	}
	write_settings();
}

void MainWindow::reset_custom_profession() {
	const QItemSelection sel = ui->stv->selectionModel()->selection();
	foreach(const QModelIndex idx, sel.indexes()) {
		if (idx.column() == 0 && !idx.data(DwarfModel::DR_IS_AGGREGATE).toBool()) {
			Dwarf *d = m_model->get_dwarf_by_id(idx.data(DwarfModel::DR_ID).toInt());
			if (d)
				d->reset_custom_profession();
		}
	}
	m_model->calculate_pending();
}

void MainWindow::edit_custom_profession() {
	if (!m_temp_cp)
		return;

	int accepted = m_temp_cp->show_builder_dialog(this);
	if (accepted) {
		draw_professions();
	}
	m_temp_cp = 0;
	write_settings();
}

void MainWindow::edit_custom_profession(QListWidgetItem *i) {
	QString name = i->text();
	foreach(CustomProfession *cp, m_custom_professions) {
		if (cp->get_name() == name) {
			m_temp_cp = cp;
		}
	}
	edit_custom_profession();
}

void MainWindow::delete_custom_profession() {
	if (!m_temp_cp)
		return;

	QList<Dwarf*> blockers;
	foreach(Dwarf *d, m_model->get_dwarves()) {
		if (d->profession() == m_temp_cp->get_name()) {
			blockers << d;
		}
	}
	if (blockers.size() > 0) {
		QMessageBox *box = new QMessageBox(this);
		box->setIcon(QMessageBox::Warning);
		box->setWindowTitle(tr("Cannot Remove Profession"));
		box->setText(tr("The following %1 dwarf(s) is(are) still using <b>%2</b>. Please change them to"
			" another profession before deleting this profession!").arg(blockers.size()).arg(m_temp_cp->get_name()));
		QString msg = tr("Dwarves with this profession:\n\n");
		foreach(Dwarf *d, blockers) {
			msg += d->nice_name() + "\n";
		}
		box->setDetailedText(msg);
		box->exec();
	} else {
		m_temp_cp->delete_from_disk();
		m_custom_professions.remove(m_custom_professions.indexOf(m_temp_cp));
	}
	draw_professions();
	m_temp_cp = 0;
	write_settings();
}

void MainWindow::draw_professions() {
	ui->list_custom_professions->clear();
	foreach(CustomProfession *cp, m_custom_professions) {
		new QListWidgetItem(cp->get_name(), ui->list_custom_professions);
	}
}

void MainWindow::draw_grid_context_menu(const QPoint &p) {
	QModelIndex idx = ui->stv->indexAt(p);
	if (!idx.isValid() || idx.column() != 0  || idx.data(DwarfModel::DR_IS_AGGREGATE).toBool())
		return;

	//int id = idx.data(DwarfModel::DR_ID).toInt();
	
	QMenu m(this);
	m.setTitle(tr("Dwarf Options"));
	m.addAction(tr("Set Nickname..."), this, SLOT(set_nickname()));
	//m.addAction(tr("View Details..."), this, "add_custom_profession()");
	m.addSeparator();

	QMenu sub(&m);
	sub.setTitle(tr("Custom Professions"));
	sub.addAction(tr("New custom profession from this dwarf..."), this, SLOT(add_custom_profession()));
	sub.addAction(tr("Reset to default profession"), this, SLOT(reset_custom_profession()));
	sub.addSeparator();
	
	foreach(CustomProfession *cp, m_custom_professions) {
		sub.addAction(cp->get_name(), this, SLOT(apply_custom_profession()));
	}
	m.addMenu(&sub);
	
	m.exec(ui->stv->viewport()->mapToGlobal(p));
}

void MainWindow::draw_custom_profession_context_menu(const QPoint &p) {
	QModelIndex idx = ui->list_custom_professions->indexAt(p);
	if (!idx.isValid())
		return;

	foreach(CustomProfession *cp, m_custom_professions) {
		if (cp && cp->get_name() == idx.data().toString())
			m_temp_cp = cp;
	}

	QMenu m(this);
	m.setTitle(tr("Custom Profession"));
	m.addAction(tr("Edit..."), this, SLOT(edit_custom_profession()));
	m.addAction(tr("Delete..."), this, SLOT(delete_custom_profession()));
	m.exec(ui->list_custom_professions->viewport()->mapToGlobal(p));
}

void MainWindow::apply_custom_profession() {
	QAction *a = qobject_cast<QAction*>(QObject::sender());
	CustomProfession *cp = get_custom_profession(a->text());
	if (!cp)
		return;

	const QItemSelection sel = ui->stv->selectionModel()->selection();
	foreach(const QModelIndex idx, sel.indexes()) {
		if (idx.column() == 0 && !idx.data(DwarfModel::DR_IS_AGGREGATE).toBool()) {
			qDebug() << idx.data();
			Dwarf *d = m_model->get_dwarf_by_id(idx.data(DwarfModel::DR_ID).toInt());
			if (d)
				d->apply_custom_profession(cp);
		}
	}
	m_model->calculate_pending();
}

CustomProfession *MainWindow::get_custom_profession(QString name) {
	CustomProfession *retval = 0;
	foreach(CustomProfession *cp, m_custom_professions) {
		if (cp && cp->get_name() == name) {
			retval = cp;
			break;
		}
	}
	return retval;
}

void MainWindow::import_existing_professions() {
	int imported = 0;
	foreach(Dwarf *d, m_model->get_dwarves()) {
		QString prof = d->custom_profession_name();
		if (prof.isEmpty())
			continue;
		CustomProfession *cp = get_custom_profession(prof);
		if (!cp) { // import it
			cp = new CustomProfession(d, this);
			cp->set_name(prof);
			m_custom_professions << cp;
			imported++;
		}
	}
	draw_professions();
	QMessageBox::information(this, tr("Import Successful"), 
		tr("Imported %n custom profession(s)", "" ,imported));
}

void MainWindow::set_nickname() {
	const QItemSelection sel = ui->stv->selectionModel()->selection();
	QModelIndexList first_col;
	foreach(QModelIndex i, sel.indexes()) {
		if (i.column() == 0 && !i.data(DwarfModel::DR_IS_AGGREGATE).toBool())
			first_col << i;
	}

	if (first_col.size() != 1) {
		QMessageBox::warning(this, tr("Too many!"), tr("Slow down, killer. One at a time."));
		return;
	}
	
	int id = first_col[0].data(DwarfModel::DR_ID).toInt();
	Dwarf *d = m_model->get_dwarf_by_id(id);
	if (d) {
		QString new_nick = QInputDialog::getText(this, tr("New Nickname"), tr("Nickname"), QLineEdit::Normal, d->nickname());
		if (new_nick.length() > 28) {
			QMessageBox::warning(this, tr("Nickname too long"), tr("Nicknames must be under 28 characters long."));
			return;
		}
		d->set_nickname(new_nick);
		m_model->setData(first_col[0], d->nice_name(), Qt::DisplayRole);
		//m_model->dataChanged(first_col[0], first_col[0]);
	}
	m_model->calculate_pending();
}

// web addresses
void MainWindow::go_to_forums() {
	QDesktopServices::openUrl(QUrl("http://udpviper.com/forums/viewforum.php?f=36"));
}
void MainWindow::go_to_donate() {
	QDesktopServices::openUrl(QUrl("http://code.google.com/p/dwarftherapist/wiki/Donations"));
}
void MainWindow::go_to_project_home() {
	QDesktopServices::openUrl(QUrl("http://code.google.com/p/dwarftherapist"));
}
void MainWindow::go_to_new_issue() {
	QDesktopServices::openUrl(QUrl("http://code.google.com/p/dwarftherapist/issues/entry"));
}
