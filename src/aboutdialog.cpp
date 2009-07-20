#include <QtGui>

#include "aboutdialog.h"

AboutDialog::AboutDialog(MainWindow *parent)
	: QDialog(parent)
	, ui(new Ui::AboutDialog)
	, m_version(Version())
{
	ui->setupUi(this);
	ui->lbl_our_version->setText(QString("VERSION %1").arg(m_version.to_string()));
}

void AboutDialog::set_latest_version(const Version &v) {
	if (m_version < v) {
		ui->lbl_up_to_date->setText("Update Available: <a href=\"http://code.google.com/p/dwarftherapist/downloads/list\">v"
			+ v.to_string() + "</a>");
	} else {
		ui->lbl_up_to_date->setText(QString("This version is up to date (v%1)").arg(m_version.to_string()));
	}
}

void AboutDialog::version_check_failed() {
	ui->lbl_up_to_date->setText(tr("Version check failed"));
}
