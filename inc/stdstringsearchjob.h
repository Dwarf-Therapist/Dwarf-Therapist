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

#ifndef STDSTRINGSEARCHJOB_H
#define STDSTRINGSEARCHJOB_H

#include "scannerjob.h"
#include "defines.h"
#include "truncatingfilelogger.h"
#include "utils.h"

class StdStringSearchJob : public ScannerJob {
    Q_OBJECT
public:
    StdStringSearchJob ()
        : ScannerJob(FIND_STD_STRING)
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


            emit scan_message(tr("Looking for %1 (%2)").arg(QString(m_needle))
                              .arg(QString(m_needle.toHex())));

            /* Finding std::strings is sort of a bitch, as the different
               compilers set up the structure differently. Windows will put
               the string bugger directly in the string object if it is less
               than 16 characters long, otherwise we have some pointer
               following to do
            */
            bool follow_ptr = true; // most of the time
#ifdef Q_WS_WIN
            // windows does some hackery on strings less than 16 chars long
            if (m_needle.size() < 16) {
                follow_ptr = false; // the string buffer will be in the string
            }
#endif

            QVector<VIRTADDR> str_buf_hits = m_df->scan_mem(m_needle);
            emit main_scan_total_steps(str_buf_hits.size());
            for(int i = 0; i < str_buf_hits.size(); ++i) {
                emit main_scan_progress(i);
                VIRTADDR str_buf = str_buf_hits.at(i);
                // found the buffer
                LOGD << "found buffer at:" << hexify(str_buf);
                LOGD << "encoded:" << encode(str_buf).toHex();
                if (follow_ptr) {
                    foreach (VIRTADDR ptr, m_df->scan_mem(encode(str_buf))) {
                        // found a ptr to the string buffer, so back up to what
                        // should be the start of the std::string object
                        VIRTADDR str_ptr = ptr - m_df->STRING_BUFFER_OFFSET;
                        QString real_str = m_df->read_string(str_ptr);
                        LOGD << "found real str:" << real_str;
                        if (real_str.toAscii() == m_needle) {
                            emit found_offset(real_str, str_ptr);
                        } else {
                            emit found_offset(QString("Incomplete? '%1'")
                                              .arg(m_df->read_string(str_ptr)),
                                              str_ptr);
                        }
                    }
                } else {
                    emit found_address(m_needle,
                                       str_buf - m_df->STRING_BUFFER_OFFSET);
                }
            }
            emit quit();
        }

private:
    QByteArray m_needle;

};
#endif // STDSTRINGSEARCHJOB_H
