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
	QString out = QString("<b>%1\t= <font color=blue>0x%2</font> (uncorrected:0x%3)\n")
		.arg(msg)
		.arg(addr - m_df->get_memory_correction(), 8, 16, QChar('0'))
		.arg(addr, 8, 16, QChar('0'));
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

void Scanner::prepare_new_thread(SCANNER_JOB_TYPE type) {
	if (m_thread && m_thread->isRunning()) {
		m_thread->terminate();
		m_thread->wait(3000);
		m_thread->deleteLater();
		m_thread = 0;
	}
		
	m_thread = new ScannerThread;
	m_thread->set_job(type);
	connect(m_thread, SIGNAL(main_scan_total_steps(int)), ui->pb_main, SLOT(setMaximum(int)));
	connect(m_thread, SIGNAL(main_scan_progress(int)), ui->pb_main, SLOT(setValue(int)));
	connect(m_thread, SIGNAL(sub_scan_total_steps(int)), ui->pb_sub, SLOT(setMaximum(int)));
	connect(m_thread, SIGNAL(sub_scan_progress(int)), ui->pb_sub, SLOT(setValue(int)));
	connect(m_thread, SIGNAL(scan_message(const QString&)), ui->lbl_scan_progress, SLOT(setText(const QString&)));
	connect(m_thread, SIGNAL(found_address(const QString&, const uint&)), this, SLOT(report_address(const QString&, const uint&)));
	connect(m_thread, SIGNAL(found_offset(const QString&, const int&)), this, SLOT(report_offset(const QString&, const int&)));
}

void Scanner::run_thread_and_wait() {
	if (!m_thread) {
		LOGW << "can't run a thread that was never set up! (m_thread == 0)";
		return;
	}
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
}

void Scanner::find_translations_vector() {
	set_ui_enabled(false);
	prepare_new_thread(FIND_TRANSLATIONS_VECTOR);
	run_thread_and_wait();
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

void Scanner::find_stone_vector() {
	set_ui_enabled(false);
	prepare_new_thread(FIND_STONE_VECTOR);
	run_thread_and_wait();
	set_ui_enabled(true);
}

void Scanner::find_metal_vector() {
	set_ui_enabled(false);
	set_ui_enabled(true);
}

void Scanner::find_null_terminated_string() {
	set_ui_enabled(false);
	prepare_new_thread(FIND_NULL_TERMINATED_STRING);
	QByteArray needle = ui->le_null_terminated_string->text().toLocal8Bit();
	m_thread->set_meta(needle);
	run_thread_and_wait();
	set_ui_enabled(true);
}

void Scanner::find_number_or_address() {
	set_ui_enabled(false);
	prepare_new_thread(FIND_NULL_TERMINATED_STRING); //re-use the basic bit searcher
	bool ok;
	QByteArray needle = encode(ui->le_find_address->text().toUInt(&ok, ui->rb_hex->isChecked() ? 16 : 10));
	m_thread->set_meta(needle);
	run_thread_and_wait();
	set_ui_enabled(true);
}

void Scanner::brute_force_read() {
	set_ui_enabled(false);
	bool ok; // for base conversions
	uint addr = ui->le_address->text().toUInt(&ok, 16);
	switch(ui->cb_interpret_as_type->currentIndex()) {
		case 0: // std::string
			ui->le_read_output->setText(m_df->read_string(addr));
			break;
		case 1: // null terminated string
			{
				char *buf = new char[512];
				m_df->read_raw(addr, 512, buf);
				QString val = QString::fromAscii(buf);
				ui->le_read_output->setText(val);
				delete[] buf;
			}
			break;
		case 2: // int
			ui->le_read_output->setText(QString::number(m_df->read_int(addr)));
			break;
		case 3: // uint
			ui->le_read_output->setText(QString::number(m_df->read_uint(addr)));
			break;
		case 4: // short
			ui->le_read_output->setText(QString::number(m_df->read_short(addr)));
			break;
		case 5: // ushort
			ui->le_read_output->setText(QString::number(m_df->read_ushort(addr)));
			break;
		case 6: // std::vector<void*>
			{
				QVector<uint> addresses = m_df->enumerate_vector(addr);
				ui->text_output->append(QString("Vector at 0x%1 contains %2 entries...").arg(addr, 8, 16, QChar('0')).arg(addresses.size()));
				foreach(uint a, addresses) {
					ui->text_output->append(QString("0x%1").arg(a, 8, 16, QChar('0')));
				}
			}
			break;
		case 7: // raw
			ui->text_output->append(m_df->pprint(addr, 0x600));
			break;
	}
	set_ui_enabled(true);
}