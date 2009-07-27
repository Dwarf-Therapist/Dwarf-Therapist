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

class DFInstance;
class DwarfModel;
class DwarfModelProxy;
class Dwarf;
class OptionsMenu;
class AboutDialog;
class CustomProfession;

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

    public slots:
        void connect_to_df();
		void read_dwarves();
		void scan_memory();
		
		//settings
		void show_toolbutton_text(bool);
		void set_group_by(int);
		void set_single_click_labor_changes(bool);

		// dialogs
		void show_about();
		void add_custom_profession();
		void reset_custom_profession();
		void open_options_menu();

		void new_pending_changes(int);
		void list_pending();
		void draw_professions();
		void draw_grid_context_menu(const QPoint &);
		void draw_custom_profession_context_menu(const QPoint &);
		void edit_custom_profession(QListWidgetItem*);
		void edit_custom_profession();
		void delete_custom_profession();
		void import_existing_professions();
		
		void check_latest_version();
		void version_check_finished(bool error);

		//links
		void go_to_forums();
		void go_to_donate();
		void go_to_project_home();
		void go_to_new_issue();
		
private:
    Ui::MainWindow *ui;
    DFInstance *m_df;
	QLabel *m_lbl_status;
	QSettings *m_settings;
	DwarfModel *m_model;
	DwarfModelProxy *m_proxy;
	OptionsMenu *m_options_menu;
	AboutDialog *m_about_dialog;
	QVector<CustomProfession*> m_custom_professions;
	CustomProfession *m_temp_cp;
	QHttp *m_http;
	bool m_reading_settings;

	void closeEvent(QCloseEvent *evt); // override;

	void read_settings();
	void write_settings();

	CustomProfession *get_custom_profession(QString);

    private slots:
        void set_interface_enabled(bool);
		void color_changed(const QString &, const QColor &);
		void apply_custom_profession();
		void set_nickname();
		
};

#endif // MAINWINDOW_H
