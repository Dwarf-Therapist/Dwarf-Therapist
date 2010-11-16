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
#ifndef SQUADVECTORSEARCHJOB_H
#define SQUADVECTORSEARCHJOB_H

#include "scannerjob.h"
#include "defines.h"
#include "truncatingfilelogger.h"
#include "gamedatareader.h"
#include "memorylayout.h"
#include "utils.h"

class SquadVectorSearchJob : public ScannerJob {
    Q_OBJECT
public:
    SquadVectorSearchJob()
        : ScannerJob(FIND_SQUADS_VECTOR)
    {}
public slots:
    void go() {
        if (!m_ok) {
            LOGE << "Scanner Thread couldn't connect to DF!";
            emit quit();
            return;
        }
        LOGD << "Starting Search in Thread" << QThread::currentThreadId();

        MemoryLayout * mem = m_df->memory_layout();

        emit main_scan_total_steps(4);
        emit main_scan_progress(0);

        // we'll fill these in as we find them
        QVector<uint> member_vectors; // as we find candidate pointers we'll put them in here

        //Search for vectors with 10 members
        emit scan_message(tr("Scanning for vectors with 10 entries"));
        QVector<VIRTADDR> vectors = m_df->find_vectors(10);
        LOGD << "Found" << vectors.size() << "vectors.";

        emit main_scan_progress(1);
        emit sub_scan_total_steps(vectors.size());

        int step = 0;
        foreach(uint addr, vectors) {
            emit sub_scan_progress(step++);

            int idx = 0;
            bool valid = true;
            foreach(uint vec_addr, m_df->enumerate_vector(addr)) {
                //Candidates should have a non-negative first value, and -1
                //for the remaining values
                int value = m_df->read_int(vec_addr);

                //int masked = 0xffff0000 & value;
                //if(masked != 0)
                //    valid = false;

                if((idx == 0 && value < 0) || (idx > 0 && value != -1)) {
                    valid = false;
                    break;
                }

                idx++;
            }
            if(valid) {
                LOGD << "Found members vector" << hex << addr;
                member_vectors << addr;
            }
        }

        LOGD << "Narrowed down to" << member_vectors.size() << "vectors.";

        if(member_vectors.size() != 1) {
            LOGW << "Warning: found" << member_vectors.size() << "potential member vectors!";
            if(member_vectors.size() == 0) {
                emit quit();
                return;
            }
        }

        //find the squad ID
        VIRTADDR squad_addr = 0;
        foreach(VIRTADDR mem_addr, member_vectors) {
            squad_addr = (mem_addr - mem->squad_offset("members"));
            LOGD << "Squad address" << hex << squad_addr;

            int squad_id = m_df->read_int(squad_addr + mem->squad_offset("id"));
            if(squad_id != 0) {
                LOGD << "Warning: squad_id is not 0, ignoring.";
            } else {
                break;
            }
        }

        //Find the vector entry pointing to this squad
        emit scan_message(tr("Locating references to this squad"));
        emit main_scan_progress(2);

        QVector<VIRTADDR> squad_ref_ptrs = m_df->scan_mem(encode(squad_addr));
        if(squad_ref_ptrs.size() != 1) {
            LOGW << "Warning: found no references to the squad";
            if(squad_ref_ptrs.size() == 0) {
                emit quit();
                return;
            }
        }

        //finally, find the vector that the entry belongs to
        emit scan_message(tr("Locating the squad vector"));
        emit main_scan_progress(3);
        QVector<VIRTADDR> squad_vectors = m_df->scan_mem(encode(squad_ref_ptrs.front()));

        emit main_scan_progress(4);
        foreach(VIRTADDR squad_vec, squad_vectors) {
            VIRTADDR addr = squad_vec - DFInstance::VECTOR_POINTER_OFFSET;

            VIRTADDR corrected_addr = addr - m_df->get_memory_correction();
            LOGD << "Squad vector address found:"
                    << hexify(corrected_addr) << "uncorrected:" << hexify(addr);

            emit found_address("squad vector", addr);
        }

        emit quit();
    }
};
#endif
