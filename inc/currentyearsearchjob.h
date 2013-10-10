/*
Dwarf Therapist
Copyright (c) 2012 Justin Ehlert (DwarfEngineer)

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
#ifndef CURRENTYEARSEARCHJOB_H
#define CURRENTYEARSEARCHJOB_H

#include "scannerjob.h"
#include "defines.h"
#include "truncatingfilelogger.h"
#include "gamedatareader.h"
#include "memorylayout.h"
#include "utils.h"

class CurrentYearSearchJob : public ScannerJob {
    Q_OBJECT
public:
    CurrentYearSearchJob()
        : ScannerJob(FIND_CURRENT_YEAR)
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
        int expected_val = gdr->get_int_for_key("ram_guesser/"
                                                "expected_current_year", 10);

        emit main_scan_total_steps(0);
        emit main_scan_progress(-1);

        // It's been observed that the current year value is pretty isolated,
        // searching for [0, year, 0] should yield few positives
        LOGD << "Looking for " << expected_val;
        emit scan_message(tr("Looking for Current Year"));
        QByteArray search;
        int tmp = 0;
        search.append((char*)&expected_val, sizeof(int));
        search.append((char*)&tmp, sizeof(int));

        // Determine the end address, it should be no higher than the dwarf_race_index
        VIRTADDR end_addr = 0;
        if( m_df->memory_layout() ) {
            end_addr = m_df->memory_layout()->address("dwarf_race_index");
        }
        if( end_addr == 0 ) {
            end_addr = 0x02000000;
        }
        end_addr += m_df->get_memory_correction();

        uint current_year = 0;
        LOGD << "Searching for current year, up to " << hex << end_addr;
        foreach(uint ptr, m_df->scan_mem(search, m_df->get_memory_correction(), end_addr)) {
            LOGD << "\tCurrent Year PTR" << hex << ptr;
            current_year = ptr;
        }
        if( current_year ) {
            // We'll just report the last address. It should almost always be the last address found
            // if we have dwarf_race_index
            emit found_address("current year", current_year);
        }
        emit quit();
    }
};
#endif
