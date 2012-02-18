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

http://www.opensource.org/licenses/mit-license.php
*/

#ifndef VECTORSEARCHJOB_H
#define VECTORSEARCHJOB_H

#include "scannerjob.h"
#include "defines.h"
#include "truncatingfilelogger.h"
#include "utils.h"

struct VectorSearchParams {
    char op;
    uint target_count;
    uint start_addr;
    uint end_addr;
};

class VectorSearchJob : public ScannerJob {
    Q_OBJECT
public:
    VectorSearchJob()
        : ScannerJob(FIND_VECTORS_OF_SIZE)
    {}

    void set_needle(const QByteArray &needle) {
        m_needle = needle;
    }

    public slots:
        void go() {
            if (!m_ok) {
                LOGE << "Scanner Thread couldn't connect to DF!";
                emit quit();
                return;
            }
            LOGD << "Starting Search in Thread" << QThread::currentThreadId();

            emit main_scan_total_steps(0);
            emit main_scan_progress(-1);
            VectorSearchParams *params = (VectorSearchParams *)m_needle.data();
            emit scan_message(tr("Looking for Vectors %1 %2 entries")
                              .arg(params->op).arg(params->target_count));
            QVector<VIRTADDR> vectors = m_df->find_vectors_ext(params->target_count,
                                         params->op, params->start_addr, params->end_addr);
            LOGD << "Search complete, found " << vectors.size() << " vectors.";

            //Only report the first 200 or so vectors, otherwise the UI hangs
            int count = 0;
            foreach(uint addr, vectors) {
                if(count < 200) {
                    emit found_address("vector found at", addr);
                } else {
                    VIRTADDR corrected_addr = addr - m_df->get_memory_correction();
                    LOGD << "Extra vector address found:"
                            << hexify(corrected_addr) << "uncorrected:" << hexify(addr);
                }
                count++;
            }
            emit quit();
        }

private:
    QByteArray m_needle;

};
#endif // VECTORSEARCHJOB_H
