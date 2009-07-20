#ifndef ABOUT_DIALOG_H
#define ABOUT_DIALOG_H

#include "mainwindow.h"
#include "ui_about.h"
#include "version.h"

class AboutDialog : public QDialog {
	Q_OBJECT
public:
	AboutDialog(MainWindow *parent = 0);

	void set_latest_version(const Version &v);
	void version_check_failed();
	
private:
	Version m_version;
	Ui::AboutDialog *ui;

};
#endif;