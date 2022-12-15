/*
Dwarf Therapist
Copyright (c) 2022 Clement Vuchener

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
#include "dfinstancewine.h"
#include "truncatingfilelogger.h"
#include "utils.h"

#include <QDirIterator>
#include <QTextCodec>

#include <fstream>
#include <regex>
#include <vector>
#include <algorithm>

#include <errno.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/uio.h>

#define DEFAULT_BASE_ADDR_AMD64 0x140000000ull

DFInstanceWine::DFInstanceWine(QObject* parent)
    : DFInstance(parent)
{
}

DFInstanceWine::~DFInstanceWine() {
}

int DFInstanceWine::wait_for_stopped() {
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

bool DFInstanceWine::attach() {
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

bool DFInstanceWine::detach() {
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

USIZE DFInstanceWine::read_raw(const VIRTADDR addr, const USIZE bytes, void *buffer) {
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

USIZE DFInstanceWine::write_raw(const VIRTADDR addr, const USIZE bytes,
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

bool DFInstanceWine::set_pid(){
    QSet<PID> pids;
    QDirIterator iter("/proc", QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable | QDir::Executable);
    while (iter.hasNext()) {
        QString fn = iter.next();
        QFile file(QString("%1/comm").arg(fn));
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray comm = file.readAll();
            if (comm == "Dwarf Fortress.\n") {
                pids << iter.fileName().toLong();
            }
        }
    }

    m_pid = select_pid(pids);

    if (!m_pid)
        return false;

    return true;
}

struct IMAGE_DOS_HEADER {      // DOS .EXE header
	uint16_t e_magic;                     // Magic number
	uint16_t e_cblp;                      // Bytes on last page of file
	uint16_t e_cp;                        // Pages in file
	uint16_t e_crlc;                      // Relocations
	uint16_t e_cparhdr;                   // Size of header in paragraphs
	uint16_t e_minalloc;                  // Minimum extra paragraphs needed
	uint16_t e_maxalloc;                  // Maximum extra paragraphs needed
	uint16_t e_ss;                        // Initial (relative) SS value
	uint16_t e_sp;                        // Initial SP value
	uint16_t e_csum;                      // Checksum
	uint16_t e_ip;                        // Initial IP value
	uint16_t e_cs;                        // Initial (relative) CS value
	uint16_t e_lfarlc;                    // File address of relocation table
	uint16_t e_ovno;                      // Overlay number
	uint16_t e_res[4];                    // Reserved words
	uint16_t e_oemid;                     // OEM identifier (for e_oeminfo)
	uint16_t e_oeminfo;                   // OEM information; e_oemid specific
	uint16_t e_res2[10];                  // Reserved words
	uint32_t e_lfanew;                    // File address of new exe header
};

static constexpr uint16_t IMAGE_DOS_SIGNATURE = 0x5a4d; // "MZ"

struct IMAGE_FILE_HEADER {
	uint16_t Machine;
	uint16_t NumberOfSections;
	uint32_t TimeDateStamp;
	uint32_t PointerToSymbolTable;
	uint32_t NumberOfSymbols;
	uint16_t SizeOfOptionalHeader;
	uint16_t Characteristics;
};

struct IMAGE_NT_HEADERS {
	uint32_t                 Signature;
	IMAGE_FILE_HEADER     FileHeader;
	//IMAGE_OPTIONAL_HEADER OptionalHeader;
};

static constexpr uint32_t IMAGE_NT_SIGNATURE = 0x00004550; // "PE\0\0"
static constexpr uint16_t IMAGE_FILE_MACHINE_AMD64 = 0x8664;


void DFInstanceWine::find_running_copy() {
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

    std::regex procmaps_re("([0-9a-f]+)-([0-9a-f]+) ([-r][-w][-x][-p]) ([0-9a-f]+) ([0-9a-f]+):([0-9a-f]+) ([0-9]+)(?:\\s*(.+))");
    if (auto maps = std::ifstream(proc_path + "/maps")) {
        std::string line;
        while (std::getline(maps, line)) {
            std::smatch res;
            if (!std::regex_search(line, res, procmaps_re))
                continue;
            VIRTADDR start = std::stoull(res[1], nullptr, 16);
            if (start != DEFAULT_BASE_ADDR_AMD64)
                continue;
            auto pathname = std::string(res[8].first, res[8].second);
            if (auto exe = std::ifstream(pathname)) {
                IMAGE_DOS_HEADER dos_header;
                exe.read(reinterpret_cast<char *>(&dos_header), sizeof(dos_header));
                if (dos_header.e_magic != IMAGE_DOS_SIGNATURE) {
                    LOGE << "Invalid DOS header";
                    return;
                }
                IMAGE_NT_HEADERS nt_headers;
                exe.seekg(dos_header.e_lfanew);
                exe.read(reinterpret_cast<char *>(&nt_headers), sizeof(nt_headers));
                if (nt_headers.Signature != IMAGE_NT_SIGNATURE) {
                    LOGE << "Invalid NT header";
                    return;
                }
                if (nt_headers.FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64) {
                    LOGE << "Unsupported architecture";
                    return;
                }
                m_base_addr = 0;
                m_pointer_size = 8;
                m_status = DFS_CONNECTED;
                set_memory_layout(hexify(nt_headers.FileHeader.TimeDateStamp));
            }
            else {
                LOGE << "Failed to open executable: " << pathname.c_str();
                return;
            }
        }
    }
    else {
        LOGE << "Failed to open memory maps";
    }
}

bool DFInstanceWine::df_running(){
    pid_t cur_pid = m_pid;
    return (set_pid() && cur_pid == m_pid);
}

static constexpr std::size_t STRING_BUFFER_LENGTH = 16;

QString DFInstanceWine::read_string(VIRTADDR addr) {
    VIRTADDR buffer_addr = addr;
    USIZE len = read_int(addr + STRING_BUFFER_LENGTH);
    USIZE cap = read_int(addr + STRING_BUFFER_LENGTH + m_pointer_size);
    if (cap >= STRING_BUFFER_LENGTH)
        buffer_addr = read_addr(buffer_addr);

    char buf[1024];

    if (cap == 0) {
        LOGW << "string at" << addr << "is zero-cap";
        return "";
    }
    if (len == 0) {
        return "";
    }
    if (len > cap) {
        // probably not really a string
        LOGW << "string at" << addr << "is length" << len << "which is larger than cap" << cap;
        return "";
    }
    if (cap > sizeof(buf)) {
        LOGW << "string at" << addr << "is cap" << cap << "which is suspiciously large, ignoring";
        return "";
    }

    read_raw(buffer_addr, len, buf);
    return QTextCodec::codecForName("IBM437")->toUnicode(buf, len);
}

USIZE DFInstanceWine::write_string(VIRTADDR addr, const QString &str) {
    /*

      THIS IS TOTALLY DANGEROUS

      */

    // TODO, don't write strings longer than 15 characters to the string
    // unless it has already been expanded to a bigger allocation

    USIZE cap = read_int(addr + STRING_BUFFER_LENGTH + m_pointer_size);
    VIRTADDR buffer_addr = addr;
    if( cap >= STRING_BUFFER_LENGTH )
        buffer_addr = read_addr(buffer_addr);

    USIZE len = qMin<int>(str.length(), cap);
    write_int(addr + STRING_BUFFER_LENGTH, len);

    QByteArray data = QTextCodec::codecForName("IBM 437")->fromUnicode(str);
    USIZE bytes_written = write_raw(buffer_addr, len, data.data());
    return bytes_written;
}
