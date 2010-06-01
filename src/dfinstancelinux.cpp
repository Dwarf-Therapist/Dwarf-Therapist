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
#include <sys/ptrace.h>
#include <errno.h>
#include <wait.h>

#include "dfinstance.h"
#include "dfinstancelinux.h"
#include "defines.h"
#include "dwarf.h"
#include "utils.h"
#include "gamedatareader.h"
#include "memorylayout.h"
#include "cp437codec.h"
#include "memorysegment.h"
#include "truncatingfilelogger.h"

DFInstanceLinux::DFInstanceLinux(QObject* parent)
    : DFInstance(parent)
{
}

DFInstanceLinux::~DFInstanceLinux() {
    if (m_attach_count > 0) {
        detach();
    }
}

QVector<uint> DFInstanceLinux::enumerate_vector(const uint &addr) {
    QVector<uint> addrs;
    if (!addr)
        return addrs;

    attach();
    VIRTADDR start = read_dword(addr);
    VIRTADDR end = read_dword(addr + 4);
    uint bytes = end - start;
    int entries = bytes / 4;
    TRACE << "enumerating vector at" << hex << addr << "START" << start
        << "END" << end << "UNVERIFIED ENTRIES" << dec << entries;
    VIRTADDR tmp_addr = 0;

#ifdef _DEBUG
    if (m_layout->is_complete()) {
        Q_ASSERT_X(start > 0, "enumerate_vector", "start pointer must be larger than 0");
        Q_ASSERT_X(end > 0, "enumerate_vector", "End must be larger than start!");
        Q_ASSERT_X(start % 4 == 0, "enumerate_vector", "Start must be divisible by 4");
        Q_ASSERT_X(end % 4 == 0, "enumerate_vector", "End must be divisible by 4");
        Q_ASSERT_X(end >= start, "enumerate_vector", "End must be >= start!");
        Q_ASSERT_X((end - start) % 4 == 0, "enumerate_vector", "end - start must be divisible by 4");
    } else {
        // when testing it's usually pretty bad to find a vector with more
        // than 5000 entries... so throw
        Q_ASSERT_X(entries < 5000, "enumerate_vector", "more than 5000 entires");
    }
#endif
    QByteArray data(bytes, 0);
    int bytes_read = read_raw(start, bytes, data);
    if (bytes_read != bytes && m_layout->is_complete()) {
        LOGW << "Tried to read" << bytes << "bytes but only got"
                << bytes_read;
        detach();
        return addrs;
    }
    for(uint i = 0; i < bytes; i += 4) {
        tmp_addr = decode_dword(data.mid(i, 4));
        if (m_layout->is_complete()) {
            if (is_valid_address(tmp_addr))
                addrs << tmp_addr;
        } else {
            addrs << tmp_addr;
        }
    }
    detach();
    return addrs;
}

uint DFInstanceLinux::calculate_checksum() {
    // ELF binaries don't seem to store a linker timestamp, so just MD5 the file.
    uint md5 = 0; // we're going to throw away a lot of this checksum we just need 4bytes worth
    QProcess *proc = new QProcess(this);
    QStringList args;
    args << "md5sum";
    args << QString("/proc/%1/exe").arg(m_pid);
    proc->start("/usr/bin/env", args);
    if (proc->waitForReadyRead(3000)) {
        QByteArray out = proc->readAll();
        QString str_md5(out);
        QStringList chunks = str_md5.split(" ");
        str_md5 = chunks[0];
        bool ok;
        md5 = str_md5.mid(0, 8).toUInt(&ok,16); // use the first 8 bytes
        TRACE << "GOT MD5:" << md5;
    }
    return md5;
}


QString DFInstanceLinux::read_string(const VIRTADDR &addr) {
    VIRTADDR buffer_addr = read_dword(addr);
    int upper_size = 256;
    QByteArray buf(upper_size, 0);
    read_raw(buffer_addr, upper_size, buf);

    QString ret_val(buf);
    CP437Codec *codec = new CP437Codec;
    ret_val = codec->toUnicode(ret_val.toAscii());
    return ret_val;
}

uint DFInstanceLinux::write_string(const uint &addr, const QString &str) {
    Q_UNUSED(addr);
    Q_UNUSED(str);
    return 0;
}

uint DFInstanceLinux::write_int(const uint &addr, const int &val) {
    return write_raw(addr, sizeof(int), (void*)&val);
}

bool DFInstanceLinux::attach() {
    TRACE << "STARTING ATTACH" << m_attach_count;
    if (is_attached()) {
        m_attach_count++;
        TRACE << "ALREADY ATTACHED SKIPPING..." << m_attach_count;
        return true;
    }

    if (ptrace(PTRACE_ATTACH, m_pid, 0, 0) == -1) { // unable to attach
        perror("ptrace attach");
        LOGE << "Could not attach to PID" << m_pid;
        return false;
    }
    int status;
    while(true) {
        TRACE << "waiting for proc to stop";
        pid_t w = waitpid(m_pid, &status, 0);
        if (w == -1) {
            // child died
            perror("wait inside attach()");
            LOGE << "child died?";
            exit(-1);
        }
        if (WIFSTOPPED(status)) {
            break;
        }
        TRACE << "waitpid returned but child wasn't stopped, keep waiting...";
    }
    m_attach_count++;
    TRACE << "FINISHED ATTACH" << m_attach_count;
    return m_attach_count > 0;
}

bool DFInstanceLinux::detach() {
    TRACE << "STARTING DETACH" << m_attach_count;
    m_attach_count--;
    if (m_attach_count > 0) {
        TRACE << "NO NEED TO DETACH SKIPPING..." << m_attach_count;
        return true;
    }

    ptrace(PTRACE_DETACH, m_pid, 0, 0);
    TRACE << "FINISHED DETACH" << m_attach_count;
    return m_attach_count > 0;
}

int DFInstanceLinux::read_raw(const VIRTADDR &addr, int bytes, QByteArray &buffer) {
    // try to attach, will be ignored if we're already attached
    attach();

    // open the memory virtual file for this proc (can only read once
    // attached and child is stopped
    QFile mem_file(QString("/proc/%1/mem").arg(m_pid));
    if (!mem_file.open(QIODevice::ReadOnly)) {
        LOGE << "Unable to open" << mem_file.fileName();
        detach();
        return 0;
    }
    int bytes_read = 0; // tracks how much we've read of what was asked for
    int step_size = 0x1000; // how many bytes to read each step
    QByteArray chunk(step_size, 0); // our temporary memory container
    buffer.fill(0, bytes); // zero our buffer
    int steps = bytes / step_size;
    if (bytes % step_size)
        steps++;

    for(VIRTADDR ptr = addr; ptr < addr+ bytes; ptr += step_size) {
        if (ptr+step_size > addr + bytes)
            step_size = addr + bytes - ptr;
        mem_file.seek(ptr);
        chunk = mem_file.read(step_size);
        buffer.replace(bytes_read, chunk.size(), chunk);
        bytes_read += chunk.size();
    }
    mem_file.close();
    detach();
    return bytes_read;
}

/*
uint DFInstanceLinux::read_raw(const uint &addr, const uint &bytes, void *buffer) {
    // try to attach, will be ignored if we're already attached
    attach();

    // open the memory virtual file for this proc (can only read once
    // attached and child is stopped
    QFile mem_file(QString("/proc/%1/mem").arg(m_pid));
    if (!mem_file.open(QIODevice::ReadOnly)) {
        LOGE << "Unable to open" << mem_file.fileName();
        detach();
        return 0;
    }

    uint bytes_read = 0; //! tracks how much we've read of what was asked for
    QByteArray data; // out final memory container
    ushort failures = 0; // how many read failures have occurred
    ushort failure_max = 0; // how many times will we retry after a read failure

    // keep going until we've read everything we were asked for
    while (bytes_read < bytes) {
        // mem behaves like a normal file, so seek to the next unread offset
        mem_file.seek(addr + bytes_read);
        // read the remainder
        QByteArray tmp = mem_file.read(bytes - data.size());
        bytes_read += tmp.size();
        // push the recently read chunk into data
        data.append(tmp);
        if (bytes_read == 0) {
            // failed to read
            failures++;
            if (m_layout->is_complete()) {
                LOGW << "read 0 bytes from" << hex << addr + bytes_read;
            }
            if (failures >= failure_max)
                break;
        }
        //qDebug() << "bytes_read:" << bytes_read;
    }
    // copy the read data into the provided buffer
    memcpy(buffer, data.data(), data.size());
    // don't leave the mem file open
    mem_file.close();

    // attempt to detach, will be ignored if we're several layers into an attach chain
    detach();
    // tell the caller their buffer is ready, with fresh bits from the oven :)
    return bytes_read;
}
*/

uint DFInstanceLinux::write_raw(const uint &addr, const uint &bytes, void *buffer) {
    // try to attach, will be ignored if we're already attached
    attach();

    /* Since most kernels won't let us write to /proc/<pid>/mem, we have to poke
     * out data in 4 bytes at a time. Good thing we read way more than we write
     */

    uint bytes_written = 0; // keep track of how much we've written
    uint steps = bytes / 4;
    if (bytes % 4)
        steps++;
    LOGD << "WRITE_RAW: WILL WRITE" << bytes << "bytes over" << steps << "steps";

    // we want to make sure that given the case where (bytes % 4 != 0) we don't
    // clobber data past where we meant to write. So we're first going to read
    // the existing data as it is, and then write the changes over the existing
    // data in the buffer first, then write the buffer 4 bytes at a time to the
    // process. This should ensure no clobbering of data.
    QByteArray existing_data(steps * 4, 0);
    read_raw(addr, (steps * 4), existing_data);
    LOGD << "WRITE_RAW: EXISTING OLD DATA     " << existing_data.toHex();

    // ok we have our insurance in place, now write our new junk to the buffer
    memcpy(existing_data.data(), buffer, bytes);
    QByteArray tmp2(existing_data);
    LOGD << "WRITE_RAW: EXISTING WITH NEW DATA" << tmp2.toHex();

    // ok, now our to be written data is in part or all of the exiting data buffer
    long tmp_data;
    for (uint i = 0; i < steps; ++i) {
        int offset = i * 4;
        // for each step write a single word to the child
        memcpy(&tmp_data, existing_data.mid(offset, 4).data(), 4);
        QByteArray tmp_data_str((char*)&tmp_data, 4);
        LOGD << "WRITE_RAW:" << hex << addr + offset << "HEX" << tmp_data_str.toHex();
        if (ptrace(PTRACE_POKEDATA, m_pid, addr + offset, tmp_data) != 0) {
            perror("write word");
            break;
        } else {
            bytes_written += 4;
        }
        long written = ptrace(PTRACE_PEEKDATA, m_pid, addr + offset, 0);
        QByteArray foo((char*)&written, 4);
        LOGD << "WRITE_RAW: WE APPEAR TO HAVE WRITTEN" << foo.toHex();
    }
    // attempt to detach, will be ignored if we're several layers into an attach chain
    detach();
    // tell the caller how many bytes we wrote
    return bytes_written;
}

bool DFInstanceLinux::find_running_copy() {
    // find PID of DF
    TRACE << "attempting to find running copy of DF by executable name";
    QProcess *proc = new QProcess(this);
    QStringList args;
    args << "dwarfort.exe";
    proc->start("pidof", args);
    proc->waitForFinished(1000);
    if (proc->exitCode() == 0) {//found it
        QByteArray out = proc->readAllStandardOutput();
        QString str_pid(out);
        m_pid = str_pid.toInt();
        m_memory_file.setFileName(QString("/proc/%1/mem").arg(m_pid));
        TRACE << "FOUND PID:" << m_pid;
    } else {
        QMessageBox::warning(0, tr("Warning"),
            tr("Unable to locate a running copy of Dwarf "
            "Fortress, are you sure it's running?"));
        LOGW << "can't find running copy";
        m_is_ok = false;
        return m_is_ok;
    }

    map_virtual_memory();

    //qDebug() << "LOWEST ADDR:" << hex << lowest_addr;


    //DUMP LIST OF MEMORY RANGES
    /*
    QPair<uint, uint> tmp_pair;
    foreach(tmp_pair, m_regions) {
        LOGD << "RANGE start:" << hex << tmp_pair.first << "end:" << tmp_pair.second;
    }*/

    VIRTADDR m_base_addr = read_dword(m_lowest_address + 0x18);
    LOGD << "base_addr:" << m_base_addr << "HEX" << hex << m_base_addr;
    m_is_ok = m_base_addr > 0;

    uint checksum = calculate_checksum();
    LOGD << "DF's checksum is" << hexify(checksum);
    if (m_is_ok) {
        m_layout = get_memory_layout(hexify(checksum).toLower());
    }
    return m_is_ok;
}

void DFInstanceLinux::map_virtual_memory() {
    // destroy existing segments
    foreach(MemorySegment *seg, m_regions) {
        delete(seg);
    }
    m_regions.clear();

    if (!m_is_ok)
        return;

    // scan the maps to populate known regions of memory
    QFile f(QString("/proc/%1/maps").arg(m_pid));
    if (!f.open(QIODevice::ReadOnly)) {
        LOGE << "Unable to open" << f.fileName();
        return;
    }
    TRACE << "opened" << f.fileName();
    QByteArray line;
    m_lowest_address = 0xFFFFFFFF;
    m_highest_address = 0;
    uint start_addr = 0;
    uint end_addr = 0;
    bool ok;

    QRegExp rx("^([a-f\\d]+)-([a-f\\d]+)\\s([rwxsp-]{4})\\s+[\\d\\w]{8}\\s+[\\d\\w]{2}:[\\d\\w]{2}\\s+(\\d+)\\s*(.+)\\s*$");
    do {
        line = f.readLine();
        // parse the first line to see find the base
        if (rx.indexIn(line) != -1) {
            //LOGD << "RANGE" << rx.cap(1) << "-" << rx.cap(2) << "PERMS" <<
            //        rx.cap(3) << "INODE" << rx.cap(4) << "PATH" << rx.cap(5);
            start_addr = rx.cap(1).toUInt(&ok, 16);
            end_addr = rx.cap(2).toUInt(&ok, 16);
            QString perms = rx.cap(3).trimmed();
            int inode = rx.cap(4).toInt();
            QString path = rx.cap(5).trimmed();

            //LOGD << "RANGE" << hex << start_addr << "-" << end_addr << perms
            //        << inode << "PATH >" << path << "<";
            bool keep_it = false;
            if (path.contains("[heap]") || path.contains("[stack]") || path.contains("[vdso]"))  {
                keep_it = true;
            } else if (perms.contains("r") && inode && path == QFile::symLinkTarget(QString("/proc/%1/exe").arg(m_pid))) {
                keep_it = true;
            } else {
                keep_it = path.isEmpty();
            }
            // uncomment to search HEAP only
            //keep_it = path.contains("[heap]");
            //keep_it = true;

            if (keep_it && end_addr > start_addr) {
                MemorySegment *segment = new MemorySegment(path, start_addr, end_addr);
                TRACE << "keeping" << segment->to_string();
                m_regions << segment;
                if (start_addr < m_lowest_address)
                    m_lowest_address = start_addr;
                else if (end_addr > m_highest_address)
                    m_highest_address = end_addr;
            }
            if (path.contains("[heap]")) {
                //LOGD << "setting heap start address at" << hex << start_addr;
                m_heap_start_address = start_addr;
            }
        }
    } while (!line.isEmpty());
    f.close();

}
