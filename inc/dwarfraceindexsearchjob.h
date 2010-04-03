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
#ifndef DWARFRACEINDEXSEARCHJOB_H
#define DWARFRACEINDEXSEARCHJOB_H

#include "scannerjob.h"
#include "defines.h"
#include "truncatingfilelogger.h"
#include "gamedatareader.h"
#include "utils.h"

class DwarfRaceIndexSearchJob : public ScannerJob {
    Q_OBJECT
public:
    DwarfRaceIndexSearchJob()
        : ScannerJob(FIND_DWARF_RACE_INDEX)
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

            uint dwarf_race_index = 0; // return value

            GameDataReader *gdr = GameDataReader::ptr();
            //! number of words that should be in a single language
            int expected_val = gdr->get_int_for_key("ram_guesser/"
                                                    "expected_dwarf_race", 10);

            emit main_scan_total_steps(0);
            emit main_scan_progress(-1);

            emit scan_message(tr("Looking for Dwarf Race Index"));
            foreach(uint ptr, m_df->scan_mem(QByteArray("A group of"))) {
                foreach(uint ptr2, m_df->scan_mem(encode(ptr))) {
                    LOGD << "\tPTR" << hex << ptr2 << m_df->read_string(ptr2);
                    QByteArray needle(2, 0);
                    needle[0] = 0x66;
                    needle[1] = 0x39;
                    int offset = m_df->get_data(ptr2, 60).indexOf(needle);
                    if (offset != 1) {
                        LOGD << "\tMATCH! offset" << offset << hex << offset;
                        uint idx_addr = m_df->read_uint(ptr2 + offset + 3);
                        LOGD << "\tREAD ADDR FROM" << hex << ptr2 + offset << "=" << idx_addr;
                        int idx = m_df->read_int(idx_addr);
                        LOGD << "\t\tRACE VALUE" << idx << "HEX" << hex << idx;
                        if (idx == expected_val) {
                            dwarf_race_index = idx_addr;
                            break;
                        }
                    }
                }
            }
            emit found_address("Dwarf Race", dwarf_race_index);
            emit quit();
        }

private:
    QByteArray m_needle;

};
#endif // DWARFRACEINDEXSEARCHJOB_H
