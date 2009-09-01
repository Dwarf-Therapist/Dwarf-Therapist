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

int DFInstanceLinux::write_string(int address, QString str) {
	return 0;
}

short DFInstanceLinux::read_short(int start_address, uint &bytes_read) {
	return 0;
}

ushort DFInstanceLinux::read_ushort(int start_address, uint &bytes_read) {
	return 0;
}

int DFInstanceLinux::read_int32(int start_address, uint &bytes_read) {
    long data = ptrace(PTRACE_PEEKDATA, m_pid, start_address, 0);
    if (errno)
        perror("ptrace read_int32");
    return data;
}

int DFInstanceLinux::write_int32(int start_address, int val) {
	return 0;
}

char DFInstanceLinux::read_char(int start_address, uint &bytes_read) {
    long data = ptrace(PTRACE_PEEKDATA, m_pid, start_address, 0);
    perror("readchar");
    char *c;
    c = (char*)&data;
    return c[0];
}

int DFInstanceLinux::read_raw(uint start_address, int bytes, void *buffer) {
    if (ptrace(PTRACE_ATTACH, m_pid, 0, 0) == -1) {
        // unable to attach
        perror("ptrace attach");
        LOGC << "Could not attach to PID" << m_pid;
        return 0;
    }
    // read this procs base address for the ELF header
    QFile mem_file(QString("/proc/%1/mem").arg(m_pid));
    if (!mem_file.open(QIODevice::ReadOnly)) {
        qCritical() << "Unable to open" << mem_file.fileName();
        ptrace(PTRACE_DETACH, m_pid, 0, 0);
        return 0;
    }
    mem_file.seek(start_address); // this should be the entry point in the ELF header
    QByteArray data = mem_file.read(bytes);
    memcpy(buffer, data.data(), data.size());
    ptrace(PTRACE_DETACH, m_pid, 0, 0);
    return data.size();
}

int DFInstanceLinux::write_raw(int start_address, int bytes, void *buffer) {
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
