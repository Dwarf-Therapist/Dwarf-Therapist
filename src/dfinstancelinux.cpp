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
#include <sys/param.h>
#include <sys/user.h>
#include <sys/sysctl.h>
#include <stdio.h>
#include <stdlib.h>

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

int DFInstanceLinux::calculate_checksum() {
    // ELF binaries don't seem to store a linker timestamp, so just MD5 the file.
    int md5 = 0; // we're going to throw away a lot of this checksum we just need 4bytes worth
    QProcess *proc = new QProcess(this);
    QStringList args;
    args << QString("/proc/%1/exe").arg(m_pid);
    proc->start("md5sum", args);
    proc->waitForFinished(1000);
    if (proc->exitCode() == 0) {//found it
        QByteArray out = proc->readAllStandardOutput();
        QString str_md5(out);
        md5 = str_md5.toInt();
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

int DFInstanceLinux::read_raw(int start_address, int bytes, void *buffer) {
    uint bytes_read = 0;
    int steps = bytes / 4;
    if (bytes % 4 != 0)
        steps++;

    for(int i = 0; i < steps; i++) {
        long data = ptrace(PTRACE_PEEKDATA, m_pid, start_address + i*sizeof(int), 0);
        if (errno)
            perror("readraw");
        char *tmp;
        tmp = (char*)&data;
        memcpy((char*)buffer + i * sizeof(int), tmp, sizeof(int));
    }
    return bytes_read;
}

int DFInstanceLinux::write_raw(int start_address, int bytes, void *buffer) {
	return 0;
}

bool DFInstanceLinux::find_running_copy() {
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

    if (ptrace(PTRACE_ATTACH, m_pid, 0, 0) < 0) {
        // unable to attach
        perror("ptrace attach");
        LOGC << "Could not attach to PID" << m_pid;
        return false;
    }
    TRACE << "Attached to PID" << m_pid;
    //we're now tracing DF...
    // attempt to locate the PE header

    QFile f(QString("/proc/%1/maps").arg(m_pid));
    if (!f.open(QIODevice::ReadOnly)) {
        LOGC << "Unable to open" << f.fileName();
        ptrace(PTRACE_DETACH, m_pid, 0, 0);
        return false;
    }
    TRACE << "opened" << f.fileName();
    QByteArray line = f.readLine();
    TRACE << "first line" << line;
    f.close();

    // parse the first line to see find the base
    int start_addr = 0;
    int end_addr = 0;
    QRegExp rx("^([0-9a-f]{8})-([0-9a-f]{8})");
    if (rx.indexIn(line) != -1) {
        bool ok;
        start_addr = rx.cap(1).toInt(&ok, 16);
        end_addr = rx.cap(2).toInt(&ok, 16);
        LOGD << "RANGE" << hex << start_addr << "-" << end_addr;
    } else {
        LOGC << "unable to read the base range from" << f.fileName();
        ptrace(PTRACE_DETACH, m_pid, 0, 0);
        return false;
    }

    // read this procs base address for the ELF header
    QFile mem_file(QString("/proc/%1/mem").arg(m_pid));
    if (!mem_file.open(QIODevice::ReadOnly)) {
        qCritical() << "Unable to open" << mem_file.fileName();
        ptrace(PTRACE_DETACH, m_pid, 0, 0);
        return false;
    }
    mem_file.seek(start_addr + 0x18); // this should be the entry point in the ELF header
    QByteArray data = mem_file.read(4);
    qDebug() << "read" << data;
    for (int i = 0; i < 4; ++i) {
        qDebug() << "byte" << i << QString::number(data[i]);
    }
    memcpy(&m_base_addr, data.data(), sizeof(int));

    LOGD << "base_addr:" << m_base_addr << "HEX" << hex << m_base_addr;
    mem_file.close();
    int checksum = calculate_checksum();

    ptrace(PTRACE_DETACH, m_pid, 0, 0);
    m_is_ok = true;
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
