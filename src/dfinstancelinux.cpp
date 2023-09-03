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
#include "dfinstance.h"
#include "dfinstancelinux.h"
#include "truncatingfilelogger.h"
#include "utils.h"

#include <QDirIterator>
#include <QTextCodec>
#include <QCryptographicHash>

#include <fstream>
#include <regex>
#include <vector>
#include <algorithm>

#include <errno.h>
#include <sys/mman.h>
#include <sys/ptrace.h>
#include <sys/syscall.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/uio.h>
#include <elf.h>

DFInstanceLinux::DFInstanceLinux(QObject* parent)
    : DFInstance(parent)
{
}

DFInstanceLinux::~DFInstanceLinux() {
}

int DFInstanceLinux::wait_for_stopped() {
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
    return status;
}

bool DFInstanceLinux::attach() {
    int attach_count = m_attach_count++;
    TRACE << "STARTING ATTACH" << attach_count;
    if (attach_count > 0) {
        TRACE << "ALREADY ATTACHED SKIPPING..." << attach_count;
        return true;
    }

    if (ptrace(PTRACE_ATTACH, m_pid, 0, 0) == -1) { // unable to attach
        LOGE << "attach:" << strerror(errno) << "attaching to PID" << m_pid;
        m_attach_count--;
        return false;
    }

    wait_for_stopped();

    TRACE << "FINISHED ATTACH" << attach_count;
    return true;
}

bool DFInstanceLinux::detach() {
    int attach_count = --m_attach_count;
    //TRACE << "STARTING DETACH" << attach_count;
    if (attach_count > 0) {
        TRACE << "NO NEED TO DETACH SKIPPING..." << attach_count;
        return true;
    }
    if (attach_count < 0) {
        LOGE << "Attempted to detach while the instance was not attached";
        m_attach_count++;
        return true;
    }

    ptrace(PTRACE_DETACH, m_pid, 0, 0);
    TRACE << "FINISHED DETACH" << attach_count;
    return true;
}

USIZE DFInstanceLinux::read_raw(const VIRTADDR addr, const USIZE bytes, void *buffer) {
    struct iovec local_iov = {buffer, bytes};
    struct iovec remote_iov = {reinterpret_cast<void *>(addr), bytes};
    SSIZE bytes_read = process_vm_readv(m_pid, &local_iov, 1, &remote_iov, 1, 0);
    if (bytes_read == -1) {
        LOGE << "READ_RAW:" << QString(strerror(errno)) << "READING" << bytes << "BYTES FROM" << hexify(addr) << "TO" << buffer;
        memset(buffer, 0, bytes);
        return 0;
    }

    TRACE << "Read" << bytes_read << "bytes of" << bytes << "bytes from" << hexify(addr) << "to" << buffer;

    if ((size_t)bytes_read < bytes)
        memset((char *)buffer + bytes_read, 0, bytes - bytes_read);

    return bytes_read;
}

USIZE DFInstanceLinux::write_raw(const VIRTADDR addr, const USIZE bytes,
                                 const void *buffer) {
    struct iovec local_iov = {const_cast<void *>(buffer), bytes};
    struct iovec remote_iov = {reinterpret_cast<void *>(addr), bytes};
    SSIZE bytes_written = process_vm_writev(m_pid, &local_iov, 1, &remote_iov, 1, 0);
    if (bytes_written == -1) {
        LOGE << "WRITE_RAW:" << QString(strerror(errno)) << "WRITING" << bytes << "BYTES FROM" << buffer << "TO" << hexify(addr);
    } else if ((USIZE)bytes_written != bytes) {
        LOGW << "WRITE_RAW: PARTIALLY WROTE:" << bytes_written << "BYTES OF" << bytes << "BYTES FROM" << buffer << "TO" << hexify(addr);
    } else {
        LOGD << "WRITE_RAW: WROTE" << bytes << "BYTES FROM" << buffer << "TO" << hexify(addr);
    }

    // tell the caller how many bytes we wrote
    return bytes_written;
}

bool DFInstanceLinux::set_pid(){
    QSet<PID> pids;
    QDirIterator iter("/proc", QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable | QDir::Executable);
    while (iter.hasNext()) {
        QString fn = iter.next();
        QFile file(QString("%1/comm").arg(fn));
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray comm = file.readAll();
            if (comm == "dwarfort\n") {
                pids << iter.fileName().toLong();
            }
        }
    }

    m_pid = select_pid(pids);

    if (!m_pid)
        return false;

    return true;
}

static constexpr char ElfMagic[4] = { ELFMAG0, ELFMAG1, ELFMAG2, ELFMAG3 };

static int get_elf_class(std::ifstream &exe)
{
    char magic[4];
    exe.seekg(0);
    exe.read(magic, 4);

    if (!std::equal (magic, magic+4, ElfMagic))
        return 0;

    return exe.get();
}

void DFInstanceLinux::find_running_copy() {
    m_status = DFS_DISCONNECTED;
    // find PID of DF
    TRACE << "attempting to find running copy of DF by executable name";

    if(set_pid()){
        TRACE << "USING PID:" << m_pid;
    }else{
        return;
    }

    m_df_dir = QDir(QFileInfo(QString("/proc/%1/cwd").arg(m_pid)).symLinkTarget());
    LOGI << "Dwarf fortress path:" << m_df_dir.absolutePath();

    std::string proc_path = std::string("/proc/") + std::to_string(m_pid);

    QString md5 = "UNKNOWN";
    if (auto exe = std::ifstream(proc_path + "/exe")) {
        // check class from ELF header to find the pointer size and string::assign relocation
        switch (get_elf_class(exe)) {
            case ELFCLASS32:
                m_pointer_size = 4;
                break;
            case ELFCLASS64:
                m_pointer_size = 8;
                break;
            default:
                LOGE << "invalid ELF header";
                m_pointer_size = sizeof(VIRTADDR);
        }

        // ELF binaries don't seem to store a linker timestamp, so just MD5 the file.
        QCryptographicHash hash(QCryptographicHash::Md5);
        char buffer[PAGE_SIZE];
        exe.seekg(0);
        while (exe.read(buffer, PAGE_SIZE))
            hash.addData(buffer, PAGE_SIZE);
        hash.addData(buffer, exe.gcount());
        md5 = hexify(hash.result().mid(0, 4)).toLower();
        TRACE << "GOT MD5:" << md5;
    }
    else {
        LOGE << "Failed to open DF executable";
        return;
    }

    if (m_pointer_size > sizeof(VIRTADDR)) {
        LOGE << "pointers too big";
        return;
    }

    m_status = DFS_CONNECTED;
    set_memory_layout(md5);
}

bool DFInstanceLinux::df_running(){
    pid_t cur_pid = m_pid;
    return (set_pid() && cur_pid == m_pid);
}

static constexpr std::size_t STRING_BUFFER_LENGTH = 16;

QString DFInstanceLinux::read_string(VIRTADDR addr) {
    VIRTADDR buffer_addr = read_addr(addr);
    std::size_t len = read_int(addr + m_pointer_size);
    std::size_t cap = buffer_addr == addr + 2*m_pointer_size
        ? STRING_BUFFER_LENGTH-1
        : read_int(addr +  2*m_pointer_size);
    if (len > cap) {
        LOGW << "string at" << addr << "is length" << len << "which is larger than cap" << cap;
        return {};
    }
    if (cap > 1000000) {
        LOGW << "string at" << addr << "is cap" << cap << "which is suspiciously large, ignoring";
        return {};
    }
    std::vector<char> buffer(len);
    read_raw(buffer_addr, buffer.size(), buffer.data());
    return QTextCodec::codecForName("IBM437")->toUnicode(buffer.data(), buffer.size());
}

USIZE DFInstanceLinux::write_string(const VIRTADDR addr, const QString &str) {
    VIRTADDR buffer_addr = read_addr(addr);
    std::size_t cap = buffer_addr == addr + 2*m_pointer_size
        ? STRING_BUFFER_LENGTH-1
        : read_int(addr + 2*m_pointer_size);
    auto data = QTextCodec::codecForName("IBM437")->fromUnicode(str);
    std::size_t capped_len = std::min(size_t(data.length()), cap);
    data.resize(capped_len);
    data.append('\0');
    write_int(addr + m_pointer_size, capped_len);
    return write_raw(buffer_addr, capped_len+1, data.data());
}
