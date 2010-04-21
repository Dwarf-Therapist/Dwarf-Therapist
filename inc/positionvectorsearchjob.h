/*
Dwarf Therapist
Copyright (c) 2010 Trey Stout (chmod)

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
#ifndef POSITIONVECTORSEARCHJOB_H
#define POSITIONVECTORSEARCHJOB_H

#include "scannerjob.h"
#include "defines.h"
#include "truncatingfilelogger.h"
#include "gamedatareader.h"
#include "utils.h"

class PositionVectorSearchJob : public ScannerJob {
    Q_OBJECT
public:
    PositionVectorSearchJob ()
        : ScannerJob(FIND_POSITION_VECTOR)
    {}

    public slots:
        void go() {
            if (!m_ok) {
                LOGE << "Scanner Thread couldn't connect to DF!";
                emit quit();
                return;
            }
            LOGD << "Starting Search in Thread" << QThread::currentThreadId();

            GameDataReader *gdr = GameDataReader::ptr();
            //! number of words that should be in a single language
            int num_positions = gdr->get_int_for_key("ram_guesser/"
                                                     "total_positions", 10);
            QString position = gdr->get_string_for_key("ram_guesser/"
                                                       "position_name");
            uint offset = gdr->get_int_for_key("ram_guesser/"
                                               "position_name_offset", 16);

            emit main_scan_total_steps(1);
            emit main_scan_progress(0);
            LOGD << "scanning for" << position.toAscii().toHex();

            emit scan_message(tr("Looking for Positions Vector (%1 entries)")
                              .arg(num_positions));
            QVector<uint> vectors = m_df->find_vectors(num_positions, 5, 4);
            emit main_scan_total_steps(vectors.size());
            for (int i = 0; i < vectors.size(); ++i) {
                uint vec = vectors.at(i);
                QVector<uint> entries = m_df->enumerate_vector(vec);
                LOGD << "looking at" << hex << vec << "entries:" << dec << entries.size();
                emit main_scan_progress(i);
                emit sub_scan_total_steps(entries.size());
                for (int j = 0; j < entries.size(); ++j) {
                    uint entry = entries.at(j);
                    QByteArray data = m_df->get_data(entry, 0x500);
                    LOGD << "got data" << data.size() << "bytes";
                    int offset = data.indexOf(position.toLocal8Bit(), 0);
                    if (offset != -1) {
                        LOGD << "WOOT!" << entry;
                        emit found_address(tr("Vector that contains %1")
                                           .arg(position), vec);
                        emit sub_scan_progress(entries.size());
                        break;
                    }
                    emit sub_scan_progress(j);
                }
            }
            emit quit();
        }
};
#endif // POSITIONVECTORSEARCHJOB_H
