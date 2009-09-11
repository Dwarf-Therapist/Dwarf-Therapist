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
#include <QThread>
#include "scanner.h"
#include "dfinstance.h"
#include "gamedatareader.h"
#include "dwarftherapist.h"
#include "scannerthread.h"
#include "defines.h"

Scanner::Scanner(DFInstance *df, MainWindow *parent)
	: QDialog(parent)
	, m_df(df)
	, m_thread(0)
	, ui(new Ui::ScannerDialog)
	, m_stop_scanning(false)
{
	ui->setupUi(this);
	set_ui_enabled(true);
}

void Scanner::cancel_scan() {
	m_stop_scanning = true;
	m_df->cancel_scan();
}

void Scanner::set_ui_enabled(bool enabled) {
	m_stop_scanning = enabled;
	ui->gb_scan_targets->setEnabled(enabled);
	ui->gb_search->setEnabled(enabled);
	ui->gb_brute_force->setEnabled(enabled);
	ui->gb_progress->setEnabled(!enabled);
	ui->btn_cancel_scan->setEnabled(!enabled);
	ui->lbl_scan_progress->setText(tr("Not Scanning"));
	ui->pb_main->reset();
	ui->pb_sub->reset();
}

void Scanner::report_address(const QString &msg, const uint &addr) {
	QString out = QString("<b>%1\t= <font color=blue>0x%2</font>\n")
		.arg(msg)
		.arg(addr - m_df->get_memory_correction(), 8, 16, QChar('0'));
	ui->text_output->append(out);
	LOGD << out;
}

void Scanner::report_offset(const QString &msg, const int &addr) {
	QString out = QString("<b>%1\t= <font color=blue>0x%2</font></b>\n")
		.arg(msg)
		.arg(addr, 8, 16, QChar('0'));
	ui->text_output->append(out);
	LOGD << out;
}

void Scanner::find_translations_vector() {
	set_ui_enabled(false);
	

	// First find all vectors that have the correct number of entries to be lang tables
	LOGD << "Launching search thread from thread" << QThread::currentThreadId();
	m_thread = new ScannerThread;
	m_thread->set_job(FIND_TRANSLATIONS_VECTOR);
	connect(m_thread, SIGNAL(main_scan_total_steps(int)), ui->pb_main, SLOT(setMaximum(int)));
	connect(m_thread, SIGNAL(main_scan_progress(int)), ui->pb_main, SLOT(setValue(int)));
	connect(m_thread, SIGNAL(sub_scan_total_steps(int)), ui->pb_sub, SLOT(setMaximum(int)));
	connect(m_thread, SIGNAL(sub_scan_progress(int)), ui->pb_sub, SLOT(setValue(int)));
	connect(m_thread, SIGNAL(scan_message(const QString&)), ui->lbl_scan_progress, SLOT(setText(const QString&)));
	connect(m_thread, SIGNAL(found_address(const QString&, const uint&)), this, SLOT(report_address(const QString&, const uint&)));
	connect(m_thread, SIGNAL(found_offset(const QString&, const int&)), this, SLOT(report_offset(const QString&, const int&)));
	m_thread->start();
	
	
	while (!m_thread->wait(100)) {
		if (m_stop_scanning)
			break;
		//ui->text_output->append("waiting on thread...");
		DT->processEvents();
	}
	
	m_thread->terminate();
	if (m_thread->wait(5000)) {
		delete m_thread;
	} else {
		LOGC << "Scanning thread failed to stop for 5 seconds after termination!";
	}
	m_thread = 0;
	

	set_ui_enabled(true);
}

void Scanner::find_dwarf_race_index() {
	set_ui_enabled(false);
	set_ui_enabled(true);
}

void Scanner::find_creature_vector() {
	set_ui_enabled(false);
	set_ui_enabled(true);
}

void Scanner::find_vector_by_length() {
	set_ui_enabled(false);
	set_ui_enabled(true);
}

void Scanner::find_null_terminated_string() {
	set_ui_enabled(false);
	set_ui_enabled(true);
}

void Scanner::brute_force_read() {
	set_ui_enabled(false);
	set_ui_enabled(true);
}