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

DFInstanceLinux::DFInstanceLinux(QObject* parent)
	: DFInstance(parent)	
{   
}

DFInstanceLinux::~DFInstanceLinux() {
    //ptrace(PTRACE_DETACH, m_pid, 0, 0);
    //perror("detach");
}

QVector<uint> DFInstanceLinux::enumerate_vector(const uint &addr) {
    attach();

    QVector<uint> addrs;
    uint start = read_uint(addr);
    uint end = read_uint(addr + 4);
    uint bytes = end - start;
    int entries = bytes / 4;
	TRACE << "enumerating vector at" << hex << addr << "START" << start << "END" << end << "UNVERIFIED ENTRIES" << dec << entries;
    uint tmp_addr = 0;

    //Q_ASSERT_X(start > 0, "enumerate_vector", "start pointer must be larger than 0");
    //Q_ASSERT_X(end > 0, "enumerate_vector", "End must be larger than start!");
    //Q_ASSERT_X(start % 4 == 0, "enumerate_vector", "Start must be divisible by 4");
    //Q_ASSERT_X(end % 4 == 0, "enumerate_vector", "End must be divisible by 4");
    //Q_ASSERT_X(end >= start, "enumerate_vector", "End must be >= start!");
    //Q_ASSERT_X((end - start) % 4 == 0, "enumerate_vector", "end - start must be divisible by 4");

    char *stuff = new char[bytes];
	if (stuff == 0) {
        qWarning() << "Unable to allocate char array of size" << bytes;
        detach();
        return addrs;
    }
    uint bytes_read = read_raw(start, bytes, stuff);
    if (bytes_read != bytes) {
        qWarning() << "Tried to read" << bytes << "bytes but only got" << bytes_read;
        detach();
        return addrs;
    }
    for(uint i = 0; i < bytes; i += 4) {
        memcpy(&tmp_addr, stuff + i, 4);
        if (is_valid_address(tmp_addr)) {
            addrs << tmp_addr;
        } else {
            //qWarning() << "bad addr in vector:" << hex << tmp_addr;
        }
    }
    delete[] stuff;
    //qDebug() << "VECTOR at" << hex << addr << "start:" << start << "end:" << end << "(" << dec << entries << "entries) valid entries" << addrs.size();
    Q_ASSERT_X(entries == addrs.size(), "enumerate_vector", "Vector did not contain 100% valid addresses!");
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

bool DFInstanceLinux::is_valid_address(const uint &addr) {
    bool valid = false;
    QPair<uint, uint> region;
    foreach(region, m_regions) {
        if (addr >= region.first && addr <= region.second) {
            valid = true;
            break;
        }
    }
    return valid;
}

uint DFInstanceLinux::read_uint(const uint &addr) {
    uint retval = 0;
    read_raw(addr, sizeof(uint), &retval);
    return retval;
}

int DFInstanceLinux::read_int(const uint &addr) {
    int retval = 0;
    read_raw(addr, sizeof(int), &retval);
    return retval;
}

QString DFInstanceLinux::read_string(const uint &addr) {
    uint buffer_addr = read_uint(addr);
    int upper_size = 1024;
    char *c = new char[upper_size];
    memset(c, 0, upper_size);
    read_raw(buffer_addr, upper_size, c);
	
	CP437Codec *codec = new CP437Codec;
	QString ret_val = codec->toUnicode(c);
	delete[] c;
	return ret_val;
}

uint DFInstanceLinux::write_string(const uint &addr, const QString &str) {
    Q_UNUSED(addr);
    Q_UNUSED(str);
	return 0;
}

short DFInstanceLinux::read_short(const uint &addr) {
    short retval = 0;
    read_raw(addr, sizeof(short), &retval);
    return retval;
}

ushort DFInstanceLinux::read_ushort(const uint &addr) {
    ushort retval = 0;
    read_raw(addr, sizeof(ushort), &retval);
    return retval;
}

uint DFInstanceLinux::write_int(const uint &addr, const int &val) {
    return write_raw(addr, sizeof(int), (void*)&val);
}

char DFInstanceLinux::read_char(const uint &addr) {
	char retval;
	read_raw(addr, sizeof(char), &retval);
	return retval;
}

bool DFInstanceLinux::attach() {
    //TRACE << "STARTING ATTACH" << m_attach_count;
    if (is_attached()) {
        m_attach_count++;
        //TRACE << "ALREADY ATTACHED SKIPPING..." << m_attach_count;
        return true;
    }

    if (ptrace(PTRACE_ATTACH, m_pid, 0, 0) == -1) { // unable to attach
        perror("ptrace attach");
        LOGC << "Could not attach to PID" << m_pid;
        return false;
    }
    int status;
    while(true) {
        //LOGD << "waiting for proc to stop";
        pid_t w = waitpid(m_pid, &status, 0);
        if (w == -1) {
            // child died
            perror("wait inside attach()");
            //exit(-1);
        }
        if (WIFSTOPPED(status)) {
            break;
        }
    }
    m_attach_count++;
    //TRACE << "FINISHED ATTACH" << m_attach_count;
    return m_attach_count > 0;
}

bool DFInstanceLinux::detach() {
    //TRACE << "STARTING DETACH" << m_attach_count;
    m_attach_count--;
    if (m_attach_count > 0) {
        //TRACE << "NO NEED TO DETACH SKIPPING..." << m_attach_count;
        return true;
    }

    ptrace(PTRACE_DETACH, m_pid, 0, 0);
    //TRACE << "FINISHED DETACH" << m_attach_count;
    return m_attach_count > 0;
}

uint DFInstanceLinux::read_raw(const uint &addr, const uint &bytes, void *buffer) {
    // try to attach, will be ignored if we're already attached
    attach();

    // open the memory virtual file for this proc (can only read once 
    // attached and child is stopped
    QFile mem_file(QString("/proc/%1/mem").arg(m_pid));
    if (!mem_file.open(QIODevice::ReadOnly)) {
        LOGC << "Unable to open" << mem_file.fileName();
        detach();
        return 0;
    }

    uint bytes_read = 0; //! tracks how much we've read of what was asked for
    QByteArray data; // out final memory container
    ushort failures = 0; // how many read failures have occurred
    ushort failure_max = 3; // how many times will we retry after a read failure

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
            LOGW << "read 0 bytes from" << hex << addr + bytes_read;
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
    char *existing_data = new char[steps * 4];
    memset(existing_data, 0, steps * 4);
    read_raw(addr, (steps * 4), existing_data);
    QByteArray tmp(existing_data, steps * 4);
    LOGD << "WRITE_RAW: EXISTING OLD DATA     " << tmp.toHex();

    // ok we have our insurance in place, now write our new junk to the buffer
    memcpy(existing_data, buffer, bytes);
    QByteArray tmp2(existing_data, steps * 4);
    LOGD << "WRITE_RAW: EXISTING WITH NEW DATA" << tmp2.toHex();

    // ok, now our to be written data is in part or all of the exiting data buffer
    long tmp_data;
    for (int i = 0; i < steps; ++i) {
        int offset = i * 4;
        // for each step write a single word to the child
        memcpy(&tmp_data, existing_data + offset, 4);
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
    delete[] existing_data;

    // attempt to detach, will be ignored if we're several layers into an attach chain
    detach();
    // tell the caller how many bytes we wrote
    return bytes_written;
}

bool DFInstanceLinux::find_running_copy() {
    m_regions.clear();
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
        TRACE << "FOUND PID:" << m_pid;
    } else {
        m_is_ok = false;
        return false;
    }

    // scan the maps to populate known regions of memory
    QFile f(QString("/proc/%1/maps").arg(m_pid));
    if (!f.open(QIODevice::ReadOnly)) {
        LOGC << "Unable to open" << f.fileName();
        return false;
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
            //qDebug() << "RANGE" << rx.cap(1) << "-" << rx.cap(2) << "PERMS" << rx.cap(3) << "INODE" << rx.cap(4) << "PATH" << rx.cap(5);
            start_addr = rx.cap(1).toUInt(&ok, 16);
            end_addr = rx.cap(2).toUInt(&ok, 16);
            QString perms = rx.cap(3).trimmed();
            int inode = rx.cap(4).toInt();
            QString path = rx.cap(5).trimmed();

            //qDebug() << "RANGE" << hex << start_addr << "-" << end_addr << perms << inode << "PATH >" << path << "<";
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

            if (keep_it) {
                //qDebug() << "KEEPING RANGE" << hex << start_addr << "-" << end_addr << "PATH " << path;
                m_regions << QPair<uint, uint>(start_addr, end_addr);
                if (start_addr < m_lowest_address)
                    m_lowest_address = start_addr;
                else if (end_addr > m_highest_address)
                    m_highest_address = end_addr;
            }
            if (path.contains("[heap]")) {
                LOGD << "setting heap start address at" << hex << start_addr;
                m_heap_start_address = start_addr;
            }
        }
    } while (!line.isEmpty());
    f.close();
    //qDebug() << "LOWEST ADDR:" << hex << lowest_addr;


    //DUMP LIST OF MEMORY RANGES
    /*
    QPair<uint, uint> tmp_pair;
    foreach(tmp_pair, m_regions) {
        LOGD << "RANGE start:" << hex << tmp_pair.first << "end:" << tmp_pair.second;
    }*/


    uint m_base_addr = read_uint(m_lowest_address + 0x18);
    LOGD << "base_addr:" << m_base_addr << "HEX" << hex << m_base_addr;
    m_is_ok = m_base_addr > 0;

    uint checksum = calculate_checksum();
    LOGD << "DF's checksum is" << hex << checksum;
    m_layout = new MemoryLayout(checksum);
    if (!m_layout->is_valid()) {
        QMessageBox::critical(0, tr("Unidentified Version"),
            tr("I'm sorry but I don't know how to talk to this version of DF!"));
        LOGC << "unable to identify version from checksum:" << hex << checksum;
        m_is_ok = false;
    }

    return m_is_ok;
}
/*
    PROCESS_MEMORY_COUNTERS pmc;
    GetProcessMemoryInfo(m_proc, &pmc, sizeof(pmc));
    m_memory_size = pmc.WorkingSetSize;
    LOGD << "working set size: " << dec << m_memory_size / (1024.0f * 1024.0f) << "MB";

    PVOID peb_addr = GetPebAddress(m_proc);
    LOGD << "PEB is at: " << hex << peb_addr;

    QString connection_error = tr("I'm sorry. I'm having trouble connecting to DF. "
        "I can't seem to locate the PEB address of the process. \n\n"
        "Please re-launch DF and try again.");

    if (peb_addr == 0){
        QMessageBox::critical(0, tr("Connection Error"), connection_error);
        qCritical() << "PEB address came back as 0";
        m_is_ok = false;
    } else {
        PEB peb;
        DWORD bytes = 0;
        if (ReadProcessMemory(m_proc, (PCHAR)peb_addr, &peb, sizeof(PEB), &bytes)) {
                LOGD << "read" << bytes << "bytes BASE ADDR is at: " << hex << peb.ImageBaseAddress;
                m_base_addr = (int)peb.ImageBaseAddress;
        } else {
                QMessageBox::critical(0, tr("Connection Error"), connection_error);
                qCritical() << "unable to read remote PEB!" << GetLastError();
                m_is_ok = false;
        }
        if (m_is_ok) {
                int checksum = calculate_checksum();
                LOGD << "DF's checksum is:" << hex << checksum;
                //GameDataReader::ptr()->set_game_checksum(checksum);

                m_layout = new MemoryLayout(checksum);
                if (!m_layout->is_valid()) {
                        QMessageBox::critical(0, tr("Unidentified Version"),
                                tr("I'm sorry but I don't know how to talk to this version of DF!"));
                        LOGC << "unable to identify version from checksum:" << hex << checksum;
                        //m_is_ok = false;
                }
        }

        m_memory_correction = (int)m_base_addr - 0x0400000;
        LOGD << "memory correction " << m_memory_correction;
    }
    return true;
    */
