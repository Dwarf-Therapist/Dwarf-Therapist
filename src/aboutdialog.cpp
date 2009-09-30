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
#include <QtGui>

#include "aboutdialog.h"
#include "dwarftherapist.h"
#include "mainwindow.h"

AboutDialog::AboutDialog(MainWindow *parent)
	: QDialog(parent)
	, ui(new Ui::AboutDialog)
	, m_version(Version())
{
	ui->setupUi(this);
	ui->lbl_our_version->setText(QString("VERSION %1").arg(m_version.to_string()));
    connect(ui->pb_check_version, SIGNAL(clicked()), SLOT(check_version()));
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

void AboutDialog::check_version() {
    DT->get_main_window()->check_latest_version(true);
}