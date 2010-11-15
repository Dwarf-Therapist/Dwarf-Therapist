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
#ifndef SCANNER_JOB_H
#define SCANNER_JOB_H

#include <QObject>
#include "dfinstance.h"

typedef enum {
    FIND_TRANSLATIONS_VECTOR,
    FIND_STONE_VECTOR,
    FIND_METAL_VECTOR,
    FIND_STD_STRING,
    FIND_NULL_TERMINATED_STRING,
    FIND_VECTORS_OF_SIZE,
    FIND_DWARF_RACE_INDEX,
    FIND_CREATURE_VECTOR,
    FIND_POSITION_VECTOR,
    FIND_NARROWING_VECTORS_OF_SIZE
} SCANNER_JOB_TYPE;


class ScannerJob : public QObject {
    Q_OBJECT
public:
    ScannerJob(SCANNER_JOB_TYPE job_type);
    virtual ~ScannerJob();
    SCANNER_JOB_TYPE job_type();
    DFInstance *df();
    static QString m_layout_override_checksum;

protected:
    SCANNER_JOB_TYPE m_job_type;
    bool m_ok;
    DFInstance *m_df;
    bool get_DFInstance();

signals:
    void main_scan_total_steps(int);
    void main_scan_progress(int);
    void sub_scan_total_steps(int);
    void sub_scan_progress(int);
    void found_address(const QString&, const quint32&);
    void found_offset(const QString&, const int&);
    void scan_message(const QString&);
    void got_result(void *);
    void quit();
};
#endif
