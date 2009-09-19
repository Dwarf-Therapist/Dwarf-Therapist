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
#ifndef SCANNER_THREAD_H
#define SCANNER_THREAD_H

#include <QThread>
#include "dfinstance.h"
#include "scannerjob.h"
#include "translationvectorsearchjob.h"
#include "nullterminatedstringsearchjob.h"

class ScannerThread : public QThread {
	Q_OBJECT
public:
	ScannerThread(QObject *parent = 0)
		: QThread(parent)
		, m_meta(QByteArray())
	{}

	void set_job(SCANNER_JOB_TYPE type) {
		m_type = type;
	}
	void set_meta(const QByteArray &meta) {
		m_meta = meta;
	}

	void run() {
		switch (m_type) {
			case FIND_TRANSLATIONS_VECTOR:
				m_job = new TranslationVectorSearchJob;
				break;
			case FIND_NULL_TERMINATED_STRING:
				{
					NullTerminatedStringSearchJob *job = new NullTerminatedStringSearchJob;
					job->set_needle(m_meta);
					m_job = job;
				}
				break;
			default:
				LOGD << "JOB TYPE NOT SET, EXITING THREAD";
				return;
		}
		// forward the status signals
		connect(m_job->df(), SIGNAL(scan_total_steps(int)), SIGNAL(sub_scan_total_steps(int)));
		connect(m_job->df(), SIGNAL(scan_progress(int)), SIGNAL(sub_scan_progress(int)));
		connect(m_job->df(), SIGNAL(scan_message(const QString&)), SIGNAL(scan_message(const QString&)));
		connect(m_job, SIGNAL(scan_message(const QString&)), SIGNAL(scan_message(const QString&)));
		connect(m_job, SIGNAL(found_address(const QString&, const uint&)), SIGNAL(found_address(const QString&, const uint&)));
		connect(m_job, SIGNAL(found_offset(const QString&, const int&)), SIGNAL(found_offset(const QString&, const int&)));
		connect(m_job, SIGNAL(main_scan_total_steps(int)), SIGNAL(main_scan_total_steps(int)));
		connect(m_job, SIGNAL(main_scan_progress(int)), SIGNAL(main_scan_progress(int)));
		connect(m_job, SIGNAL(sub_scan_total_steps(int)), SIGNAL(sub_scan_total_steps(int)));
		connect(m_job, SIGNAL(sub_scan_progress(int)), SIGNAL(sub_scan_progress(int)));
		connect(m_job, SIGNAL(quit()), this, SLOT(quit()));
		QTimer::singleShot(0, m_job, SLOT(go()));
		exec();
		m_job->deleteLater();
		deleteLater();
	}

private:
	SCANNER_JOB_TYPE m_type;
	ScannerJob *m_job;
	QByteArray m_meta;

signals:
	void main_scan_total_steps(int);
	void main_scan_progress(int);
	void sub_scan_total_steps(int);
	void sub_scan_progress(int);
	void scan_message(const QString&);
	void found_address(const QString&, const uint&);
	void found_offset(const QString&, const int&);
};
#endif
