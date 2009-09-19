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
#ifndef SCANNER_H
#define SCANNER_H

#include "mainwindow.h"
#include "ui_scannerdialog.h"
#include "scannerjob.h"

class DFInstance;
class ScannerThread;

class Scanner: public QDialog {
	Q_OBJECT
public:
	Scanner(DFInstance *df, MainWindow *parent = 0);
	virtual ~Scanner(){}

	public slots:
		void report_address(const QString&, const uint&);
		void report_offset(const QString&, const int&);
		void cancel_scan();

private:
	DFInstance *m_df;
	ScannerThread *m_thread;
	Ui::ScannerDialog *ui;
	bool m_stop_scanning;

	void set_ui_enabled(bool enabled);
	void prepare_new_thread(SCANNER_JOB_TYPE type);


	private slots:
		void find_creature_vector();
		void find_dwarf_race_index();
		void find_translations_vector();
		void find_vector_by_length();
		void find_null_terminated_string();
		void brute_force_read();

};
#endif
