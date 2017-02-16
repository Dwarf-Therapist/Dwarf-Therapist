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
#include <QDateTime>
#include <QTimer>
#include <QMessageBox>
#include <QTextCodec>

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

DFInstanceWindows::DFInstanceWindows(QObject* parent)
    : DFInstance(parent)
    , m_proc(0)
{}

DFInstanceWindows::~DFInstanceWindows() {
    // CloseHandle(0) is a no-op
    CloseHandle(m_proc);
}

QString DFInstanceWindows::get_last_error() {
    LPWSTR bufPtr = NULL;
    DWORD err = GetLastError();
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                  FORMAT_MESSAGE_FROM_SYSTEM |
                  FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL, err, 0, (LPWSTR)&bufPtr, 0, NULL);
    const QString result = bufPtr ? QString::fromWCharArray(bufPtr).trimmed()
                                  : QString("Unknown Error %1").arg(err);
    LocalFree(bufPtr);
    return result;
}

QString DFInstanceWindows::calculate_checksum(const IMAGE_NT_HEADERS &pe_header) {
    time_t compile_timestamp = pe_header.FileHeader.TimeDateStamp;
    LOGI << "Target EXE was compiled at " << QDateTime::fromTime_t(compile_timestamp).toString(Qt::ISODate);
    return hexify(compile_timestamp).toLower();
}

QString DFInstanceWindows::read_string(const uint &addr) {
    USIZE len = read_int(addr + memory_layout()->string_length_offset());
    USIZE cap = read_int(addr + memory_layout()->string_cap_offset());
    VIRTADDR buffer_addr = addr + memory_layout()->string_buffer_offset();
    if (cap >= 16)
        buffer_addr = read_addr(buffer_addr);

    char buf[1024];

    if (len == 0 || cap == 0) {
        LOGW << "string at" << addr << "is zero-length or zero-cap";
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

USIZE DFInstanceWindows::write_string(VIRTADDR addr, const QString &str) {
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

USIZE DFInstanceWindows::read_raw(VIRTADDR addr, USIZE bytes,
                                void *buffer) {
    ZeroMemory(buffer, bytes);
    USIZE bytes_read = 0;
    ReadProcessMemory(m_proc, reinterpret_cast<LPCVOID>(addr), buffer,
                      bytes, reinterpret_cast<SIZE_T*>(&bytes_read));
    return bytes_read;
}

USIZE DFInstanceWindows::write_raw(VIRTADDR addr, USIZE bytes, const void *buffer) {
    USIZE bytes_written = 0;
    WriteProcessMemory(m_proc, reinterpret_cast<LPVOID>(addr), buffer,
                       bytes, reinterpret_cast<SIZE_T*>(&bytes_written));

    Q_ASSERT(bytes_written == bytes);
    return bytes_written;
}

static const QSet<QString> df_window_classes{"OpenGL", "SDL_app"};

BOOL CALLBACK static enumWindowsProc(HWND hWnd, LPARAM lParam) {
    auto pids = reinterpret_cast<QSet<PID> *>(lParam);
    WCHAR classNameW[8];
    if (!GetClassName(hWnd, classNameW, sizeof(classNameW))) {
        LOGE << "GetClassName failed:" << get_last_error();
        return false;
    }

    if (!className && wcscmp(className, L"OpenGL") && wcscmp(className, L"SDL_app"))
        return true;

    WCHAR windowName[16];
    if (!GetWindowName(hWnd, windowName, sizeof(windowName))) {
        LOGE << "GetWindowName failed:" << get_last_error();
        return false;
    }

    Q_ASSERT(windowName);

    if (wcscmp(windowName, L"Dwarf Fortress"))
        return true;

    GetWindowThreadProcessId(hWnd, &pid);
    if (!pid) {
        LOGE << "could not get PID for hwnd";
        return false;
    }

    pids << pid;

    return true;
}

bool DFInstanceWindows::set_pid(){
    QSet<PID> pids;
    if (!EnumWindows(enumWindowsProc, &pids)) {
        LOGE << "error enumerating windows";
        return false;
    }

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        LOGE << "error creating toolhelp32 process snapshot:" << get_last_error();
        return false;
    }
    
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(pe32);

    if (!Process32First(snapshot, &pe32))
        return false;

    do {
        if (!_tcscmp(&pe32.szExeFile, _T("Dwarf Fortress.exe")))
            pids << pe32.th32ProcessID;
    } while (Process32Next(snapshot, &pe32));

    m_pid = select_pid(pids);

    return m_pid != 0;
}

bool DFInstanceWindows::df_running(){
    DWORD cur_pid = m_pid;
    return (set_pid() && cur_pid == m_pid);
}

void DFInstanceWindows::find_running_copy() {
    m_status = DFS_DISCONNECTED;
    LOGI << "attempting to find running copy of DF";

    if(!set_pid()){
        return;
    }

    LOGI << "PID of process is: " << m_pid;

    m_proc = OpenProcess(PROCESS_QUERY_INFORMATION
                         | PROCESS_VM_OPERATION
                         | PROCESS_VM_READ
                         | PROCESS_VM_WRITE, false, m_pid);
    LOGI << "PROC HANDLE:" << m_proc;
    if (!m_proc) {
        LOGE << "Error opening process!" << get_last_error();
    }

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, m_pid);
    if (snapshot == INVALID_HANDLE_VALUE) {
        LOGE << "Error creating toolhelp32 snapshot!" << get_last_error();
        return;
    } else {
        MODULEENTRY32 me32;
        me32.dwSize = sizeof(MODULEENTRY32);
        if (!Module32First(snapshot, &me32)) {
            LOGE << "Error enumerating modules!" << get_last_error();
            return;
        } else {
            VIRTADDR base_addr = me32.modBaseAddr;
            IMAGE_DOS_HEADER dos_header;
            read_raw(base_addr, sizeof(dos_header), &dos_header);
            if(dos_header.e_magic != IMAGE_DOS_SIGNATURE){
                qWarning() << "invalid executable";
                return;
            }

            //the dos stub contains a relative address to the pe header, which is used to get the pe header information
            IMAGE_NT_HEADERS pe_header;
            read_raw(base_addr + dos_header.e_lfanew, sizeof(pe_header), &pe_header);
            if(pe_header.Signature != IMAGE_NT_SIGNATURE){
                qWarning() << "unsupported PE header type";
                return;
            }

            LOGI << "RAW BASE ADDRESS:" << base_addr;
            m_base_addr = base_addr - 0x00400000;

            m_status = DFS_CONNECTED;
            set_memory_layout(calculate_checksum(pe_header));

        }
        CloseHandle(snapshot);     // Must clean up the snapshot object!
    }

    WCHAR modName[MAX_PATH];
    DWORD lenModName = GetModuleFileNameExW(m_proc, NULL, modName, MAX_PATH);
    if (lenModName) {
        QString exe_path = QString::fromWCharArray(modName, lenModName);
        LOGI << "GetModuleFileNameEx returned: " << exe_path;
        QFileInfo exe(exe_path);
        m_df_dir = exe.absoluteDir();
        LOGI << "Dwarf Fortress path:" << m_df_dir.absolutePath();
    }

    return;
}
