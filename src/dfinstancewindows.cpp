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
#include <QtDebug>
#include <QMessageBox>

#ifdef Q_OS_WIN
#include <windows.h>
#include <psapi.h>

#include "dfinstance.h"
#include "dfinstancewindows.h"
#include "defines.h"
#include "truncatingfilelogger.h"
#include "dwarf.h"
#include "utils.h"
#include "gamedatareader.h"
#include "memorylayout.h"
#include "memorysegment.h"
#include "dwarftherapist.h"
#include "cp437codec.h"

DFInstanceWindows::DFInstanceWindows(QObject* parent)
    : DFInstance(parent)
    , m_proc(0)
{}

DFInstanceWindows::~DFInstanceWindows() {
    if (m_proc) {
        CloseHandle(m_proc);
    }
}

QString DFInstanceWindows::get_last_error() {
    LPWSTR bufPtr = NULL;
    DWORD err = GetLastError();
    FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                   FORMAT_MESSAGE_FROM_SYSTEM |
                   FORMAT_MESSAGE_IGNORE_INSERTS,
                   NULL, err, 0, (LPWSTR)&bufPtr, 0, NULL);
    const QString result =
        (bufPtr) ? QString::fromUtf16((const ushort*)bufPtr).trimmed() :
                   QString("Unknown Error %1").arg(err);
    LocalFree(bufPtr);
    return result;
}

QString DFInstanceWindows::calculate_checksum() {
    QDateTime compile_timestamp = QDateTime::fromTime_t(m_pe_header.FileHeader.TimeDateStamp);
    LOGI << "Target EXE was compiled at " << compile_timestamp.toString(Qt::ISODate);
    return hexify(m_pe_header.FileHeader.TimeDateStamp).toLower();
}

QString DFInstanceWindows::read_string(const uint &addr) {
    int len = read_int(addr + memory_layout()->string_length_offset());
    int cap = read_int(addr + memory_layout()->string_cap_offset());
    VIRTADDR buffer_addr = addr + memory_layout()->string_buffer_offset();
    if (cap >= 16)
        buffer_addr = read_addr(buffer_addr);

    if (len > cap || len < 0 || len > 1024) {
#ifdef _DEBUG
        // probably not really a string
        LOGW << "Tried to read a string at" << hex << addr
            << "but it was totally not a string...";
#endif
        return QString();
    }
    Q_ASSERT_X(len <= cap, "read_string",
               "Length must be less than or equal to capacity!");
    Q_ASSERT_X(len >= 0, "read_string", "Length must be >=0!");
    Q_ASSERT_X(len < (1 << 16), "read_string",
               "String must be of sane length!");

    QByteArray buf = get_data(buffer_addr, len);
    CP437Codec *c = new CP437Codec();
    return c->toUnicode(buf);

    //the line below would be nice, but apparently a ~20mb *.icu library is required for that single call to qtextcodec...wtf. really.
    //it's also been pretty bad performance-wise on linux, so it may be best to forget about it entirely
    //return QTextCodec::codecForName("IBM 437")->toUnicode(buf);
}

USIZE DFInstanceWindows::write_string(const VIRTADDR &addr, const QString &str) {
    /*

      THIS IS TOTALLY DANGEROUS

      */

    // TODO, don't write strings longer than 15 characters to the string
    // unless it has already been expanded to a bigger allocation

    int cap = read_int(addr + memory_layout()->string_cap_offset());
    VIRTADDR buffer_addr = addr + memory_layout()->string_buffer_offset();
    if( cap >= 16 )
        buffer_addr = read_addr(buffer_addr);

    int len = qMin<int>(str.length(), cap);
    write_int(addr + memory_layout()->string_length_offset(), len);

    QByteArray data = QTextCodec::codecForName("IBM 437")->fromUnicode(str);
    int bytes_written = write_raw(buffer_addr, len, data.data());
    return bytes_written;
}

USIZE DFInstanceWindows::read_raw(const VIRTADDR &addr, const USIZE &bytes,
                                void *buffer) {
    ZeroMemory(buffer, bytes);
    USIZE bytes_read = 0;
    ReadProcessMemory(m_proc, reinterpret_cast<LPCVOID>(addr), buffer,
                      bytes, reinterpret_cast<SIZE_T*>(&bytes_read));
    return bytes_read;
}

USIZE DFInstanceWindows::write_raw(const VIRTADDR &addr, const USIZE &bytes, const void *buffer) {
    USIZE bytes_written = 0;
    WriteProcessMemory(m_proc, reinterpret_cast<LPVOID>(addr), buffer,
                       bytes, reinterpret_cast<SIZE_T*>(&bytes_written));

    Q_ASSERT(bytes_written == bytes);
    return bytes_written;
}

bool DFInstanceWindows::find_running_copy(bool connect_anyway) {
    LOGI << "attempting to find running copy of DF by window handle";
    m_is_ok = false;

    HWND hwnd = FindWindow(L"OpenGL", L"Dwarf Fortress");
    if (!hwnd)
        hwnd = FindWindow(L"SDL_app", L"Dwarf Fortress");
    if (!hwnd)
        hwnd = FindWindow(NULL, L"Dwarf Fortress");

    if (!hwnd) {
        QMessageBox::warning(0, tr("Warning"),
            tr("Unable to locate a running copy of Dwarf "
            "Fortress, are you sure it's running?"));
        LOGW << "can't find running copy";
        return m_is_ok;
    }
    LOGI << "found copy with HWND: " << hwnd;

    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    if (pid == 0) {
        return m_is_ok;
    }
    LOGI << "PID of process is: " << pid;
    m_hwnd = hwnd;

    m_proc = OpenProcess(PROCESS_QUERY_INFORMATION
                         | PROCESS_VM_READ
                         | PROCESS_VM_OPERATION
                         | PROCESS_VM_WRITE, false, pid);
    LOGI << "PROC HANDLE:" << m_proc;
    if (m_proc == NULL) {
        LOGE << "Error opening process!" << get_last_error();
    }

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
    if (snapshot == INVALID_HANDLE_VALUE) {
        LOGE << "Error creating toolhelp32 snapshot!" << get_last_error();
    } else {
        MODULEENTRY32 me32;
        me32.dwSize = sizeof(MODULEENTRY32);
        if (!Module32First(snapshot, &me32)) {
            LOGE << "Error enumerating modules!" << get_last_error();
        } else {
            m_base_addr = (intptr_t)me32.modBaseAddr;
            read_raw(m_base_addr, sizeof(m_dos_header), &m_dos_header);
            if(m_dos_header.e_magic != IMAGE_DOS_SIGNATURE){
                qWarning() << "invalid executable";
            }

            //the dos stub contains a relative address to the pe header, which is used to get the pe header information
            read_raw(m_base_addr + m_dos_header.e_lfanew, sizeof(m_pe_header), &m_pe_header);
            if(m_pe_header.Signature != IMAGE_NT_SIGNATURE){
                qWarning() << "unsupported PE header type";
            }
            m_is_ok = true;
        }
        CloseHandle(snapshot);     // Must clean up the snapshot object!
    }

    if (m_is_ok){
        m_layout = get_memory_layout(calculate_checksum(), !connect_anyway);
        //pass the imagebase address - the default windows linker address to the memory layout
        //for use with global addresses (anyting in the [addresses] section of the layout file
        m_layout->set_base_address(m_pe_header.OptionalHeader.ImageBase - 0x00400000);
    } else {
        if(connect_anyway)
            m_is_ok = true;
        else // time to bail
            return m_is_ok;
    }

    map_virtual_memory();

    if (DT->user_settings()->value("options/alert_on_lost_connection", true)
        .toBool() && m_layout && m_layout->is_complete()) {
        m_heartbeat_timer->start(1000); // check every second for disconnection
    }

    char * modName = new char[MAX_PATH];
    DWORD lenModName = 0;
    if ((lenModName = GetModuleFileNameExA(m_proc, NULL, modName, MAX_PATH)) != 0) {
        QString exe_path = QString::fromLocal8Bit(modName, lenModName);
        LOGI << "GetModuleFileNameEx returned: " << exe_path;
        QFileInfo exe(exe_path);
        m_df_dir = exe.absoluteDir();
        LOGI << "Dwarf fortress path:" << m_df_dir.absolutePath();
    }

    m_is_ok = true;
    return m_is_ok;
}

/*! OS specific way of asking the kernel for valid virtual memory pages from
  the DF process. These pages are used to speed up scanning, and validate
  reads from DF's memory. If addresses are not within ranges found by this
  method, they will fail the is_valid_address() method */
void DFInstanceWindows::map_virtual_memory() {
    // destroy existing segments
    foreach(MemorySegment *seg, m_regions) {
        delete(seg);
    }
    m_regions.clear();

    if (!m_is_ok)
        return;

    // start by figuring out what kernel we're talking to
    TRACE << "Mapping out virtual memory";
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    TRACE << "PROCESSORS:" << info.dwNumberOfProcessors;
    TRACE << "PROC TYPE:" << info.wProcessorArchitecture <<
            info.wProcessorLevel <<
            info.wProcessorRevision;
    TRACE << "PAGE SIZE" << info.dwPageSize;

    VIRTADDR start = (intptr_t)info.lpMinimumApplicationAddress;
    VIRTADDR max_address = (intptr_t)info.lpMaximumApplicationAddress;
    TRACE << "MIN ADDRESS:" << hexify(start);
    TRACE << "MAX ADDRESS:" << hexify(max_address);

    int page_size = info.dwPageSize;
    int accepted = 0;
    int rejected = 0;
    VIRTADDR segment_start = start;
    USIZE segment_size = page_size;
    while (start < max_address) {
        MEMORY_BASIC_INFORMATION mbi;
        int sz = VirtualQueryEx(m_proc, reinterpret_cast<LPCVOID>(start), &mbi,
                                sizeof(MEMORY_BASIC_INFORMATION));
        if (sz != sizeof(MEMORY_BASIC_INFORMATION)) {
            // incomplete data returned. increment start and move on...
            start += page_size;
            continue;
        }

        segment_start = (intptr_t)mbi.BaseAddress;
        segment_size = (USIZE)mbi.RegionSize;
        if (mbi.State == MEM_COMMIT
            //&& !(mbi.Protect & PAGE_GUARD)
            && (mbi.Protect & PAGE_EXECUTE_READ ||
                mbi.Protect & PAGE_EXECUTE_READWRITE ||
                mbi.Protect & PAGE_READONLY ||
                mbi.Protect & PAGE_READWRITE ||
                mbi.Protect & PAGE_WRITECOPY)
            ) {
            TRACE << "FOUND READABLE COMMITED MEMORY SEGMENT FROM" <<
                    hexify(segment_start) << "-" <<
                    hexify(segment_start + segment_size) <<
                    "SIZE:" << (segment_size / 1024.0f) << "KB" <<
                    "FLAGS:" << mbi.Protect;
            MemorySegment *segment = new MemorySegment("", segment_start,
                                                       segment_start
                                                       + segment_size);
            segment->is_guarded = mbi.Protect & PAGE_GUARD;
            m_regions << segment;
            accepted++;
        } else {
            TRACE << "REJECTING MEMORY SEGMENT AT" << hexify(segment_start) <<
                     "SIZE:" << (segment_size / 1024.0f) << "KB FLAGS:" <<
                     mbi.Protect;
            rejected++;
        }
        if (mbi.RegionSize)
            start += mbi.RegionSize;
        else
            start += page_size;
    }
    m_lowest_address = 0xFFFFFFFF;
    m_highest_address = 0;
    foreach(MemorySegment *seg, m_regions) {
        if (seg->start_addr < m_lowest_address)
            m_lowest_address = seg->start_addr;
        if (seg->end_addr > m_highest_address)
            m_highest_address = seg->end_addr;
    }
    LOGD << "MEMORY SEGMENT SUMMARY: accepted" << accepted << "rejected" <<
            rejected << "total" << accepted + rejected;
}

bool DFInstance::authorize(){
    return true;
}

#endif
