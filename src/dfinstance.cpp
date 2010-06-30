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

#include <QtGui>
#include <QtDebug>
#include "defines.h"
#include "dfinstance.h"
#include "dwarf.h"
#include "utils.h"
#include "gamedatareader.h"
#include "memorylayout.h"
#include "cp437codec.h"
#include "dwarftherapist.h"
#include "memorysegment.h"
#include "truncatingfilelogger.h"

DFInstance::DFInstance(QObject* parent)
    : QObject(parent)
    , m_pid(0)
    , m_memory_correction(0)
    , m_stop_scan(false)
    , m_is_ok(true)
    , m_bytes_scanned(0)
    , m_layout(0)
    , m_attach_count(0)
    , m_heartbeat_timer(new QTimer(this))
    , m_memory_remap_timer(new QTimer(this))
    , m_scan_speed_timer(new QTimer(this))
    , m_dwarf_race_id(0)
{
    connect(m_scan_speed_timer, SIGNAL(timeout()),
            SLOT(calculate_scan_rate()));
    connect(m_memory_remap_timer, SIGNAL(timeout()),
            SLOT(map_virtual_memory()));
    m_memory_remap_timer->start(20000); // 20 seconds
    // let subclasses start the heartbeat timer, since we don't want to be
    // checking before we're connected
    connect(m_heartbeat_timer, SIGNAL(timeout()), SLOT(heartbeat()));

    // We need to scan for memory layout files to get a list of DF versions this
    // DT version can talk to. Start by building up a list of search paths
    QDir working_dir = QDir::current();
    QStringList search_paths;
    search_paths << working_dir.path();
#ifdef Q_WS_WIN
    QString subdir = "windows";
#else
#ifdef Q_WS_X11
    QString subdir = "linux";
#else
#ifdef _OSX
    QString subdir = "osx";
#endif
#endif
#endif
    search_paths << QString("etc/memory_layouts/%1").arg(subdir);

    TRACE << "Searching for MemoryLayout ini files in the following directories";
    foreach(QString path, search_paths) {
        TRACE<< path;
        QDir d(path);
        d.setNameFilters(QStringList() << "*.ini");
        d.setFilter(QDir::NoDotAndDotDot | QDir::Readable | QDir::Files);
        d.setSorting(QDir::Name | QDir::Reversed);
        QFileInfoList files = d.entryInfoList();
        foreach(QFileInfo info, files) {
            MemoryLayout *temp = new MemoryLayout(info.absoluteFilePath());
            if (temp && temp->is_valid()) {
                LOGD << "adding valid layout" << temp->game_version()
                        << temp->checksum();
                m_memory_layouts.insert(temp->checksum().toLower(), temp);
            }
        }
    }
    // if no memory layouts were found that's a critical error
    if (m_memory_layouts.size() < 1) {
        LOGE << "No valid memory layouts found in the following directories..."
                << QDir::searchPaths("memory_layouts");
        qApp->exit(ERROR_NO_VALID_LAYOUTS);
    }
}

DFInstance::~DFInstance() {
    LOGD << "DFInstance baseclass virtual dtor!";
    foreach(MemoryLayout *l, m_memory_layouts) {
        delete(l);
    }
    m_memory_layouts.clear();
}

BYTE DFInstance::read_byte(const VIRTADDR &addr) {
    QByteArray out;
    read_raw(addr, sizeof(BYTE), out);
    return out.at(0);
}

WORD DFInstance::read_word(const VIRTADDR &addr) {
    QByteArray out;
    read_raw(addr, sizeof(WORD), out);
    return decode_word(out);
}

VIRTADDR DFInstance::read_addr(const VIRTADDR &addr) {
    QByteArray out;
    read_raw(addr, sizeof(VIRTADDR), out);
    return decode_dword(out);
}

qint16 DFInstance::read_short(const VIRTADDR &addr) {
    QByteArray out;
    read_raw(addr, sizeof(qint16), out);
    return decode_short(out);
}

qint32 DFInstance::read_int(const VIRTADDR &addr) {
    QByteArray out;
    read_raw(addr, sizeof(qint32), out);
    return decode_int(out);
}

QVector<VIRTADDR> DFInstance::scan_mem(const QByteArray &needle) {
    // progress reporting
    m_scan_speed_timer->start(500);
    m_memory_remap_timer->stop(); // don't remap segments while scanning
    int total_bytes = 0;
    m_bytes_scanned = 0; // for global timings
    int bytes_scanned = 0; // for progress calcs
    foreach(MemorySegment *seg, m_regions) {
        total_bytes += seg->size;
    }
    int report_every_n_bytes = total_bytes / 1000;
    emit scan_total_steps(1000);
    emit scan_progress(0);


    m_stop_scan = false;
    QVector<VIRTADDR> addresses; //! return value
    QByteArrayMatcher matcher(needle);

    int step_size = 0x1000;
    QByteArray buffer(step_size, 0);
    QByteArray back_buffer(step_size * 2, 0);

    QTime timer;
    timer.start();
    attach();
    foreach(MemorySegment *seg, m_regions) {
        int step = step_size;
        int steps = seg->size / step;
        if (seg->size % step)
            steps++;

        for(VIRTADDR ptr = seg->start_addr; ptr < seg->end_addr; ptr += step) {
            step = step_size;
            if (ptr + step > seg->end_addr) {
                step = seg->end_addr - ptr;
            }

            // move the last thing we read to the front of the back_buffer
            back_buffer.replace(0, step, buffer);

            // fill the main read buffer
            int bytes_read = read_raw(ptr, step, buffer);
            if (bytes_read < step && !seg->is_guarded) {
                if (m_layout->is_complete()) {
                    LOGW << "tried to read" << step << "bytes starting at" <<
                            hexify(ptr) << "but only got" << dec << bytes_read;
                }
                continue;
            }
            bytes_scanned += bytes_read;
            m_bytes_scanned += bytes_read;

            // put the main buffer on the end of the back_buffer
            back_buffer.replace(step, step, buffer);

            int idx = -1;
            forever {
                idx = matcher.indexIn(back_buffer, idx+1);
                if (idx == -1) {
                    break;
                } else {
                    VIRTADDR hit = ptr + idx - step;
                    if (!addresses.contains(hit)) {
                        // backbuffer may cause duplicate hits
                        addresses << hit;
                    }
                }
            }

            if (m_stop_scan)
                break;
            emit scan_progress(bytes_scanned / report_every_n_bytes);

        }
        DT->processEvents();
        if (m_stop_scan)
            break;
    }
    detach();
    m_memory_remap_timer->start(20000); // start the remapper again
    LOGD << QString("Scanned %L1MB in %L2ms").arg(bytes_scanned / 1024 * 1024)
            .arg(timer.elapsed());
    return addresses;
}

bool DFInstance::looks_like_vector_of_pointers(const VIRTADDR &addr) {
    int start = read_int(addr + 0x4);
    int end = read_int(addr + 0x8);
    int entries = (end - start) / sizeof(int);
    LOGD << "LOOKS LIKE VECTOR? unverified entries:" << entries;

    return start >=0 &&
           end >=0 &&
           end >= start &&
           (end-start) % 4 == 0 &&
           start % 4 == 0 &&
           end % 4 == 0 &&
           entries < 10000;

}

QVector<Dwarf*> DFInstance::load_dwarves() {
    map_virtual_memory();
    QVector<Dwarf*> dwarves;
    if (!m_is_ok) {
        LOGW << "not connected";
        detach();
        return dwarves;
    }

    // we're connected, make sure we have good addresses
    VIRTADDR creature_vector = m_layout->address("creature_vector");
    creature_vector += m_memory_correction;
    VIRTADDR dwarf_race_index = m_layout->address("dwarf_race_index");
    dwarf_race_index += m_memory_correction;

    if (!is_valid_address(creature_vector)) {
        LOGW << "Active Memory Layout" << m_layout->filename() << "("
                << m_layout->game_version() << ")" << "contains an invalid"
                << "creature_vector address. Either you are scanning a new "
                << "DF version or your config files are corrupted.";
        return dwarves;
    }
    if (!is_valid_address(dwarf_race_index)) {
        LOGW << "Active Memory Layout" << m_layout->filename() << "("
                << m_layout->game_version() << ")" << "contains an invalid"
                << "dwarf_race_index address. Either you are scanning a new "
                << "DF version or your config files are corrupted.";
        return dwarves;
    }

    // both necessary addresses are valid, so let's try to read the creatures
    LOGD << "loading creatures from " << hexify(creature_vector) <<
            hexify(creature_vector - m_memory_correction) << "(UNCORRECTED)";
    LOGD << "dwarf race index" << hexify(dwarf_race_index) <<
            hexify(dwarf_race_index - m_memory_correction) << "(UNCORRECTED)";
    emit progress_message(tr("Loading Dwarves"));

    attach();
    // which race id is dwarven?
    m_dwarf_race_id = read_word(dwarf_race_index);
    LOGD << "dwarf race:" << hexify(m_dwarf_race_id);

    QVector<VIRTADDR> entries = enumerate_vector(creature_vector);
    emit progress_range(0, entries.size()-1);
    TRACE << "FOUND" << entries.size() << "creatures";
    if (!entries.empty()) {
        Dwarf *d = 0;
        int i = 0;
        foreach(VIRTADDR creature_addr, entries) {
            d = Dwarf::get_dwarf(this, creature_addr);
            if (d) {
                dwarves.append(d);
                TRACE << "FOUND DWARF" << hexify(creature_addr)
                        << d->nice_name();
            } else {
                TRACE << "FOUND OTHER CREATURE" << hexify(creature_addr);
            }
            emit progress_value(i++);
        }
    } else {
        // we lost the fort!
        m_is_ok = false;
    }
    detach();
    LOGI << "found" << dwarves.size() << "dwarves out of" << entries.size()
            << "creatures";
    return dwarves;
}

void DFInstance::heartbeat() {
    // simple read attempt that will fail if the DF game isn't running a fort,
    // or isn't running at all
    QVector<VIRTADDR> creatures = enumerate_vector(
            m_layout->address("creature_vector") + m_memory_correction);
    if (creatures.size() < 1) {
        // no game loaded, or process is gone
        emit connection_interrupted();
    }
}

bool DFInstance::is_valid_address(const VIRTADDR &addr) {
    bool valid = false;
    foreach(MemorySegment *seg, m_regions) {
        if (seg->contains(addr)) {
            valid = true;
            break;
        }
    }
    return valid;
}

QByteArray DFInstance::get_data(const VIRTADDR &addr, int size) {
    QByteArray ret_val(size, 0); // 0 filled to proper length
    int bytes_read = read_raw(addr, size, ret_val);
    if (bytes_read != size) {
        ret_val.clear();
    }
    return ret_val;
}

//! ahhh convenience
QString DFInstance::pprint(const VIRTADDR &addr, int size) {
    return pprint(get_data(addr, size), addr);
}

QString DFInstance::pprint(const QByteArray &ba, const VIRTADDR &start_addr) {
    QString out = "    ADDR   | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F | TEXT\n";
    out.append("------------------------------------------------------------------------\n");
    int lines = ba.size() / 16;
    if (ba.size() % 16)
        lines++;
    if (lines < 1)
        lines = 0;

    for(int i = 0; i < lines; ++i) {
        VIRTADDR offset = start_addr + i * 16;
        out.append(hexify(offset));
        out.append(" | ");
        for (int c = 0; c < 16; ++c) {
            out.append(ba.mid(i*16 + c, 1).toHex());
            out.append(" ");
        }
        out.append("| ");
        for (int c = 0; c < 16; ++c) {
            QByteArray tmp = ba.mid(i*16 + c, 1);
            if (tmp.at(0) == 0)
                out.append(".");
            else if (tmp.at(0) <= 126 && tmp.at(0) >= 32)
                out.append(tmp);
            else
                out.append(tmp.toHex());
        }
        //out.append(ba.mid(i*16, 16).toPercentEncoding());
        out.append("\n");
    }
    return out;
}

QVector<VIRTADDR> DFInstance::find_vectors_in_range(const int &max_entries,
                                                const VIRTADDR &start_address,
                                                const int &range_length) {
    QByteArray data = get_data(start_address, range_length);
    QVector<VIRTADDR> vectors;
    VIRTADDR int1 = 0; // holds the start val
    VIRTADDR int2 = 0; // holds the end val

    for (int i = 0; i < range_length; i += 4) {
        memcpy(&int1, data.data() + i, 4);
        memcpy(&int2, data.data() + i + 4, 4);
        if (int2 >= int1 && is_valid_address(int1) && is_valid_address(int2)) {
            int bytes = int2 - int1;
            int entries = bytes / 4;
            if (entries > 0 && entries <= max_entries) {
                VIRTADDR vector_addr = start_address + i - VECTOR_POINTER_OFFSET;
                QVector<VIRTADDR> addrs = enumerate_vector(vector_addr);
                bool all_valid = true;
                foreach(VIRTADDR vec_entry, addrs) {
                    if (!is_valid_address(vec_entry)) {
                        all_valid = false;
                        break;
                    }
                }
                if (all_valid) {
                    vectors << vector_addr;
                }
            }
        }
    }
    return vectors;
}

QVector<VIRTADDR> DFInstance::find_vectors(int num_entries, int fuzz/* =0 */,
                                           int entry_size/* =4 */) {
    /*
    glibc++ does vectors like so...
    |4bytes      | 4bytes    | 4bytes
    START_ADDRESS|END_ADDRESS|END_ALLOCATOR

    MSVC++ does vectors like so...
    | 4bytes     | 4bytes      | 4 bytes   | 4bytes
    ALLOCATOR    |START_ADDRESS|END_ADDRESS|END_ALLOCATOR
    */
    m_stop_scan = false; //! if ever set true, bail from the inner loop
    QVector<VIRTADDR> vectors; //! return value collection of vectors found
    VIRTADDR int1 = 0; // holds the start val
    VIRTADDR int2 = 0; // holds the end val

    // progress reporting
    m_scan_speed_timer->start(500);
    m_memory_remap_timer->stop(); // don't remap segments while scanning
    int total_bytes = 0;
    m_bytes_scanned = 0; // for global timings
    int bytes_scanned = 0; // for progress calcs
    foreach(MemorySegment *seg, m_regions) {
        total_bytes += seg->size;
    }
    int report_every_n_bytes = total_bytes / 1000;
    emit scan_total_steps(1000);
    emit scan_progress(0);

    int scan_step_size = 0x10000;
    QByteArray buffer(scan_step_size, '\0');
    QTime timer;
    timer.start();
    attach();
    foreach(MemorySegment *seg, m_regions) {
        //TRACE << "SCANNING REGION" << hex << seg->start_addr << "-"
        //        << seg->end_addr << "BYTES:" << dec << seg->size;
        if ((int)seg->size <= scan_step_size) {
            scan_step_size = seg->size;
        }
        int scan_steps = seg->size / scan_step_size;
        if (seg->size % scan_step_size) {
            scan_steps++;
        }
        VIRTADDR addr = 0; // the ptr we will read from
        for(int step = 0; step < scan_steps; ++step) {
            addr = seg->start_addr + (scan_step_size * step);
            //LOGD << "starting scan for vectors at" << hex << addr << "step"
            //        << dec << step << "of" << scan_steps;
            int bytes_read = read_raw(addr, scan_step_size, buffer);
            if (bytes_read < scan_step_size) {
                continue;
            }
            for(int offset = 0; offset < scan_step_size; offset += entry_size) {
                int1 = decode_int(buffer.mid(offset, entry_size));
                int2 = decode_int(buffer.mid(offset + entry_size, entry_size));
                if (int1 && int2 && int2 >= int1
                    //&& is_valid_address(int1)
                    //&& is_valid_address(int2)
                    ) {
                    int bytes = int2 - int1;
                    int entries = bytes / entry_size;
                    int diff = entries - num_entries;
                    if (qAbs(diff) <= fuzz) {
                        VIRTADDR vector_addr = addr + offset -
                                               VECTOR_POINTER_OFFSET;
                        QVector<VIRTADDR> addrs = enumerate_vector(vector_addr);
                        diff = addrs.size() - num_entries;
                        if (qAbs(diff) <= fuzz) {
                            vectors << vector_addr;
                        }
                    }
                }
                m_bytes_scanned += entry_size;
                bytes_scanned += entry_size;
                if (m_stop_scan)
                    break;
            }
            emit scan_progress(bytes_scanned / report_every_n_bytes);
            DT->processEvents();
            if (m_stop_scan)
                break;
        }
    }
    detach();
    m_memory_remap_timer->start(20000); // start the remapper again
    m_scan_speed_timer->stop();
    LOGD << QString("Scanned %L1MB in %L2ms").arg(bytes_scanned / 1024 * 1024)
            .arg(timer.elapsed());
    emit scan_progress(100);
    return vectors;
}

MemoryLayout *DFInstance::get_memory_layout(QString checksum) {
    checksum = checksum.toLower();
    LOGD << "DF's checksum is:" << checksum;

    MemoryLayout *ret_val = NULL;
    ret_val = m_memory_layouts.value(checksum, NULL);
    m_is_ok = ret_val != NULL && ret_val->is_valid();
    if (m_is_ok) {
        LOGI << "Detected Dwarf Fortress version"
                << ret_val->game_version() << "using MemoryLayout from"
                << ret_val->filename();
    } else {
        QString supported_vers;
        foreach(QString tmp_checksum, m_memory_layouts.uniqueKeys()) {
            MemoryLayout *l = m_memory_layouts[tmp_checksum];
            supported_vers.append(
                    QString("<li><b>%1</b>(<font color=\"#444444\">%2"
                            "</font>) from <font size=-1>%3</font></li>")
                    .arg(l->game_version())
                    .arg(l->checksum())
                    .arg(l->filename()));
        }

        QMessageBox *mb = new QMessageBox(qApp->activeWindow());
        mb->setIcon(QMessageBox::Critical);
        mb->setWindowTitle(tr("Unidentified Game Version"));
        mb->setText(tr("I'm sorry but I don't know how to talk to this "
            "version of Dwarf Fortress! (checksum:%1)<br><br> <b>Supported "
            "Versions:</b><ul>%2</ul>").arg(checksum).arg(supported_vers));
        mb->setInformativeText(tr("<a href=\"%1\">Click Here to find out "
                                  "more online</a>.")
                               .arg(URL_SUPPORTED_GAME_VERSIONS));

        /*
        mb->setDetailedText(tr("Failed to locate a memory layout file for "
            "Dwarf Fortress exectutable with checksum '%1'").arg(checksum));
        */
        mb->exec();
        LOGE << tr("unable to identify version from checksum:") << checksum;
    }
    return ret_val;
}

void DFInstance::calculate_scan_rate() {
    float rate = (m_bytes_scanned / 1024.0f / 1024.0f) /
                 (m_scan_speed_timer->interval() / 1000.0f);
    QString msg = QString("%L1MB/s").arg(rate);
    emit scan_message(msg);
    m_bytes_scanned = 0;
}
