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
            QString token = gdr->get_string_for_key(
                    "ram_guesser/position/token");
            QString needle = gdr->get_string_for_key(
                    "ram_guesser/position/name_male_singluar/value");

            LOGD << "searching for distance between" << token << "and" <<
                    needle;

            /*! this should be set to a big enough range to cover the distance
                between a position token and its male singluar name buffer
            */
            uint bytes_to_backtrack = 0x200;

            // search for the needle, then search backwards for the token
            QVector<uint> needle_hits = m_df->scan_mem(needle.toAscii());
            emit scan_message(tr("Searching for %1").arg(needle));
            emit main_scan_total_steps(needle_hits.size());
            emit main_scan_progress(0);
            int i = 1;
            foreach(uint hit, needle_hits) {
                // all we found was a null terminated c-string, make sure if
                // we back up we find a valid std::string
                uint hit_addr = hit - DFInstance::STRING_BUFFER_OFFSET;
                QString hit_str = m_df->read_string(hit_addr);
                if (hit_str != needle) {
                    continue; // bail out this was just a c-string hit
                }
                LOGD << "found" << hit_str;

                // fetch some raw data in front of our string match, so we can
                // look for the position token
                QByteArray data = m_df->get_data(hit_addr - bytes_to_backtrack,
                                                 bytes_to_backtrack);

                // search for the token c-string in the backtrack data
                int token_offset = data.indexOf(token.toAscii());
                if (token_offset != -1) {
                    // this may only be a c-string hit as well, so we need to
                    // try to make it a reference to a std::string.
                    // The offset we found this at is based on backtrack bytes,
                    // not the needle buffer. So we need to convert it to be a
                    // distance from the TOKEN to the name buffer
                    uint token_addr = hit_addr - bytes_to_backtrack
                                      + token_offset
                                      - DFInstance::STRING_BUFFER_OFFSET;
                    QString token_str = m_df->read_string(token_addr);
                    if (token_str != token) {
                        continue; // bail out this was just a c-string hit
                    }

                    // Hell yes, we now have valid pointers to the std::string
                    // instances for this postion token and name, get the offset
                    uint dist = hit_addr - token_addr;

                    LOGD << "TOKEN:" << hex << token_addr;
                    LOGD << "NAME:" << hex << hit_addr;
                    LOGD << "DISTANCE:" << hex << dist;
                    //LOGD << m_df->pprint(data, 0);
                    emit found_offset("position name offset:",
                                      dist);

                    // here it gets a bit weird, find all pointers to our token
                    // string which should be the position object
                    foreach(uint pos_ptr1, m_df->scan_mem(encode(token_addr))) {
                        // now find pointers to this pointer, which should
                        // hopefully be a vector of positions with our
                        // token_addr as the first entry
                        foreach(uint pos_ptr2,
                                m_df->scan_mem(encode(pos_ptr1))) {
                            emit found_address(tr("Positions Vector"),
                                pos_ptr2 - DFInstance::VECTOR_POINTER_OFFSET);
                            foreach(uint position_addr, m_df->enumerate_vector(pos_ptr2 - DFInstance::VECTOR_POINTER_OFFSET)) {
                                int flags = m_df->read_int(position_addr + 0x20);
                                LOGD << hexify(position_addr) << m_df->read_int(position_addr + 0x1C);
                                LOGD << "POSITION TOKEN:" << m_df->read_string(position_addr) << "BOXES NEEDED:" << m_df->read_int(position_addr + 0x384);
                                LOGD << "NAME:" << m_df->read_string(position_addr + 0xe8) << "FEMALE NAME PLURAL:" << m_df->read_string(position_addr + 0x13C);
                                LOGD << "FLAGS:" << flags << hexify(flags);
                            }
                        }
                    }
                }
                emit main_scan_progress(i++);
            }
            emit quit();
        }
};
#endif // POSITIONVECTORSEARCHJOB_H
