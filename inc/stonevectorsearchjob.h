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
#ifndef STONE_VECTOR_SEARCH_JOB_H
#define STONE_VECTOR_SEARCH_JOB_H

#include "scannerjob.h"
#include "truncatingfilelogger.h"
#include "gamedatareader.h"
#include "utils.h"

class StoneVectorSearchJob : public ScannerJob {
    Q_OBJECT
public:
    StoneVectorSearchJob()
        : ScannerJob(FIND_TRANSLATIONS_VECTOR)
    {}
    public slots:
        void go() {
            if (!m_ok) {
                LOGE << "Scanner Thread couldn't connect to DF!";
                emit quit();
                return;
            }
            LOGD << "Starting Search in Thread" << QThread::currentThreadId();

            emit main_scan_total_steps(1);
            emit main_scan_progress(0);

            GameDataReader *gdr = GameDataReader::ptr();
            //! first word in the stone vector
            int target_stones = gdr->get_int_for_key("ram_guesser/total_stones", 10);
            QString first_stone = gdr->get_string_for_key("ram_guesser/first_stone");

            emit scan_message(tr("Looking for vectors with %1 entries").arg(target_stones));
            QVector<uint> vecs = m_df->find_vectors(target_stones);
            emit main_scan_total_steps(vecs.size());
            emit scan_message(tr("Looking for a correctly sized vector where the first entry is %1").arg(first_stone));
            int count = 0;
            foreach(uint vec_addr, vecs) {
                if (m_df->looks_like_vector_of_pointers(vec_addr)) {
                    foreach(uint entry, m_df->enumerate_vector(vec_addr)) {
                        if (m_df->is_valid_address(entry)) {
                            QString first_entry = m_df->read_string(entry);
                            if (first_entry == first_stone) {
                                emit found_address("stone_vector", vec_addr);
                            }
                        }
                        break;
                    }
                }
                emit main_scan_progress(++count);
            }
            emit quit();
        }
};
#endif
