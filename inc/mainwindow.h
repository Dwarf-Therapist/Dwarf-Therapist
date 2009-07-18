#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore>
#include <QtGui>

class DFInstance;
class DwarfModel;
class Dwarf;
class OptionsMenu;

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
	typedef enum {
		CC_CURSOR,
		CC_DIRTY_BORDER,
		CC_ACTIVE_LABOR,
		CC_ACTIVE_GROUP,
		CC_PARTIAL_LABOR_GROUP
	} CONFIGURABLE_COLORS;

    MainWindow(QWidget *parent = 0);
    ~MainWindow();

	QSettings *get_settings() {return m_settings;}

    public slots:
        void connect_to_df();
		void read_dwarves();
		void scan_memory();
		void filter_dwarves();
		void show_toolbutton_text(bool);
		void set_group_by(int);
		void show_about();
		void add_custom_profession();
		void new_pending_changes(int);
		void list_pending();
		void open_options_menu();
		void new_custom_profession(Dwarf *d);

private:
    Ui::MainWindow *ui;
    DFInstance *m_df;
	QLabel *m_lbl_status;
	QSettings *m_settings;
	DwarfModel *m_model;
	OptionsMenu *m_options_menu;
	bool m_reading_settings;

	void closeEvent(QCloseEvent *evt); // override;

	void read_settings();
	void write_settings();

    private slots:
        void set_interface_enabled(bool);
		void color_changed(MainWindow::CONFIGURABLE_COLORS picker, const QColor &c);
		
};

#endif // MAINWINDOW_H
