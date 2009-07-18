#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore>
#include <QtGui>

class DFInstance;
class DwarfModel;
class Dwarf;
class OptionsMenu;
class CustomProfession;

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
		void draw_professions();
		void draw_grid_context_menu(const QPoint &);
		void draw_custom_profession_context_menu(const QPoint &);
		void edit_custom_profession(QListWidgetItem*);
		void edit_custom_profession();
		void delete_custom_profession();
		
		

private:
    Ui::MainWindow *ui;
    DFInstance *m_df;
	QLabel *m_lbl_status;
	QSettings *m_settings;
	DwarfModel *m_model;
	OptionsMenu *m_options_menu;
	QVector<CustomProfession*> m_custom_professions;
	CustomProfession *m_temp_cp;
	bool m_reading_settings;

	void closeEvent(QCloseEvent *evt); // override;

	void read_settings();
	void write_settings();

	CustomProfession *get_custom_profession(QString);

    private slots:
        void set_interface_enabled(bool);
		void color_changed(MainWindow::CONFIGURABLE_COLORS picker, const QColor &c);
		void apply_custom_profession();
		
};

#endif // MAINWINDOW_H
