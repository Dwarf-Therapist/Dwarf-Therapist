#ifndef OPTIONS_MENU_H
#define OPTIONS_MENU_H

#include "mainwindow.h"
#include "ui_optionsmenu.h"

class OptionsMenu : public QDialog {
	Q_OBJECT
public:
	OptionsMenu(MainWindow *parent = 0);
	virtual ~OptionsMenu();

	void read_settings(QSettings *s);
	void write_settings(QSettings *s);

	static QColor default_color_cursor;
	static QColor default_color_active_labor;
	static QColor default_color_active_group;
	static QColor default_color_dirty_border;

	public slots:
		void restore_defaults();
	
private:
	bool m_reading_settings;
	Ui::OptionsMenu *ui;

	private slots:
		void color_changed(const QColor &c);

signals:
	void picker_changed(MainWindow::CONFIGURABLE_COLORS, const QColor &);
};
#endif;