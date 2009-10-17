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

#include <QtCore>
#include <QtGui>
#include <QtNetwork>

class StateTableView;
class DFInstance;
class DwarfModel;
class DwarfModelProxy;
class Dwarf;
class AboutDialog;
class CustomProfession;
class ViewManager;
class Scanner;

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

	QSettings *get_settings() {return m_settings;}
	QToolBar *get_toolbar();
	DwarfModel *get_model() {return m_model;}
	DwarfModelProxy *get_proxy() {return m_proxy;}
	ViewManager *get_view_manager() {return m_view_manager;}
    DFInstance *get_DFInstance() {return m_df;}

	Ui::MainWindow *ui;

	public slots:
		// DF related
		void connect_to_df();
		void read_dwarves();
		void scan_memory();
		void new_pending_changes(int);
		void lost_df_connection();
		
		//settings
		void set_group_by(int);
		void export_custom_professions();
		void import_custom_professions();
		void export_gridviews();
		void import_gridviews();

		// dialogs
		void show_about();
		void list_pending();
		void draw_professions();
		void draw_custom_profession_context_menu(const QPoint &);
		
		// version check
		void check_latest_version(bool show_result_on_equal=false);
		void version_check_finished(bool error);

		//links
		void go_to_forums();
		void go_to_donate();
		void go_to_project_home();
		void go_to_new_issue();

		void show_dwarf_details_dock(Dwarf *d = 0);

private:
	DFInstance *m_df;
	QLabel *m_lbl_status;
	QSettings *m_settings;
	ViewManager *m_view_manager;
	DwarfModel *m_model;
	DwarfModelProxy *m_proxy;
	AboutDialog *m_about_dialog;
	CustomProfession *m_temp_cp;
	Scanner *m_scanner;
	QHttp *m_http;
	bool m_reading_settings;
    bool m_show_result_on_equal; //! used during version checks
    QCompleter *m_dwarf_name_completer;
    QStringList m_dwarf_names_list;

	void closeEvent(QCloseEvent *evt); // override;

	void read_settings();
	void write_settings();

	private slots:
		void set_interface_enabled(bool);
		
};

#endif // MAINWINDOW_H
