/*
Dwarf Therapist
Copyright (c) 2009,2010 Trey Stout (chmod)

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

#ifndef CREATUREVECTORSEARCHJOB_H
#define CREATUREVECTORSEARCHJOB_H

#include "scannerjob.h"
#include "defines.h"
#include "truncatingfilelogger.h"
#include "gamedatareader.h"
#include "utils.h"

class CreatureVectorSearchJob : public ScannerJob {
    Q_OBJECT
public:
    CreatureVectorSearchJob ()
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

        GameDataReader *gdr = GameDataReader::ptr();
        int dwarf_nickname_offset = gdr->get_int_for_key(
                "ram_guesser/dwarf_nickname_offset", 16);
        QByteArray custom_nickname = gdr->get_string_for_key(
                "ram_guesser/dwarf_nickname").toAscii();
        QByteArray custom_profession = gdr->get_string_for_key(
                "ram_guesser/dwarf_custom_profession").toAscii();

        emit main_scan_total_steps(1);
        emit main_scan_progress(1);

        emit scan_message(tr("Scanning for known nickname"));
        QByteArray needle(custom_nickname);
        foreach(uint nickname_buf, m_df->scan_mem(needle)) {
            LOGD << "FOUND NICKNAME" << hexify(nickname_buf);
            foreach(uint nickname_str, m_df->scan_mem(encode(nickname_buf))) {
                uint possible_addr = nickname_str - dwarf_nickname_offset -
                                     DFInstance::STRING_BUFFER_OFFSET;

                LOGD << "DWARF POINTER SHOULD BE AT:" << hexify(possible_addr);
                foreach(uint dwarf, m_df->scan_mem(encode(possible_addr))) {
                    LOGD << "FOUND DWARF" << hex << dwarf;
                    emit scan_message(tr("Scanning for dwarf vector pointer"));
                    // since this is the first dwarf, it should also be the vector
                    foreach(uint vector_ptr, m_df->scan_mem(encode(dwarf))) {
                        uint creature_vec = vector_ptr -
                                            DFInstance::VECTOR_POINTER_OFFSET;
                        emit found_address("creature_vector", creature_vec);
                        LOGD << "FOUND CREATURE VECTOR" << hex << creature_vec;
                    }
                }
            }
        }

        /*
        QByteArray skillpattern_miner = encode_skillpattern(0, 3340, 4);
        QByteArray skillpattern_metalsmith = encode_skillpattern(29, 0, 2);
        QByteArray skillpattern_swordsman = encode_skillpattern(40, 462, 3);
        QByteArray skillpattern_pump_operator = encode_skillpattern(65, 462, 1);

        needle = custom_profession;
        foreach(int prof, scan_mem_find_all(needle, dwarf, dwarf + 0x1000)) {
                qDebug() << "Custom Profession Offset" << prof - dwarf - 4;
        }

        return creature_vector_address;

        QVector<QByteArray> patterns;
        patterns.append(skillpattern_miner);
        patterns.append(skillpattern_metalsmith);
        patterns.append(skillpattern_swordsman);
        patterns.append(skillpattern_pump_operator);

        QVector< QVector<int> > addresses;
        addresses.reserve(patterns.size());
        for (int i = 0; i < 4; ++i) {
                QVector<int> lst;
                emit scan_message("scanning for skill pattern" + QString::number(i));
                foreach(int skill, scan_mem_find_all(patterns[i], 0, m_memory_size + m_base_addr - m_memory_correction)) {
                        qDebug() << "FOUND SKILL PATTERN" << i << by_char(patterns[i]) << "AT" << hex << skill;
                        lst.append(skill);
                }
                addresses.append(lst);
                break;
        }
        */

        emit quit();
    }
};
#endif // CREATUREVECTORSEARCHJOB_H
