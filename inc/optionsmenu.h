#ifndef OPTIONS_MENU_H
#define OPTIONS_MENU_H

#include "mainwindow.h"
#include "ui_optionsmenu.h"

class CustomColor;

class OptionsMenu : public QDialog {
	Q_OBJECT
public:
	OptionsMenu(MainWindow *parent = 0);
	virtual ~OptionsMenu();

	void read_settings(QSettings *s);
	void write_settings(QSettings *s);

	public slots:
		void accept();
		void reject();
		void restore_defaults();
	
private:
	bool m_reading_settings;
	Ui::OptionsMenu *ui;
	QList<CustomColor*> m_general_colors;

signals:
	void color_changed(const QString &, const QColor &);
	//void picker_changed(MainWindow::CONFIGURABLE_COLORS, const QColor &);
};
#endif;