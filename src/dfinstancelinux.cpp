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
        return addrs;
    }
    uint bytes_read = read_raw(start, bytes, stuff);
    if (bytes_read != bytes) {
        qWarning() << "Tried to read" << bytes << "bytes but only got" << bytes_read;
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
	return 0;
}

char DFInstanceLinux::read_char(const uint &addr) {
	char retval;
	read_raw(addr, sizeof(char), &retval);
	return retval;
}

uint DFInstanceLinux::read_raw(const uint &addr, const uint &bytes, void *buffer) {
    if (ptrace(PTRACE_ATTACH, m_pid, 0, 0) == -1) {
        // unable to attach
        perror("ptrace attach");
        qCritical() << "Could not attach to PID" << m_pid;
        return 0;
    }
    // read this procs base address for the ELF header
    QFile mem_file(QString("/proc/%1/mem").arg(m_pid));
    if (!mem_file.open(QIODevice::ReadOnly)) {
        qCritical() << "Unable to open" << mem_file.fileName();
        ptrace(PTRACE_DETACH, m_pid, 0, 0);
        return 0;
    }
    int status;
    wait(&status);

    uint bytes_read = 0;
    QByteArray data;
    while (bytes_read < bytes) {
        //qDebug() << "reading raw from:" << hex << start_address + bytes_read;
        mem_file.seek(addr + bytes_read);
        QByteArray tmp = mem_file.read(bytes - data.size());
        bytes_read += tmp.size();
        data.append(tmp);
        if (bytes_read == 0) {
            qWarning() << "read 0 bytes from" << hex << addr + bytes_read;
            break;
        }
        //qDebug() << "bytes_read:" << bytes_read;
    }
    memcpy(buffer, data.data(), data.size());
    mem_file.close();
    ptrace(PTRACE_DETACH, m_pid, 0, 0);
    return bytes_read;
}


uint DFInstanceLinux::write_raw(const uint &addr, const uint &bytes, void *buffer) {
	return 0;
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
    uint lowest_addr = 0xFFFFFFFF;
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
                if (start_addr < lowest_addr)
                    lowest_addr = start_addr;
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


    if (ptrace(PTRACE_ATTACH, m_pid, 0, 0) < 0) {
        // unable to attach
        perror("ptrace attach");
        LOGC << "Could not attach to PID" << m_pid;
        return false;
    }
    TRACE << "Waiting for PID" << m_pid << "to stop...";
    int status;
    wait(&status);
    TRACE << "attached to process" << m_pid;
    //we're now tracing DF, and DF is stopped

    // read this procs base address for the ELF header
    QFile mem_file(QString("/proc/%1/mem").arg(m_pid));
    if (!mem_file.open(QIODevice::ReadOnly)) {
        qCritical() << "Unable to open" << mem_file.fileName();
        ptrace(PTRACE_DETACH, m_pid, 0, 0);
        return false;
    }
    mem_file.seek(lowest_addr + 0x18); // this should be the entry point in the ELF header
    QByteArray data = mem_file.read(4);
    mem_file.close();
    memcpy(&m_base_addr, data.data(), sizeof(uint));
    LOGD << "base_addr:" << m_base_addr << "HEX" << hex << m_base_addr;

    ptrace(PTRACE_DETACH, m_pid, 0, 0);
    m_is_ok = true;
    uint checksum = calculate_checksum();
    LOGD << "DF's checksum is" << hex << checksum;
    m_layout = new MemoryLayout(checksum);
    if (!m_layout->is_valid()) {
        QMessageBox::critical(0, tr("Unidentified Version"),
            tr("I'm sorry but I don't know how to talk to this version of DF!"));
        LOGC << "unable to identify version from checksum:" << hex << checksum;
        m_is_ok = false;
    }

    return true;
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
