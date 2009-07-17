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

private:
	bool m_reading_settings;
	Ui::OptionsMenu *ui;

	private slots:
		void color_changed(const QColor &c);

signals:
	void picker_changed(MainWindow::CONFIGURABLE_COLORS, const QColor &);
};
#endif;