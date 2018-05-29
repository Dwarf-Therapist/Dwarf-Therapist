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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>

#include "global_enums.h"

class AboutDialog;
class DFInstance;
class Dwarf;
class DwarfModel;
class DwarfModelProxy;
class RolePreferenceModel;
class NotifierWidget;
class QCompleter;
class QLabel;
class QProgressBar;
class QSettings;
class QToolButton;
class ScriptDialog;
class Updater;
class ViewManager;
class optimizereditor;
class RoleDialog;

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

    QSettings *get_settings() {return m_settings.get();}
    QToolBar *get_toolbar();
    DwarfModel *get_model() {return m_model;}
    DwarfModelProxy *get_proxy() {return m_proxy;}
    ViewManager *get_view_manager() {return m_view_manager;}
    DFInstance *get_DFInstance() {return m_df;}

    Ui::MainWindow *ui;

    struct pop_info{
        QString race_name;
        int adults;
        int children;
        int infants;
        int filtered;
    };

public slots:
    // DF related
    void connect_to_df();
    void read_dwarves();
    void new_pending_changes(int);
    void new_creatures_count(int, int, int, QString);
    void lost_df_connection(bool show_dialog = true);

    //settings
    void set_group_by(int);
    void export_custom_professions();
    void import_custom_professions();
    void export_gridviews();
    void import_gridviews();
    void import_custom_roles();
    void export_custom_roles();
    void clear_user_settings();

    // about
    void show_about();

    //pending changes
    void list_pending();

    //custom profession
    void load_customizations();
    void draw_custom_profession_context_menu(const QPoint &);

    //filter scripts
    void add_new_filter_script();
    void edit_filter_script();
    void remove_filter_script();

    //gridview exporting
    void print_gridview();
    void save_gridview_csv();

    // links
    void go_to_forums();
    void go_to_donate();
    void go_to_project_home();
    void go_to_new_issue();
    void go_to_latest_release();
    void check_latest_version();
    void open_data_dir();

    //help
    void open_help();

    // progress/status
    void set_progress_message(const QString &msg);
    void set_progress_range(int min, int max);
    void set_progress_value(int value);
    void set_status_message(QString msg, QString tooltip_msg);

    // misc
    void show_dwarf_details_dock(Dwarf *d = 0);
    void new_filter_script_chosen();
    void reload_filter_scripts();

    //roles
    void add_new_custom_role();
    void add_new_opt();
    void write_roles(bool custom = true);
    void refresh_roles_data();

    //optimizer
    void refresh_opts_data();
    void write_labor_optimizations();
    void init_optimize();
    void optimize(QString plan_name);

    //filter scripts
    void refresh_active_scripts();
    void clear_filter();

private:
    DFInstance *m_df;
    QLabel *m_lbl_status;
    QLabel *m_lbl_message;
    QProgressBar *m_progress;
    std::unique_ptr<QSettings> m_settings;
    ViewManager *m_view_manager;
    DwarfModel *m_model;
    DwarfModelProxy *m_proxy;
    RolePreferenceModel *m_pref_model;
    AboutDialog *m_about_dialog;
    ScriptDialog *m_script_dialog;
    RoleDialog *m_role_editor;
    optimizereditor *m_optimize_plan_editor;
    bool m_reading_settings;
    bool m_show_result_on_equal; //! used during version checks
    QCompleter *m_dwarf_name_completer;
    QStringList m_dwarf_names_list;
    bool m_try_download;
    bool m_deleting_settings;
    pop_info m_pop_info;
    bool m_toolbar_configured;
    QVector<int> m_selected_units;

    //optimize button and separator widgets and their corresponding toolbar actions
    QAction *m_act_sep_optimize;
    QAction *m_act_btn_optimize; //this is required in addition to the button to allow easy visibility toggling
    QToolButton *m_btn_optimize;
    QTimer *m_retry_connection;

    Updater *m_updater;
    NotifierWidget *m_notifier;

    void showEvent(QShowEvent *evt);
    void closeEvent(QCloseEvent *evt);
    void resizeEvent(QResizeEvent *evt);
    void moveEvent(QMoveEvent *evt);

    void read_settings();
    void write_settings();

    void refresh_role_menus();

    void refresh_opts_menus();
    void reset();

    void refresh_pop_counts();

private slots:
    void set_interface_enabled(bool);

    void edit_custom_role();
    void remove_custom_role();

    void display_group(const int);
    void apply_filter();
    void apply_filter(QModelIndex);

    void preference_selected(QList<QPair<QString,QString> > vals, QString filter_name = "", FILTER_SCRIPT_TYPE pType = SCR_PREF);
    void thought_selected(QVariantList ids);
    void equipoverview_selected(QVariantList ids);
    void health_legend_selected(QList<QPair<int,int> > vals);

    void toggle_opts_menu();
    void edit_opt();
    void remove_opt();
    void done_editing_opt_plan(int result);
    void done_editing_role(int result);

    void main_toolbar_style_changed(Qt::ToolButtonStyle button_style);
    void clear_all_filters();

    void commit_changes();
    void save_ui_selections();
    void restore_ui_selections();

signals:
    void lostConnection();

};

#endif // MAINWINDOW_H
