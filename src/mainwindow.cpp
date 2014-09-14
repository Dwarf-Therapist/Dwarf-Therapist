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
#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>
#include <QtDebug>
#include <QShortcut>

#include "mainwindow.h"
#include "ui_about.h"
#include "ui_mainwindow.h"
#include "ui_pendingchanges.h"
#include "aboutdialog.h"
#include "dwarf.h"
#include "dwarfmodel.h"
#include "dwarfmodelproxy.h"
#include "memorylayout.h"
#include "viewmanager.h"
#include "viewcolumnset.h"
#include "uberdelegate.h"
#include "customprofession.h"
#include "superlabor.h"
#include "labor.h"
#include "defines.h"
#include "version.h"
#include "dwarftherapist.h"
#include "importexportdialog.h"
#include "gridviewdock.h"
#include "skilllegenddock.h"
#include "dwarfdetailsdock.h"
#include "columntypes.h"
#include "rotatedheader.h"
#include "scanner.h"
#include "scriptdialog.h"
#include "truncatingfilelogger.h"
#include "roledialog.h"
#include "viewcolumn.h"
#include "rolecolumn.h"
#include "statetableview.h"
#include "preferencesdock.h"
#include "laboroptimizer.h"
#include "laboroptimizerplan.h"
#include "optimizereditor.h"
#include "gamedatareader.h"
#include "thoughtsdock.h"
#include "fortressentity.h"
#include "preference.h"
#include "healthlegenddock.h"
#include "dfinstance.h"
#include "squad.h"

#include "eventfilterlineedit.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_df(0)
    , m_lbl_status(new QLabel(tr("Disconnected"), this))
    , m_lbl_message(new QLabel(tr("Initializing"), this))
    , m_progress(new QProgressBar(this))
    , m_settings(0)
    , m_view_manager(0)
    , m_model(new DwarfModel(this))
    , m_proxy(new DwarfModelProxy(this))
    , m_about_dialog(new AboutDialog(this))
    , m_scanner(0)
    , m_script_dialog(0)
    , m_role_editor(0)
    , m_optimize_plan_editor(0)
    , m_reading_settings(false)
    , m_show_result_on_equal(false)
    , m_dwarf_name_completer(0)
    , m_force_connect(false)
    , m_try_download(true)
    , m_deleting_settings(false)
    , m_act_sep_optimize(0)
    , m_btn_optimize(0)
{
    ui->setupUi(this);

    //connect to df first, we need to read raws for some ui elements first!!!
    //connect_to_df();

    m_view_manager = new ViewManager(m_model, m_proxy, this);
    ui->v_box->addWidget(m_view_manager);
    setCentralWidget(ui->main_widget);

    setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);

    /* docks! */
    GridViewDock *grid_view_dock = new GridViewDock(m_view_manager, this);
    grid_view_dock->setHidden(true); // hide by default
    grid_view_dock->setFloating(false);
    addDockWidget(Qt::RightDockWidgetArea, grid_view_dock);

    SkillLegendDock *skill_legend_dock = new SkillLegendDock(this);
    skill_legend_dock->setHidden(true); // hide by default
    skill_legend_dock->setFloating(false);
    addDockWidget(Qt::RightDockWidgetArea, skill_legend_dock);

    DwarfDetailsDock *dwarf_details_dock = new DwarfDetailsDock(this);
    dwarf_details_dock->setHidden(true);
    dwarf_details_dock->setFloating(false);
    addDockWidget(Qt::RightDockWidgetArea, dwarf_details_dock);

    PreferencesDock *pref_dock = new PreferencesDock(this);
    pref_dock->setHidden(true);
    pref_dock->setFloating(false);
    addDockWidget(Qt::RightDockWidgetArea, pref_dock);

    ThoughtsDock *thought_dock = new ThoughtsDock(this);
    thought_dock->setHidden(true);
    thought_dock->setFloating(false);
    addDockWidget(Qt::RightDockWidgetArea, thought_dock);

    HealthLegendDock *health_dock = new HealthLegendDock(this);
    health_dock->setHidden(true);
    health_dock->setFloating(false);
    addDockWidget(Qt::RightDockWidgetArea, health_dock);

    ui->menu_docks->addAction(ui->dock_pending_jobs_list->toggleViewAction());
    ui->menu_docks->addAction(ui->dock_custom_professions->toggleViewAction());
    ui->menu_docks->addAction(grid_view_dock->toggleViewAction());
    ui->menu_docks->addAction(skill_legend_dock->toggleViewAction());
    ui->menu_docks->addAction(dwarf_details_dock->toggleViewAction());
    ui->menu_docks->addAction(pref_dock->toggleViewAction());
    ui->menu_docks->addAction(thought_dock->toggleViewAction());
    ui->menu_docks->addAction(health_dock->toggleViewAction());

    ui->menuWindows->addAction(ui->main_toolbar->toggleViewAction());

    LOGD << "setting up connections for MainWindow";
    connect(ui->main_toolbar, SIGNAL(toolButtonStyleChanged(Qt::ToolButtonStyle)),this, SLOT(main_toolbar_style_changed(Qt::ToolButtonStyle)));

    connect(m_model, SIGNAL(new_creatures_count(int,int,int, QString)), this, SLOT(new_creatures_count(int,int,int, QString)));
    connect(m_model, SIGNAL(new_pending_changes(int)), this, SLOT(new_pending_changes(int)));
    connect(ui->act_clear_pending_changes, SIGNAL(triggered()), m_model, SLOT(clear_pending()));
    connect(ui->act_commit_pending_changes, SIGNAL(triggered()), m_model, SLOT(commit_pending()));
    connect(ui->act_expand_all, SIGNAL(triggered()), m_view_manager, SLOT(expand_all()));
    connect(ui->act_collapse_all, SIGNAL(triggered()), m_view_manager, SLOT(collapse_all()));
    connect(ui->act_add_new_gridview, SIGNAL(triggered()), grid_view_dock, SLOT(add_new_view()));

    connect(ui->tree_pending, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)),
            m_view_manager, SLOT(jump_to_dwarf(QTreeWidgetItem *, QTreeWidgetItem *)));

    connect(ui->tree_custom_professions, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(draw_custom_profession_context_menu(QPoint)));
    connect(ui->tree_custom_professions,SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
            m_view_manager, SLOT(jump_to_profession(QTreeWidgetItem*,QTreeWidgetItem*)));

    connect(m_view_manager, SIGNAL(dwarf_focus_changed(Dwarf*)), dwarf_details_dock, SLOT(show_dwarf(Dwarf*)));
    connect(m_view_manager, SIGNAL(selection_changed()), this, SLOT(toggle_opts_menu()));
    connect(ui->cb_filter_script, SIGNAL(currentIndexChanged(const QString &)), SLOT(new_filter_script_chosen(const QString &)));

    connect(m_script_dialog, SIGNAL(test_script(QString)),m_proxy,SLOT(test_script(QString)));
    connect(m_script_dialog, SIGNAL(scripts_changed()), SLOT(reload_filter_scripts()));
    connect(m_script_dialog, SIGNAL(accepted()),m_proxy,SLOT(clear_test()));
    connect(m_script_dialog, SIGNAL(rejected()), m_proxy, SLOT(clear_test()));

    connect(m_view_manager,SIGNAL(group_changed(int)), this, SLOT(display_group(int)));

    connect(pref_dock,SIGNAL(item_selected(QList<QPair<QString,QString> >)),this,SLOT(preference_selected(QList<QPair<QString,QString> >)));
    connect(thought_dock, SIGNAL(item_selected(QList<short>)), this, SLOT(thought_selected(QList<short>)));
    connect(health_dock, SIGNAL(item_selected(QList<QPair<int,int> >)), this, SLOT(health_legend_selected(QList<QPair<int,int> >)));

    connect(ui->btn_clear_filters, SIGNAL(clicked()),this,SLOT(clear_all_filters()));

    connect(m_proxy, SIGNAL(filter_changed()),this,SLOT(refresh_active_scripts()));

    //the model will perform the unit reads, and emit a signal, which we pass to the DT object
    //DT will perform any other changes/updates required, and then pass on the signal, which will be used to update data (ie. superlabor columns)
    connect(m_model,SIGNAL(units_refreshed()),DT,SLOT(emit_units_refreshed()));
    //DT then emits a second signal, ensuring that any updates to data is done before signalling other objects
    //the view manager is signalled this way, to ensure that the order of signals is preserved
    connect(DT,SIGNAL(connected()),m_view_manager,SLOT(rebuild_global_sort_keys()));

    m_settings = new QSettings(QSettings::IniFormat, QSettings::UserScope, COMPANY, PRODUCT, this);

    m_progress->setVisible(false);
    statusBar()->addPermanentWidget(m_lbl_message, 0);
    statusBar()->addPermanentWidget(m_lbl_status, 0);
    set_interface_enabled(false);

    ui->cb_group_by->setItemData(0, DwarfModel::GB_NOTHING);
    ui->cb_group_by->addItem(tr("Age"), DwarfModel::GB_AGE);
    ui->cb_group_by->addItem(tr("Caste"), DwarfModel::GB_CASTE);
    ui->cb_group_by->addItem(tr("Current Job"), DwarfModel::GB_CURRENT_JOB);
    ui->cb_group_by->addItem(tr("Gender & Orientation"), DwarfModel::GB_SEX);
    ui->cb_group_by->addItem(tr("Goals"), DwarfModel::GB_GOALS);
    ui->cb_group_by->addItem(tr("Happiness"), DwarfModel::GB_HAPPINESS);
    ui->cb_group_by->addItem(tr("Has Nickname"),DwarfModel::GB_HAS_NICKNAME);
    ui->cb_group_by->addItem(tr("Health"),DwarfModel::GB_HEALTH);
    ui->cb_group_by->addItem(tr("Highest Moodable Skill"), DwarfModel::GB_HIGHEST_MOODABLE);
    ui->cb_group_by->addItem(tr("Highest Skill"), DwarfModel::GB_HIGHEST_SKILL);
    ui->cb_group_by->addItem(tr("Legendary Status"), DwarfModel::GB_LEGENDARY);
    ui->cb_group_by->addItem(tr("Migration Wave"),DwarfModel::GB_MIGRATION_WAVE);
    ui->cb_group_by->addItem(tr("Military Status"),DwarfModel::GB_MILITARY_STATUS);
    ui->cb_group_by->addItem(tr("Profession"), DwarfModel::GB_PROFESSION);
    ui->cb_group_by->addItem(tr("Race"), DwarfModel::GB_RACE);
    ui->cb_group_by->addItem(tr("Skill Rust"), DwarfModel::GB_SKILL_RUST);
    ui->cb_group_by->addItem(tr("Squad"), DwarfModel::GB_SQUAD);
    ui->cb_group_by->addItem(tr("Total Assigned Labors"),DwarfModel::GB_ASSIGNED_LABORS);
    ui->cb_group_by->addItem(tr("Total Assigned Skilled Labors"),DwarfModel::GB_ASSIGNED_SKILLED_LABORS);
    ui->cb_group_by->addItem(tr("Total Skill Levels"),DwarfModel::GB_TOTAL_SKILL_LEVELS);

    read_settings();
    load_customizations();
    reload_filter_scripts();
    refresh_role_menus();
    refresh_opts_menus();

    if (m_settings->value("options/check_for_updates_on_startup", true).toBool())
        check_latest_version();


    //add CTRL+A to select all currently filtered/visible dwarves
    new QShortcut(Qt::CTRL + Qt::Key_A, m_view_manager, SLOT(select_all()));
    raise();
}

MainWindow::~MainWindow() {
    delete m_lbl_status;
    delete m_lbl_message;
    delete m_progress;

    delete m_about_dialog;
    delete m_scanner;
    delete m_script_dialog;
    delete m_role_editor;
    delete m_optimize_plan_editor;
    delete m_dwarf_name_completer;

    delete m_model;
    delete m_view_manager;
    delete m_proxy;
    delete m_df;
    delete m_settings;
    delete ui;
}


void MainWindow::read_settings() {
    m_reading_settings = true;
    m_settings->beginGroup("window");
    { // WINDOW SETTINGS
        try{
            //restore window size/position
            QByteArray geom = m_settings->value("geometry").toByteArray();
            if (!geom.isEmpty()) {
                restoreGeometry(geom);
            }
            //restore toolbars, docks, etc..
            QByteArray state = m_settings->value("state").toByteArray();
            if (!state.isEmpty()) {
                restoreState(state);
            }

        }catch(...){
            //this can sometimes crash, no idea why
        }
    }
    m_settings->endGroup();
    m_reading_settings = false;

}

void MainWindow::write_settings() {
    if (m_settings && !m_reading_settings) {
        LOGI << "beginning to write settings";
        QByteArray geom = saveGeometry();
        QByteArray state = saveState();
        m_settings->beginGroup("window");
        m_settings->setValue("geometry", QVariant(geom));
        m_settings->setValue("state", QVariant(state));
        m_settings->endGroup();
        m_settings->beginGroup("gui_options");
        m_settings->setValue("group_by", m_model->current_grouping());

        DwarfDetailsDock *dock = qobject_cast<DwarfDetailsDock*>(QObject::findChild<DwarfDetailsDock*>("dock_dwarf_details"));
        if(dock){
            QByteArray sizes = dock->splitter_sizes();
            if(!sizes.isNull())
                m_settings->setValue("detailPanesSizes", sizes);
        }
        m_settings->endGroup();

        LOGI << "finished writing settings";
    }
}

void MainWindow::closeEvent(QCloseEvent *evt) {
    LOGI << "Beginning shutdown";
    if(!m_deleting_settings) {
        write_settings();
        m_view_manager->write_views();
    }
    evt->accept();
    LOGI << "Closing Dwarf Therapist normally";
}

void MainWindow::connect_to_df() {
    LOGI << "attempting connection to running DF game";
    if (m_df) {
        LOGI << "already connected, disconnecting";
        delete m_df;
        set_interface_enabled(false);
        m_df = 0;
        reset();
    }

    m_df = DFInstance::newInstance();
    GameDataReader::ptr()->refresh_facets();

    // find_running_copy can fail for several reasons, and will take care of
    // logging and notifying the user.
    if (m_force_connect && m_df && m_df->find_running_copy(true)) {
        m_scanner = new Scanner(m_df, this);

        if(m_df->memory_layout()){
            LOGI << "Connection to DF version" << m_df->memory_layout()->game_version() << "established.";
            set_status_message(tr("Connected to DF %1").arg(m_df->memory_layout()->game_version()),tr("Currently using layout file: %1").arg(m_df->memory_layout()->filename()));
        }else{
            LOGI << "Connection to unknown DF Version established.";
            set_status_message("Connected to unknown version!","");
        }
        m_force_connect = false;
    } else if (m_df && m_df->find_running_copy() && m_df->is_ok()) {
        m_scanner = new Scanner(m_df, this);
        LOGI << "Connection to DF version" << m_df->memory_layout()->game_version() << "established.";
        set_status_message(tr("Connected to DF %1").arg(m_df->memory_layout()->game_version()),tr("Currently using layout file: %1").arg(m_df->memory_layout()->filename()));
        m_force_connect = false;
    } else {
        m_force_connect = true;
    }

    if(m_df && m_df->is_ok()){
        set_interface_enabled(true);
        connect(m_df, SIGNAL(progress_message(QString)), SLOT(set_progress_message(QString)), Qt::UniqueConnection);
        connect(m_df, SIGNAL(progress_range(int,int)), SLOT(set_progress_range(int,int)), Qt::UniqueConnection);
        connect(m_df, SIGNAL(progress_value(int)), SLOT(set_progress_value(int)), Qt::UniqueConnection);

        if (m_df->memory_layout() && m_df->memory_layout()->is_complete()) {
            connect(m_df, SIGNAL(connection_interrupted()), SLOT(lost_df_connection()));

            m_df->load_game_data();
            if(m_view_manager){
                m_view_manager->reload_views();
                m_view_manager->draw_views();

                GridViewDock *dock = qobject_cast<GridViewDock*>(QObject::findChild<GridViewDock*>("GridViewDock"));
                if(dock)
                    dock->draw_views();
            }
            if (DT->user_settings()->value("options/read_on_startup", true).toBool()) {
                read_dwarves();
            }
        }
    }
}

void MainWindow::set_status_message(QString msg, QString tooltip_msg){
    m_lbl_status->setText(tr("%1 - DT Version %2").arg(msg).arg(Version().to_string()));
    m_lbl_status->setToolTip(tr("<span>%1</span>").arg(tooltip_msg));
}

void MainWindow::lost_df_connection() {
    LOGW << "lost connection to DF";
    if (m_df) {
        m_model->clear_all(true);
        m_df->disconnect();
        delete m_df;
        m_df = 0;
        reset();
        set_interface_enabled(false);
        QString details = tr("Dwarf Fortress has either stopped running, or you unloaded your game. Please re-connect when a fort is loaded.");
        set_status_message(tr("Disconnected"), details);
        QMessageBox::information(this, tr("Unable to talk to Dwarf Fortress"), details);
    }
}

void MainWindow::read_dwarves() {
    if (!m_df || !m_df->is_ok()) {
        lost_df_connection();
        return;
    }

    QTime t;
    t.start();
    //clear the selected dwarf's details, save the id of the one we're showing
    int dock_id = -1;
    DwarfDetailsDock *dock = qobject_cast<DwarfDetailsDock*>(QObject::findChild<DwarfDetailsDock*>("dock_dwarf_details"));
    if(dock){
        dock_id = dock->current_id();
        dock->clear(false);
    }
    //save the ids of the currently selected dwarfs
    QVector<int> ids;
    foreach(Dwarf *d, m_view_manager->get_selected_dwarfs()){
        ids << d->id();
    }
    //clear selected dwarfs in the view
    if(m_view_manager)
        m_view_manager->clear_selected();

    //clear data in each column for each view
    foreach(GridView *gv, m_view_manager->views()){
        foreach(ViewColumnSet *set, gv->sets()) {
            foreach(ViewColumn *col, set->columns()) {
                col->clear_cells();
            }
        }
    }
    m_model->clear_all(false);

    m_model->set_instance(m_df);
    m_df->load_fortress();
    m_df->load_squads(false);
    m_model->load_dwarves();

    set_progress_message("Setting up interface...");

    if (m_model->get_dwarves().size() < 1) {
        lost_df_connection();
        return;
    }

    set_interface_enabled(true);
    new_pending_changes(0);

    m_view_manager->redraw_current_tab();

    //reselect the ids we saved above
    m_view_manager->reselect(ids);

    // setup the filter auto-completer and reselect our dwarf for the details dock
    bool dwarf_found = false;
    QStandardItemModel *filters = new QStandardItemModel(this);
    foreach(Dwarf *d, m_model->get_dwarves()){
        QStandardItem *i = new QStandardItem(d->nice_name());
        i->setData(d->nice_name(),DwarfModel::DR_SPECIAL_FLAG);
        filters->appendRow(i);
        if(dock_id > 0 && !dwarf_found && d->id() == dock_id){
            dock->show_dwarf(d);
            dwarf_found = true;
        }
    }

    QHash<QPair<QString,QString>,DFInstance::pref_stat* > prefs = DT->get_DFInstance()->get_preference_stats();
    QPair<QString,QString> key_pair;
    foreach(key_pair, prefs.uniqueKeys()){
        QStandardItem *i = new QStandardItem(key_pair.second);
        i->setData(key_pair.second,DwarfModel::DR_SPECIAL_FLAG);
        QVariantList data;
        data << key_pair.first << key_pair.second;
        i->setData(SCR_PREF_EXP,Qt::UserRole);
        i->setData(data,Qt::UserRole+1);
        filters->appendRow(i);
    }
    if(!dwarf_found)
        dock->clear(false);

    filters->sort(0,Qt::AscendingOrder);

    if (!m_dwarf_name_completer) {
        m_dwarf_name_completer = new QCompleter(filters,this);
        m_dwarf_name_completer->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
        m_dwarf_name_completer->setCompletionRole(DwarfModel::DR_SPECIAL_FLAG);
        m_dwarf_name_completer->setCaseSensitivity(Qt::CaseInsensitive);
        ui->le_filter_text->setCompleter(m_dwarf_name_completer);
    }

    //apply a filter when an item is clicked with the mouse in the popup list
    connect(m_dwarf_name_completer->popup(),SIGNAL(clicked(QModelIndex)),this,SLOT(apply_filter(QModelIndex)));
    //we need a custom event filter to intercept the enter key and emit our own signal to filter when enter is hit
    EventFilterLineEdit *filter = new EventFilterLineEdit(ui->le_filter_text);
    m_dwarf_name_completer->popup()->installEventFilter(filter);
    connect(filter,SIGNAL(enterPressed(QModelIndex)),this,SLOT(apply_filter(QModelIndex)));

    //refresh preference dock
    PreferencesDock *d_prefs = qobject_cast<PreferencesDock*>(QObject::findChild<PreferencesDock*>("dock_preferences"));
    if(d_prefs){
        d_prefs->refresh();
    }
    //refresh thoughts dock
    ThoughtsDock *d_thoughts = qobject_cast<ThoughtsDock*>(QObject::findChild<ThoughtsDock*>("dock_thoughts"));
    if(d_thoughts){
        d_thoughts->refresh();
    }

    if(!m_role_editor){
        m_role_editor = new roleDialog(m_df, this);
        connect(m_role_editor, SIGNAL(finished(int)), this, SLOT(done_editing_role(int)),Qt::UniqueConnection);
    }
    if(!m_optimize_plan_editor){
        m_optimize_plan_editor = new optimizereditor(this);
        connect(m_optimize_plan_editor, SIGNAL(finished(int)), this, SLOT(done_editing_opt_plan(int)), Qt::UniqueConnection);
    }\
    if(!m_script_dialog){
        m_script_dialog = new ScriptDialog(this);
    }

    if(DT->multiple_castes && ui->cb_group_by->findData(DwarfModel::GB_CASTE_TAG) < 0){
        //special grouping when using multiple castes, insert it after the caste group
        ui->cb_group_by->blockSignals(true);
        int grp_by = ui->cb_group_by->itemData(ui->cb_group_by->currentIndex()).toInt();
        ui->cb_group_by->insertItem(3, QIcon(":img/exclamation-red-frame.png"), tr("Caste Tag"), DwarfModel::GB_CASTE_TAG);
        ui->cb_group_by->setItemData(3, tr("Possible Spoilers! This may show special/hidden castes!"), Qt::ToolTipRole);
        ui->cb_group_by->setCurrentIndex(ui->cb_group_by->findData(grp_by));
        ui->cb_group_by->blockSignals(false);
    }

    this->setWindowTitle(QString("%1 %2").arg(tr("Dwarf Therapist - ")).arg(m_df->fortress_name()));

    LOGI << "completed read in" << t.elapsed() << "ms";
    set_progress_message("");
}

void MainWindow::apply_filter(){
    apply_filter(m_dwarf_name_completer->currentIndex());
}

void MainWindow::apply_filter(QModelIndex idx){
    if(idx.data(Qt::UserRole) == SCR_PREF_EXP){
        QVariantList data = idx.data(Qt::UserRole+1).toList();
        QList<QPair<QString,QString> > prefs;
        QPair<QString,QString> pref_pair = qMakePair(data.at(0).toString(),data.at(1).toString());
        prefs << pref_pair;
        preference_selected(prefs,QString("%1: %2").arg(pref_pair.first).arg(pref_pair.second),SCR_PREF_EXP);
    }
    ui->le_filter_text->clear();
}

void MainWindow::set_interface_enabled(bool enabled) {
    ui->act_connect_to_DF->setEnabled(!enabled);
    ui->act_read_dwarves->setEnabled(enabled);
    //ui->act_scan_memory->setEnabled(enabled);
    ui->act_expand_all->setEnabled(enabled);
    ui->act_collapse_all->setEnabled(enabled);
    ui->cb_group_by->setEnabled(enabled);
    ui->act_import_existing_professions->setEnabled(enabled);
    ui->act_print->setEnabled(enabled);
    if(m_view_manager)
        m_view_manager->setEnabled(enabled);
}

void MainWindow::check_latest_version(bool show_result_on_equal) {
    Q_UNUSED(show_result_on_equal);
    return;
    //TODO: check for updates
}

void MainWindow::version_check_finished(bool error) {
    Q_UNUSED(error);
//    if (error) {
//        qWarning() <<  m_http->errorString();
//    }
//    QString data = QString(m_http->readAll());
//    QRegExp rx("(\\d+)\\.(\\d+)\\.(\\d+)");
//    int pos = rx.indexIn(data);

//    if (pos != -1) {
//        Version our_v(DT_VERSION_MAJOR, DT_VERSION_MINOR, DT_VERSION_PATCH);
//        QString major = rx.cap(1);
//        QString minor = rx.cap(2);
//        QString patch = rx.cap(3);
//        Version newest_v(major.toInt(), minor.toInt(), patch.toInt());
//        LOGI << "RUNNING VERSION         :" << our_v.to_string();
//        LOGI << "LATEST AVAILABLE VERSION:" << newest_v.to_string();
//        if (our_v < newest_v) {
//            LOGI << "LATEST VERSION IS NEWER!";
//            QMessageBox *mb = new QMessageBox(this);
//            mb->setIcon(QMessageBox::Information);
//            mb->setWindowTitle(tr("Update Available"));
//            mb->setText(tr("A newer version of this application is available."));
//            QString link = tr("<br><a href=\"%1\">Click Here to Download v%2"
//                              "</a>")
//                           .arg(URL_DOWNLOAD_LIST)
//                           .arg(newest_v.to_string());
//            mb->setInformativeText(tr("You are currently running v%1. %2")
//                                   .arg(our_v.to_string()).arg(link));
//            mb->exec();
//        } else if (m_show_result_on_equal) {
//            QMessageBox *mb = new QMessageBox(this);
//            mb->setWindowTitle(tr("Up to Date"));
//            mb->setText(tr("You are running the most recent version of Dwarf "
//                           "Therapist."));
//            mb->exec();
//        }
//        m_about_dialog->set_latest_version(newest_v);
//    } else {
//        m_about_dialog->version_check_failed();
//    }
}

void MainWindow::check_for_layout(const QString & checksum) {
    m_try_download = false;

    if(m_try_download &&
            (m_settings->value("options/check_for_updates_on_startup", true).toBool())) {
//        m_try_download = false;

//        LOGI << "Checking for layout for checksum: " << checksum;
//        m_tmp_checksum = checksum;

//        Version our_v(DT_VERSION_MAJOR, DT_VERSION_MINOR, DT_VERSION_PATCH);

//        QString request = QString("/memory_layouts/checksum/%1").arg(checksum);
//        QHttpRequestHeader header("GET", request);
//        header.setValue("Host", "www.dwarftherapist.com");
//        header.setValue("User-Agent", QString("DwarfTherapist %1").arg(our_v.to_string()));
//        if (m_http) {
//            m_http->deleteLater();
//        }
//        m_http = new QHttp(this);
//        m_http->setHost("www.dwarftherapist.com");

//        disconnect(m_http, SIGNAL(done(bool)));
//        connect(m_http, SIGNAL(done(bool)), this, SLOT(layout_check_finished(bool)));
//        m_http->request(header);
    } else if (!m_force_connect) {
        m_df->layout_not_found(checksum);
    }
}

void MainWindow::layout_check_finished(bool error) {
    //int status = m_http->lastResponse().statusCode();
    //LOGD << "Status: " << status;

//    error = error || (status != 200);
//    if(!error) {
//        QTemporaryFile outFile("layout.ini");
//        if (!outFile.open())
//         return;

//    if( !outFile.setPermissions((QFile::Permission)0x666) ) {
//        LOGD << "WARNING: Unable to set permissions for new layout.";
//    }

//        QString fileName = outFile.fileName();
//        QTextStream out(&outFile);
//        out << m_http->readAll();
//        outFile.close();

//        QString version;

//        {
//            QSettings layout(fileName, QSettings::IniFormat);
//            version = layout.value("info/version_name", "").toString();
//        }

//        LOGD << "Found version" << version;

//        if(m_df->add_new_layout(version, outFile)) {
//            QMessageBox *mb = new QMessageBox(this);
//            mb->setIcon(QMessageBox::Information);
//            mb->setWindowTitle(tr("New Memory Layout Added"));
//            mb->setText(tr("A new memory layout has been downloaded for this version of dwarf fortress!"));
//            mb->setInformativeText(tr("New layout for version %1 of Dwarf Fortress.").arg(version));
//            mb->exec();

//            LOGD << "Reconnecting to Dwarf Fortress!";
//            m_force_connect = false;
//            connect_to_df();
//        } else {
//            error = true;
//        }
//    }

    LOGI << "Error: " << error << " Force Connect: " << m_force_connect;

    if(error) {
        m_df->layout_not_found(m_tmp_checksum);
    }
}

void MainWindow::scan_memory() {
    if (m_scanner) {
        m_scanner->show();
    }
}

void MainWindow::set_group_by(int group_by) {
    //if disconnected, don't try to update the view grouping
    if(!m_df)
        return;
    group_by = ui->cb_group_by->itemData(group_by, Qt::UserRole).toInt();
    if(group_by < 0)
        group_by = 0;
    m_view_manager->set_group_by(group_by);
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
    disconnect(ui->tree_pending, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), 0, 0);
    ui->tree_pending->clear();
    foreach(Dwarf *d, m_model->get_dirty_dwarves()) {
        ui->tree_pending->addTopLevelItem(d->get_pending_changes_tree());
    }
    foreach(Squad *s, m_df->squads()){
        if(s->pending_changes())
            ui->tree_pending->addTopLevelItem(s->get_pending_changes_tree());
    }

    ui->tree_pending->expandAll();
    connect(ui->tree_pending, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)),
        m_view_manager, SLOT(jump_to_dwarf(QTreeWidgetItem *, QTreeWidgetItem *)));
}

void MainWindow::new_creatures_count(int adults, int children, int babies, QString race_name) {
    //on tab change recheck the filtered count if there are active scripts
    if(QString::compare(race_name,m_pop_info.race_name,Qt::CaseInsensitive) != 0){
        if(m_proxy->active_scripts())
            m_pop_info.filtered = m_proxy->get_filtered_dwarves().count();
        else
            m_pop_info.filtered = -1;
    }

    m_pop_info.race_name = race_name;
    m_pop_info.adults = adults;
    m_pop_info.children = children;
    m_pop_info.infants = babies;
    refresh_pop_counts();
}

void MainWindow::refresh_pop_counts(){
    ui->lbl_dwarf_total->setText(tr("%1/%2/%3%4").arg(m_pop_info.adults).arg(m_pop_info.children).arg(m_pop_info.infants)
                                 .arg((m_pop_info.filtered >= 0 ? QString(" (%1)").arg(m_pop_info.filtered) : "")));
    ui->lbl_dwarf_total->setToolTip(tr("%1 Adult%2<br/>%3 Child%4<br/>%5 Bab%6<br/>%7 Total Population%8")
                                    .arg(m_pop_info.adults).arg(m_pop_info.adults == 1 ? "" : "s")
                                    .arg(m_pop_info.children).arg(m_pop_info.children == 1 ? "" : "ren")
                                    .arg(m_pop_info.infants).arg(m_pop_info.infants == 1 ? "y" : "ies")
                                    .arg(m_pop_info.adults+m_pop_info.children+m_pop_info.infants)
                                    .arg((m_pop_info.filtered >= 0 ? tr("<h4>Showing %1 due to filters.</h4>").arg(m_pop_info.filtered) : "")));
    ui->lbl_dwarfs->setText(m_pop_info.race_name);
    ui->lbl_filter->setText(tr("Filter %1").arg(m_pop_info.race_name));
    ui->act_read_dwarves->setText(tr("Read %1").arg(m_pop_info.race_name));
    //TODO: update other interface stuff for the race name when using a custom race
}

void MainWindow::load_customizations() {
    ui->tree_custom_professions->blockSignals(true);
    CustomProfession *cp;
    ui->tree_custom_professions->clear();

    ui->tree_custom_professions->setSortingEnabled(false);

    //add custom professions
    QTreeWidgetItem *cps = new QTreeWidgetItem();
    cps->setText(0,"Custom Professions");
    QTreeWidgetItem *i;
    QList<CustomProfession*> profs = DT->get_custom_professions();
    foreach(cp, profs){
        i = new QTreeWidgetItem(cps);
        i->setText(0,cp->get_name());
        i->setIcon(0,QIcon(cp->get_pixmap()));
        i->setData(0,Qt::UserRole,QVariant(cp->get_name()));
        i->setData(0,Qt::UserRole+1,CUSTOM_PROF);
    }
    ui->act_export_custom_professions->setEnabled(profs.size());
    ui->tree_custom_professions->addTopLevelItem(cps);

    //add profession icons
    QTreeWidgetItem *icons = new QTreeWidgetItem();
    icons->setText(0,"Custom Icons");
    icons->setToolTip(0,tr("Right click on a profession icon in the grid to customize the default icon."));
    foreach(int key, DT->get_custom_prof_icons().uniqueKeys()){
        i = new QTreeWidgetItem(icons);
        cp = DT->get_custom_prof_icon(key);
        i->setText(0,cp->get_name());
        i->setIcon(0,QIcon(cp->get_pixmap()));
        i->setData(0,Qt::UserRole,QVariant(cp->prof_id()));
        i->setData(0,Qt::UserRole+1,CUSTOM_ICON);
    }
    ui->tree_custom_professions->addTopLevelItem(icons);

    //add super labors
    QTreeWidgetItem *super_labors = new QTreeWidgetItem();
    super_labors->setText(0,"Super Labor Columns");
    foreach(SuperLabor *sl, DT->get_super_labors()){
            i = new QTreeWidgetItem(super_labors);
            i->setText(0, sl->get_name());
            i->setData(0,Qt::UserRole,QVariant(sl->get_name()));
            i->setData(0,Qt::UserRole+1,CUSTOM_SUPER);
    }
    ui->tree_custom_professions->addTopLevelItem(super_labors);


    ui->tree_custom_professions->expandAll();
    ui->tree_custom_professions->blockSignals(false);

    ui->tree_custom_professions->sortItems(-1,Qt::AscendingOrder);
    ui->tree_custom_professions->setSortingEnabled(true);

    connect(ui->tree_custom_professions, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)),
        m_view_manager, SLOT(jump_to_profession(QTreeWidgetItem *, QTreeWidgetItem *)),Qt::UniqueConnection);

    m_view_manager->refresh_custom_professions();
}

void MainWindow::draw_custom_profession_context_menu(const QPoint &p) {
    QModelIndex idx = ui->tree_custom_professions->indexAt(p);
    if (!idx.isValid() || ui->tree_custom_professions->itemAt(p)->parent() == 0)
        return;

    QVariantList data;
    data << idx.data(Qt::UserRole) << idx.data(Qt::UserRole+1);
    QMenu m(this);
    CUSTOMIZATION_TYPE t = static_cast<CUSTOMIZATION_TYPE>(idx.data(Qt::UserRole+1).toInt());
    if(t != CUSTOM_SUPER){
        m.setTitle(tr("Custom Profession"));
        if(t == CUSTOM_ICON)
            m.setTitle(m.title() + " Icon");
    }else{
        m.setTitle(tr("Super Labor"));
    }

    QAction *a = m.addAction(QIcon(":img/pencil.png"), tr("Edit %1").arg(idx.data().toString()), DT, SLOT(edit_customization()));
    a->setData(data);
    a = m.addAction(QIcon(":img/minus-circle.png"), tr("Delete %1").arg(idx.data().toString()), DT, SLOT(delete_customization()));
    a->setData(data);
    m.exec(ui->tree_custom_professions->viewport()->mapToGlobal(p));
}


// web addresses
void MainWindow::go_to_forums() {
    QDesktopServices::openUrl(QUrl("http://www.bay12forums.com/smf/index.php?topic=122968.0"));
}
void MainWindow::go_to_donate() {
    QDesktopServices::openUrl(QUrl("https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=GM5Z6DYJEVW56&item_name=Donation"));
}
void MainWindow::go_to_project_home() {
    QDesktopServices::openUrl(QUrl("https://github.com/splintermind/Dwarf-Therapist"));
}
void MainWindow::go_to_new_issue() {
    QDesktopServices::openUrl(QUrl("https://github.com/splintermind/Dwarf-Therapist/issues?state=open"));
}

void MainWindow::open_help(){
    const QString local_manual = "share:doc/Therapist Manual.pdf";
    QUrl url = QFile::exists(local_manual) ? QUrl("http://dffd.wimbli.com/file.php?id=7889")
                                           : QUrl::fromLocalFile(local_manual);
    QDesktopServices::openUrl(url);
}

QToolBar *MainWindow::get_toolbar() {
    return ui->main_toolbar;
}

void MainWindow::export_custom_professions() {
    ImportExportDialog d(this);
    if(d.setup_for_profession_export())
        d.exec();
}

void MainWindow::import_custom_professions() {
    ImportExportDialog d(this);
    if(d.setup_for_profession_import())
        d.exec();
}

void MainWindow::export_custom_roles(){
    ImportExportDialog d(this);
    if(d.setup_for_role_export())
        d.exec();
}

void MainWindow::import_custom_roles(){
    ImportExportDialog d(this);
    if(d.setup_for_role_import())
        d.exec();
}

void MainWindow::save_gridview_csv()
{
    GridView *gv = m_view_manager->get_active_view();

    QString defaultPath = QString("%1.csv").arg(gv->name());
    QString fileName = QFileDialog::getSaveFileName(0 , tr("Save file as"), defaultPath, tr("csv files (*.csv)"));
    if (fileName.length()==0)
        return;
    if (!fileName.endsWith(".csv"))
        fileName.append(".csv");
    QFile f( fileName );
    if (f.exists())
        f.remove();
    f.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&f);

    QStringList row;
    row.append(tr("Name"));
    foreach(ViewColumnSet *set, gv->sets()) {
        foreach(ViewColumn *col, set->columns()) {
            if (col->type() != CT_SPACER)
                row.append(col->title());
        }
    }
    out << row.join(",") << endl;
    row.clear();

    QList<Dwarf*> dwarves = m_proxy->get_filtered_dwarves();
    foreach(Dwarf *d, dwarves){
        if(d->is_animal() || d->is_adult() || (!DT->hide_non_adults() && !d->is_adult())){
            row.append(d->nice_name());
            foreach(ViewColumnSet *set, gv->sets()) {
                foreach(ViewColumn *col, set->columns()) {
                    if (col->type() != CT_SPACER)
                        row.append(col->get_cell_value(d));
                }
            }
            out << row.join(",") << endl;
            row.clear();
        }
    }
    f.close();
}

void MainWindow::export_gridviews() {
    ImportExportDialog d(this);
    if(d.setup_for_gridview_export())
        d.exec();
}

void MainWindow::import_gridviews() {
    ImportExportDialog d(this);
    if(d.setup_for_gridview_import()){
        if (d.exec()) {
            GridViewDock *dock = qobject_cast<GridViewDock*>(QObject::findChild<GridViewDock*>("GridViewDock"));
            if(dock)
                dock->draw_views();
        }
    }
}

void MainWindow::clear_user_settings() {
    QMessageBox *mb = new QMessageBox(qApp->activeWindow());
    mb->setIcon(QMessageBox::Warning);
    mb->setWindowTitle(tr("Clear User Settings"));
    mb->setText(tr("Warning: This will delete all of your user settings and exit Dwarf Therapist!"));
    mb->addButton(QMessageBox::Ok);
    mb->addButton(QMessageBox::Cancel);
    if(QMessageBox::Ok == mb->exec()) {
        //Delete data
        m_settings->clear();
        m_settings->sync();

        QFile file(m_settings->fileName());
        LOGI << "Removing file:" << m_settings->fileName();

        delete m_settings;
        m_settings = 0;

        if(!file.remove()) {
            LOGW << "Error removing file!";
            delete mb;
            mb = new QMessageBox(qApp->activeWindow());
            mb->setIcon(QMessageBox::Critical);
            mb->setWindowTitle("Clear User Settings");
            mb->setText(tr("Unable to delete settings file."));
            mb->exec();
            return;
        }

        m_deleting_settings = true;
        close();
    }
}


void MainWindow::show_dwarf_details_dock(Dwarf *d) {
    DwarfDetailsDock *dock = qobject_cast<DwarfDetailsDock*>(QObject::findChild<DwarfDetailsDock*>("dock_dwarf_details"));
    if (dock && d) {
        dock->show_dwarf(d);
        dock->show();
    }
}

void MainWindow::add_new_filter_script() {
    m_script_dialog->clear_script();
    m_script_dialog->show();
}

void MainWindow::edit_filter_script() {
    QAction *a = qobject_cast<QAction*>(QObject::sender());
    QStringList script = a->data().toStringList();
    m_script_dialog->load_script(script.at(0),script.at(1));
    m_script_dialog->show();
}

void MainWindow::remove_filter_script(){
    QAction *a = qobject_cast<QAction*>(QObject::sender());
    QString name =a->data().toString();

    int answer = QMessageBox::question(0,"Confirm Remove",tr("Are you sure you want to remove script: <b>%1</b>?").arg(name),QMessageBox::Yes,QMessageBox::No);
    if(answer == QMessageBox::Yes){
        QSettings *s = DT->user_settings();
        s->remove(QString("filter_scripts/%1").arg(name));
    }
    reload_filter_scripts();
}

void MainWindow::reload_filter_scripts() {
    ui->cb_filter_script->clear();
    ui->cb_filter_script->addItem("");

    ui->menu_edit_filters->clear();
    ui->menu_remove_script->clear();
    QSettings *s = DT->user_settings();

    QMap<QString,QString> m_scripts;
    s->beginGroup("filter_scripts");
    foreach(QString script_name, s->childKeys()){
        m_scripts.insert(script_name,s->value(script_name).toString());
    }
    s->endGroup();

    QMap<QString, QString>::const_iterator i = m_scripts.constBegin();
    while(i != m_scripts.constEnd()){
        QStringList data;
        data.append(i.key());
        data.append(i.value());
        QAction *a = ui->menu_edit_filters->addAction(i.key(),this,SLOT(edit_filter_script()) );
        a->setData(data);

        QAction *r = ui->menu_remove_script->addAction(i.key(),this,SLOT(remove_filter_script()) );
        r->setData(i.key());
        i++;
    }
    ui->cb_filter_script->addItems(m_scripts.uniqueKeys());
}

void MainWindow::add_new_custom_role() {
    if(m_role_editor){
        connect(m_view_manager, SIGNAL(selection_changed()), m_role_editor, SLOT(selection_changed()),Qt::UniqueConnection);
        m_role_editor->load_role("");
        m_role_editor->show();
    }
}

void MainWindow::edit_custom_role() {
    QAction *a = qobject_cast<QAction*>(QObject::sender());
    QString name = a->data().toString();
    if(m_role_editor){
        connect(m_view_manager, SIGNAL(selection_changed()), m_role_editor, SLOT(selection_changed()),Qt::UniqueConnection);
        m_role_editor->load_role(name);
        m_role_editor->show();
    }
}

void MainWindow::done_editing_role(int result){
    if(result == QDialog::Accepted){
        write_custom_roles();
        refresh_roles_data();
    }
    disconnect(m_view_manager, SIGNAL(selection_changed()), m_role_editor, SLOT(selection_changed()));
}

void MainWindow::remove_custom_role(){
    QAction *a = qobject_cast<QAction*>(QObject::sender());
    QString name = a->data().toString();
    int answer = QMessageBox::question(0,"Confirm Remove",tr("Are you sure you want to remove role: <b>%1</b>?").arg(name),QMessageBox::Yes,QMessageBox::No);
    if(answer == QMessageBox::Yes){
        GameDataReader::ptr()->get_roles().remove(name);

        //prompt and remove columns??
        answer = QMessageBox::question(0,"Clean Views",tr("Would you also like to remove role <b>%1</b> from all custom views?").arg(name),QMessageBox::Yes,QMessageBox::No);
        if(answer == QMessageBox::Yes){
            ViewManager *vm = m_view_manager;
            foreach(GridView *gv, vm->views()){
                if(gv->is_custom() && gv->is_active()){ //only remove from custom views which are active
                    foreach(ViewColumnSet *vcs, gv->sets()){
                        foreach(ViewColumn *vc, vcs->columns()){
                            if(vc->type()==CT_ROLE){
                                RoleColumn *rc = ((RoleColumn*)vc);
                                if(rc->get_role() && rc->get_role()->name==name){
                                    vcs->remove_column(vc);
                                }
                            }
                        }
                    }
                }
            }
        }
        //first write our custom roles
        write_custom_roles();
        //re-read roles from the ini to replace any default roles that may have been replaced by a custom role which was just removed
        //this will also rebuild our sorted role list
        GameDataReader::ptr()->load_roles();
        //update our current roles/ui elements
        DT->emit_roles_changed();
        refresh_role_menus();
        if(m_df){
            m_view_manager->update();
            m_view_manager->redraw_current_tab();
        }
    }
}

void MainWindow::refresh_roles_data(){
    DT->emit_roles_changed();
    GameDataReader::ptr()->load_role_mappings();

    refresh_role_menus();
    if(m_df){
        m_view_manager->update();
        m_view_manager->redraw_current_tab();
    }
}

void MainWindow::refresh_role_menus() {
    ui->menu_edit_roles->clear();
    ui->menu_remove_roles->clear();

    QList<QPair<QString, Role*> > roles = GameDataReader::ptr()->get_ordered_roles();
    QPair<QString, Role*> role_pair;
    foreach(role_pair, roles){
        if(role_pair.second->is_custom){
            QAction *edit = ui->menu_edit_roles->addAction(role_pair.first,this,SLOT(edit_custom_role()) );
            edit->setData(role_pair.first);

            QAction *rem = ui->menu_remove_roles->addAction(role_pair.first,this,SLOT(remove_custom_role()) );
            rem->setData(role_pair.first);
        }
    }
}

void MainWindow::write_custom_roles(){
    //re-write custom roles, ugly but doesn't seem that replacing only one works properly
    QSettings *s = DT->user_settings();
    s->remove("custom_roles");

    //read defaults before we start writing
    float default_attributes_weight = s->value("options/default_attributes_weight",1.0).toFloat();
    float default_skills_weight = s->value("options/default_skills_weight",1.0).toFloat();
    float default_traits_weight = s->value("options/default_traits_weight",1.0).toFloat();
    float default_prefs_weight = s->value("options/default_prefs_weight",1.0).toFloat();

    s->beginWriteArray("custom_roles");
    int count = 0;
    foreach(Role *r, GameDataReader::ptr()->get_roles()){
        if(r->is_custom){
            s->setArrayIndex(count);
            r->write_to_ini(*s, default_attributes_weight, default_traits_weight, default_skills_weight, default_prefs_weight);
            count++;
        }
    }
    s->endArray();
}




void MainWindow::new_filter_script_chosen(const QString &script_name) {
    if(!script_name.trimmed().isEmpty()){
        m_proxy->apply_script(script_name,DT->user_settings()->value(QString("filter_scripts/%1").arg(script_name), QString()).toString());
        ui->cb_filter_script->blockSignals(true);
        ui->cb_filter_script->setCurrentIndex(0);
        ui->cb_filter_script->blockSignals(false);
    }
}

void MainWindow::print_gridview() {
    QWidget *curr = get_view_manager()->currentWidget();
    if(!curr)
        return;
    StateTableView *s;
    s = qobject_cast<StateTableView*>(curr);

    if(!s)
        return;

    if(!m_view_manager || !m_view_manager->get_active_view())
        return;

    QString path = QFileDialog::getSaveFileName(this,tr("Save Snapshot"),m_view_manager->get_active_view()->name(),tr("PNG (*.png);;All Files (*)"));
    if(path.isEmpty())
        return;

    int w = 0;
    int h = 0;
    QSize currSize = this->size();
    QSize currMax = this->maximumSize();
    QSize currMin = this->minimumSize();
    int cell_size = DT->user_settings()->value("options/grid/cell_size", DEFAULT_CELL_SIZE).toInt();
    int cell_pad = DT->user_settings()->value("options/grid/cell_padding", 0).toInt();

    int actual_cell_size = (cell_size + 2) + (2 * cell_pad); //cell size + 2 for the border lines + (2 * cell padding)

    //currently this is just a hack to resize the form to ensure all rows/columns are showing
    //then rendering to the painter and resizing back to the previous size
    //it may be possible to avoid this by using the opengl libs and accessing the frame buffer

    //calculate the width
    int first_col_width = s->columnWidth(0);
    w = first_col_width;
    w += (this->width() - s->width());
    foreach(ViewColumnSet *vcs, m_view_manager->get_active_view()->sets()){
        foreach(ViewColumn *vc, vcs->columns()){
            if(vc->type() != CT_SPACER)
                w += actual_cell_size;
            else
                w += DEFAULT_SPACER_WIDTH;
        }
    }
    w += 2;

    //calculate the height
    h = (s->get_model()->total_row_count * actual_cell_size) + s->get_header()->height();
    h += (this->height() - s->height());
    h += 2; //small buffer for the edge

    this->setMaximumHeight(h);
    this->setMaximumWidth(w);
    this->setMinimumSize(100,100);
    this->resize(w,h);

    QImage img(QSize(s->width(),s->height()),QImage::Format_ARGB32_Premultiplied);
    QPainter painter(&img);
    painter.setRenderHints(QPainter::SmoothPixmapTransform);
    s->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    s->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    s->render(&painter,QPoint(0,0),QRegion(0,0,s->width(),s->height()),QWidget::DrawChildren | QWidget::DrawWindowBackground);
    painter.end();
    img.save(path,"PNG");
    s->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    s->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    this->setMaximumSize(currMax);
    this->setMinimumSize(currMin);
    this->resize(currSize);
}

///////////////////////////////////////////////////////////////////////////////
//! Progress Stuff
void MainWindow::set_progress_message(const QString &msg) {
    m_lbl_message->setText(msg);
}

void MainWindow::set_progress_range(int min, int max) {
    m_progress->setVisible(true);
    m_progress->setRange(min, max);
    m_progress->setValue(min);
    statusBar()->insertPermanentWidget(1, m_progress, 1);
}

void MainWindow::set_progress_value(int value) {
    m_progress->setValue(value);
    if (value >= m_progress->maximum()) {
        statusBar()->removeWidget(m_progress);
        m_progress->setVisible(false);
        set_progress_message("");
    }
}
///////////////////////////////////////////////////////////////////////////////

void MainWindow::display_group(const int group_by){
    //this is a signal sent from the view manager when we change tabs and update grouping
    //we also want to change the combobox's index, but not refresh the grid again
    ui->cb_group_by->blockSignals(true);
    int idx = ui->cb_group_by->findData(static_cast<DwarfModel::GROUP_BY>(group_by));
    if(idx < 0)
        idx = 0;
    ui->cb_group_by->setCurrentIndex(idx);
    ui->cb_group_by->blockSignals(false);
}

void MainWindow::preference_selected(QList<QPair<QString,QString> > vals, QString filter_name, FILTER_SCRIPT_TYPE pType){
    if(pType == SCR_PREF)
        m_proxy->clear_script(pType,false);
    //pairs are of category, pref_name
    QString filter = "";
    QStringList pref_names;
    if(!vals.empty()){
        QPair<QString,QString> pref;
        bool create_name = (filter_name.isEmpty());
        //function args are reversed, pref_name, category
        foreach(pref,vals){
            if(pref.first == Preference::get_pref_desc(LIKE_CREATURE)){
                filter.append(QString("(d.find_preference(\"%1\", \"%2\") || d.find_preference(\"%1\", \"%3\")) && ")
                                              .arg(pref.second.toLower()).arg(pref.first)
                                              .arg(Preference::get_pref_desc(HATE_CREATURE)));
            }else{
                filter.append(QString("d.find_preference(\"%1\", \"%2\") && ").arg(pref.second.toLower()).arg(pref.first));
            }
            if(create_name){
                if(vals.count() == 1)
                    filter_name = tr("%1: %2").arg(capitalize(pref.first)).arg(capitalize(pref.second));
                else
                    pref_names.append(QString("%1 (%2)").arg(capitalize(pref.second)).arg(capitalize(pref.first)));
            }
        }
        filter.chop(4);
        if(pref_names.count()>0)
            filter_name = tr("Preferences: ").append(pref_names.join(","));

        m_proxy->apply_script(filter_name, filter, pType);
    }else{
        m_proxy->clear_script(filter_name);
    }

    m_proxy->refresh_script();
}

void MainWindow::thought_selected(QList<short> ids){
    QString filter = "";
    if(!ids.empty()){
        foreach(short id, ids){
            filter.append(QString("d.has_thought(%1) && ").arg(QString::number(id)));
        }
        filter.chop(4);
        m_proxy->apply_script("thoughts",filter);
    }else{
        m_proxy->clear_script("thoughts");
    }
    m_proxy->refresh_script();
}

void MainWindow::health_legend_selected(QList<QPair<int, int> > vals){
    QStringList filters;
    if(!vals.isEmpty()){
        QPair<int,int> key_pair;
        foreach(key_pair, vals){
            filters.append(QString("d.has_health_issue(%1,%2)").arg(QString::number(key_pair.first)).arg(QString::number(key_pair.second)));
        }
        m_proxy->apply_script("health",filters.join(" && "));
    }else{
        m_proxy->clear_script("health");
    }
    m_proxy->refresh_script();
}

void MainWindow::add_new_opt()
{
    if(m_optimize_plan_editor){
        connect(m_view_manager, SIGNAL(selection_changed()), m_optimize_plan_editor, SLOT(populationChanged()), Qt::UniqueConnection);
        connect(m_proxy, SIGNAL(filter_changed()), m_optimize_plan_editor, SLOT(populationChanged()), Qt::UniqueConnection);
        m_optimize_plan_editor->load_plan("");
        m_optimize_plan_editor->show();
    }
}

void MainWindow::edit_opt() {
    if(m_optimize_plan_editor){
        connect(m_view_manager, SIGNAL(selection_changed()), m_optimize_plan_editor, SLOT(populationChanged()), Qt::UniqueConnection);
        connect(m_proxy, SIGNAL(filter_changed()), m_optimize_plan_editor, SLOT(populationChanged()), Qt::UniqueConnection);
        QAction *a = qobject_cast<QAction*>(QObject::sender());
        QString name = a->data().toString();
        m_optimize_plan_editor->load_plan(name);
        m_optimize_plan_editor->show();
    }
}

void MainWindow::done_editing_opt_plan(int result){
    if(result == QDialog::Accepted){
        disconnect(m_view_manager, SIGNAL(selection_changed()), m_optimize_plan_editor, SLOT(populationChanged()));
        disconnect(m_proxy, SIGNAL(filter_changed()), m_optimize_plan_editor, SLOT(populationChanged()));
        write_labor_optimizations();
        refresh_opts_data();
    }
}

void MainWindow::remove_opt(){
    QAction *a = qobject_cast<QAction*>(QObject::sender());
    QString name = a->data().toString();
    int answer = QMessageBox::question(0,"Confirm Remove",tr("Are you sure you want to remove optimization plan: <b>%1</b>?").arg(name),QMessageBox::Yes,QMessageBox::No);
    if(answer == QMessageBox::Yes){
        GameDataReader::ptr()->get_opt_plans().remove(name);
        write_labor_optimizations();
        refresh_opts_data();
    }
}

void MainWindow::refresh_opts_data(){
    GameDataReader::ptr()->refresh_opt_plans();
    refresh_opts_menus();
}

void MainWindow::write_labor_optimizations(){
    //save to the ini file
    QSettings *s = DT->user_settings();
    s->remove("labor_optimizations");

    s->beginWriteArray("labor_optimizations");
    int count = 0;
    QHash<QString, laborOptimizerPlan*> plans = GameDataReader::ptr()->get_opt_plans();
    foreach(QString key, plans.uniqueKeys()){
        s->setArrayIndex(count);
        plans.value(key)->write_to_ini(*s);
        count++;
    }
    s->endArray();
}

void MainWindow::toggle_opts_menu(){
    if(m_view_manager && m_view_manager->get_selected_dwarfs().count() <= 0)
        m_btn_optimize->setEnabled(false);
    else
        m_btn_optimize->setEnabled(true);
}

void MainWindow::refresh_opts_menus() {
    //setup the optimize button
    if(!m_btn_optimize && ! m_act_sep_optimize){
        m_btn_optimize = new QToolButton(ui->main_toolbar);
        if (DT->user_settings()->value("options/show_toolbutton_text", true).toBool()) {
            m_btn_optimize->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        } else {
            m_btn_optimize->setToolButtonStyle(Qt::ToolButtonIconOnly);
        }
        m_btn_optimize->setText("&Optimize");
        m_btn_optimize->setStatusTip("Optimize (CTRL+O)");
        //m_btn_optimize->setMinimumWidth(70);
        m_btn_optimize->setObjectName("m_btn_optimize");
        m_btn_optimize->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_O));
        new QShortcut(Qt::CTRL + Qt::Key_O, this, SLOT(init_optimize()));
        QIcon btn_icon;
        btn_icon.addFile(QString::fromUtf8(":/img/control.png"), QSize(), QIcon::Normal, QIcon::Off);
        m_btn_optimize->setIcon(btn_icon);
        connect(m_btn_optimize, SIGNAL(clicked()), this, SLOT(init_optimize()));

        m_act_btn_optimize = ui->main_toolbar->insertWidget(ui->act_options, m_btn_optimize);
        m_act_sep_optimize = ui->main_toolbar->insertSeparator(ui->act_options);
    }
    QMenu *opt_menu = new QMenu(m_btn_optimize);

    ui->menu_edit_opts->clear();
    ui->menu_remove_opt->clear();
    opt_menu->clear();

    QList<QPair<QString, laborOptimizerPlan*> > plans = GameDataReader::ptr()->get_ordered_opt_plans();
    QPair<QString, laborOptimizerPlan*> plan_pair;
    foreach(plan_pair, plans){
            QAction *edit = ui->menu_edit_opts->addAction(plan_pair.first,this,SLOT(edit_opt()));
            edit->setData(plan_pair.first);

            QAction *rem = ui->menu_remove_opt->addAction(plan_pair.first,this,SLOT(remove_opt()));
            rem->setData(plan_pair.first);

            QAction *o = opt_menu->addAction(plan_pair.first, this, SLOT(init_optimize()));
            o->setData(plan_pair.first);
    }


    if(opt_menu->actions().count() <= 0){
        if(m_btn_optimize->menu()){
            m_btn_optimize->menu()->clear();
            m_btn_optimize->setMenu(NULL);
        }
        m_btn_optimize->setPopupMode(QToolButton::DelayedPopup);
        m_btn_optimize->setProperty("last_optimize","");
        m_btn_optimize->setToolTip("");
        m_act_btn_optimize->setVisible(false);
        m_act_sep_optimize->setVisible(false);
    }else if(opt_menu->actions().count() >= 1){
        QString name = plans.at(0).first;
        if(opt_menu->actions().count() == 1){
            if(m_btn_optimize->menu()){
                m_btn_optimize->menu()->clear();
                m_btn_optimize->setMenu(NULL);
            }
            m_btn_optimize->setPopupMode(QToolButton::DelayedPopup);
        }else{
            m_btn_optimize->setMenu(opt_menu);
            m_btn_optimize->setPopupMode(QToolButton::MenuButtonPopup);
        }
        m_btn_optimize->setProperty("last_optimize",name);
        m_btn_optimize->setToolTip(tr("Optimize labors using %1").arg(name));
        m_act_btn_optimize->setVisible(true);
        m_act_sep_optimize->setVisible(true);
    }
    //start disabled
    m_btn_optimize->setEnabled(false);
}

void MainWindow::init_optimize(){
    if(!m_df)
        return;

    m_act_btn_optimize->setEnabled(false);
    QAction *a = qobject_cast<QAction*>(QObject::sender());
    QString name = "";
    if(a)
        name = a->data().toString();
    else
        name = m_btn_optimize->property("last_optimize").toString();

    if(!name.isEmpty()){
        optimize(name);

        m_btn_optimize->setProperty("last_optimize",name);
        m_btn_optimize->setToolTip("Optimize selected using " + name);

        m_model->calculate_pending();
        DT->emit_labor_counts_updated();
    }

    m_act_btn_optimize->setEnabled(true);
}

void MainWindow::optimize(QString plan_name){
    if(!m_df)
        return;

    laborOptimizerPlan *p = GameDataReader::ptr()->get_opt_plans().value(plan_name);
    if(!p){
        QMessageBox::information(this, tr("Plan Missing"), tr("Couldn't find optimization plan."));
        return;
    }
    LaborOptimizer *o = new LaborOptimizer(p,this);
    QList<Dwarf*> dwarfs = m_view_manager->get_selected_dwarfs();
    if(dwarfs.count() <= 0)
        dwarfs = m_proxy->get_filtered_dwarves();

    o->optimize_labors(dwarfs);
}

void MainWindow::main_toolbar_style_changed(Qt::ToolButtonStyle button_style){
    //update any manually added buttons' style here
    m_btn_optimize->setToolButtonStyle(button_style);
}

void MainWindow::reset(){
    if(DT->multiple_castes){
        DT->multiple_castes = false;
        int grp_by = ui->cb_group_by->itemData(ui->cb_group_by->currentIndex()).toInt();
        if(grp_by == 3) //caste tag
            grp_by = 2; //set to caste in case
        ui->cb_group_by->blockSignals(true);
        ui->cb_group_by->removeItem(3); //GB_CASTE_TAG
        ui->cb_group_by->setCurrentIndex(ui->cb_group_by->findData(grp_by));
        ui->cb_group_by->blockSignals(false);
    }

    this->setWindowTitle("Dwarf Therapist - Disconnected");

    DwarfDetailsDock *dock = qobject_cast<DwarfDetailsDock*>(QObject::findChild<DwarfDetailsDock*>("dock_dwarf_details"));
    if(dock)
        dock->clear();

    //clear selected dwarfs
    if(m_view_manager)
        m_view_manager->clear_selected();

    //clear dock lists
    PreferencesDock *d_prefs = qobject_cast<PreferencesDock*>(QObject::findChild<PreferencesDock*>("dock_preferences"));
    if(d_prefs){
        d_prefs->clear();
    }
    ThoughtsDock *d_thoughts = qobject_cast<ThoughtsDock*>(QObject::findChild<ThoughtsDock*>("dock_thoughts"));
    if(d_thoughts){
        d_thoughts->clear();
    }

    new_creatures_count(0,0,0,tr("Dwarfs"));
}

void MainWindow::clear_filter(){
    QAction *a = qobject_cast<QAction*>(QObject::sender());
    QString name = a->data().toString();
    m_proxy->clear_script(name);
}

void MainWindow::clear_all_filters(){
    ui->le_filter_text->clear();
    m_proxy->clear_script();
}

void MainWindow::refresh_active_scripts(){
    QList<QString> names = m_proxy->get_script_names();
    ui->btn_clear_filters->setMenu(NULL);
    QMenu *scripts = new QMenu(ui->btn_clear_filters);
    foreach(QString n, names){
        QAction *a = scripts->addAction(QIcon(":img/cross.png"), capitalizeEach(n.replace("&","&&")),this,SLOT(clear_filter()));
        a->setData(n);
    }

    int script_count = scripts->actions().count();
    if(!m_proxy->active_scripts()){
        if(ui->btn_clear_filters->menu()){
            ui->btn_clear_filters->menu()->clear();
            ui->btn_clear_filters->setMenu(NULL);
        }
        ui->btn_clear_filters->setText(tr("Active Filters"));
        ui->btn_clear_filters->setPopupMode(QToolButton::DelayedPopup);
        m_pop_info.filtered = -1;
    }else{
        ui->btn_clear_filters->setMenu(scripts);
        ui->btn_clear_filters->setPopupMode(QToolButton::MenuButtonPopup);
        ui->btn_clear_filters->setText(QString::number(script_count) + (" Active Filters"));
        m_pop_info.filtered = m_proxy->get_filtered_dwarves().count();
    }
    ui->btn_clear_filters->updateGeometry();
    m_view_manager->expand_all();
    refresh_pop_counts();
}
