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
#include "stdstringsearchjob.h"
#include "nullterminatedstringsearchjob.h"
#include "stonevectorsearchjob.h"
#include "vectorsearchjob.h"
#include "dwarfraceindexsearchjob.h"
#include "creaturevectorsearchjob.h"
#include "positionvectorsearchjob.h"
#include "narrowingvectorsearchjob.h"
#include "squadvectorsearchjob.h"

class ScannerThread : public QThread {
    Q_OBJECT
public:
    ScannerThread(QObject *parent = 0)
        : QThread(parent)
        , m_meta(QByteArray())
        , m_result(NULL)
    {}

    void set_job(SCANNER_JOB_TYPE type) {
        m_type = type;
    }
    void set_meta(const QByteArray &meta) {
        m_meta = meta;
    }
    void set_search_vector(const QVector<VIRTADDR> &searchvector) {
        m_searchvector = searchvector;
    }

    void * get_result() {
        return m_result;
    }

    void clear_result() {
        m_result = NULL;
    }

    void run() {
        // Don't forget your 'break' when adding new sections, Trey. You've done
        // it twice now for a total waste of about 50 minutes >:|
        switch (m_type) {
            case FIND_TRANSLATIONS_VECTOR:
                m_job = new TranslationVectorSearchJob;
                break;
            case FIND_STONE_VECTOR:
                m_job = new StoneVectorSearchJob;
                break;
            case FIND_DWARF_RACE_INDEX:
                m_job = new DwarfRaceIndexSearchJob;
                break;
            case FIND_CREATURE_VECTOR:
                m_job = new CreatureVectorSearchJob;
                break;
            case FIND_POSITION_VECTOR:
                m_job = new PositionVectorSearchJob;
                break;
            case FIND_STD_STRING:
                {
                    StdStringSearchJob *job = new StdStringSearchJob;
                    job->set_needle(m_meta);
                    m_job = job;
                }
                break;
            case FIND_NULL_TERMINATED_STRING:
                {
                    // what is this, Java?
                    NullTerminatedStringSearchJob *job = new NullTerminatedStringSearchJob;
                    job->set_needle(m_meta);
                    m_job = job;
                }
                break;
            case FIND_VECTORS_OF_SIZE:
                {
                    VectorSearchJob *job = new VectorSearchJob;
                    job->set_needle(m_meta);
                    m_job = job;
                }
                break;
            case FIND_NARROWING_VECTORS_OF_SIZE:
                {
                    NarrowingVectorSearchJob * job = new NarrowingVectorSearchJob;
                    job->set_needle(m_meta);
                    job->set_search_vector(m_searchvector);
                    m_job = job;
                }
                break;
            case FIND_SQUADS_VECTOR:
                {
                    m_job = new SquadVectorSearchJob;
                    break;
                }
                break;
            default:
                LOGW << "JOB TYPE NOT SET, EXITING THREAD";
                return;
        }
        // forward the status signals
        connect(m_job->df(), SIGNAL(scan_total_steps(int)),
                SIGNAL(sub_scan_total_steps(int)));
        connect(m_job->df(), SIGNAL(scan_progress(int)),
                SIGNAL(sub_scan_progress(int)));
        connect(m_job->df(), SIGNAL(scan_message(const QString&)),
                SIGNAL(scan_message(const QString&)));
        connect(m_job, SIGNAL(scan_message(const QString&)),
                SIGNAL(scan_message(const QString&)));
        connect(m_job, SIGNAL(found_address(const QString&, const quint32&)),
                SIGNAL(found_address(const QString&, const quint32&)));
        connect(m_job, SIGNAL(found_offset(const QString&, const int&)),
                SIGNAL(found_offset(const QString&, const int&)));
        connect(m_job, SIGNAL(main_scan_total_steps(int)),
                SIGNAL(main_scan_total_steps(int)));
        connect(m_job, SIGNAL(main_scan_progress(int)),
                SIGNAL(main_scan_progress(int)));
        connect(m_job, SIGNAL(sub_scan_total_steps(int)),
                SIGNAL(sub_scan_total_steps(int)));
        connect(m_job, SIGNAL(sub_scan_progress(int)),
                SIGNAL(sub_scan_progress(int)));
        connect(m_job, SIGNAL(quit()), this, SLOT(quit()));
        connect(m_job, SIGNAL(got_result(void *)), this, SLOT(set_result(void *)));
        QTimer::singleShot(0, m_job, SLOT(go()));
        exec();
        m_job->deleteLater();
        deleteLater();
    }

private:
    SCANNER_JOB_TYPE m_type;
    ScannerJob *m_job;
    QByteArray m_meta;
    QVector<VIRTADDR> m_searchvector;

protected:
    void * m_result;

protected slots:
    void set_result(void * result) {
        m_result = result;
    }

signals:
    void main_scan_total_steps(int);
    void main_scan_progress(int);
    void sub_scan_total_steps(int);
    void sub_scan_progress(int);
    void scan_message(const QString&);
    void found_address(const QString&, const quint32&);
    void found_offset(const QString&, const int&);
};
#endif
