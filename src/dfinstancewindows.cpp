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
#include <tchar.h>

#include "dfinstance.h"
#include "dfinstancewindows.h"
#include "defines.h"
#include "truncatingfilelogger.h"
#include "dwarf.h"
#include "utils.h"
#include "gamedatareader.h"
#include "memorylayout.h"
#include "dwarftherapist.h"

#define DEFAULT_BASE_ADDR_I386  0x400000ul
#define DEFAULT_BASE_ADDR_AMD64 0x140000000ull
#if defined Q_PROCESSOR_X86_32
#   define DEFAULT_BASE_ADDR DEFAULT_BASE_ADDR_I386
#elif defined Q_PROCESSOR_X86_64
#   define DEFAULT_BASE_ADDR DEFAULT_BASE_ADDR_AMD64
#else
#   error Unsupported architecture
#endif

static constexpr std::size_t STRING_BUFFER_LENGTH = 16;

DFInstanceWindows::DFInstanceWindows(QObject* parent)
    : DFInstance(parent)
    , m_proc(0)
{}

DFInstanceWindows::~DFInstanceWindows() {
    // CloseHandle(0) is a no-op
    CloseHandle(m_proc);
}

static QString get_error_string(DWORD error) {
    LPWSTR bufPtr = NULL;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                  FORMAT_MESSAGE_FROM_SYSTEM |
                  FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL, error, 0, (LPWSTR)&bufPtr, 0, NULL);
    const QString result = bufPtr ? QString::fromWCharArray(bufPtr).trimmed()
                                  : QString("Unknown Error %1").arg(error);
    LocalFree(bufPtr);
    return result;
}

QString DFInstanceWindows::calculate_checksum(const IMAGE_NT_HEADERS &pe_header) {
    time_t compile_timestamp = pe_header.FileHeader.TimeDateStamp;
    LOGI << "Target EXE was compiled at " << QDateTime::fromTime_t(compile_timestamp).toString(Qt::ISODate);
    return hexify(compile_timestamp).toLower();
}

QString DFInstanceWindows::read_string(VIRTADDR addr) {
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

USIZE DFInstanceWindows::write_string(VIRTADDR addr, const QString &str) {
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

USIZE DFInstanceWindows::read_raw(VIRTADDR addr, USIZE bytes, void *buffer) {
    ZeroMemory(buffer, bytes);
    SIZE_T bytes_read = 0;
    if (!ReadProcessMemory(m_proc, reinterpret_cast<LPCVOID>(addr), buffer,
                           bytes, &bytes_read)) {
        DWORD error = GetLastError();
        LOGE << "ReadProcessMemory failed:" << get_error_string(error);
    }
    return bytes_read;
}

USIZE DFInstanceWindows::write_raw(VIRTADDR addr, USIZE bytes, const void *buffer) {
    SIZE_T bytes_written = 0;
    if (!WriteProcessMemory(m_proc, reinterpret_cast<LPVOID>(addr), buffer,
                            bytes, &bytes_written)) {
        DWORD error = GetLastError();
        LOGE << "WriteProcessMemory failed:" << get_error_string(error);
    }
    if (bytes_written != bytes) {
        LOGE << "WriteProcessMemoty partial write";
    }
    return bytes_written;
}

static const QSet<QString> df_window_classes{"OpenGL", "SDL_app"};

#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))

static BOOL CALLBACK enumWindowsProc(HWND hWnd, LPARAM lParam) {
    auto pids = reinterpret_cast<QSet<PID> *>(lParam);
    WCHAR className[8];
    if (!GetClassName(hWnd, className, ARRAY_SIZE(className))) {
        DWORD error = GetLastError();
        LOGE << "GetClassName failed:" << get_error_string(error);
        return true;
    }

    if (!className || (wcscmp(className, L"OpenGL") && wcscmp(className, L"SDL_app")))
        return true;

    WCHAR windowName[16];
    if (!GetWindowText(hWnd, windowName, ARRAY_SIZE(windowName))) {
        DWORD error = GetLastError();
        if (error != ERROR_SUCCESS) { // Windows 7 enumerate some special windows without title, ignore them.
            LOGE << "GetWindowText failed:" << get_error_string(error);
        }
        return true;
    }

    Q_ASSERT(windowName);

    if (wcscmp(windowName, L"Dwarf Fortress"))
        return true;

    DWORD pid = 0;
    GetWindowThreadProcessId(hWnd, &pid);
    if (!pid) {
        LOGE << "could not get PID for hwnd";
        return true;
    }

    *pids << pid;

    return true;
}

bool DFInstanceWindows::set_pid(){
    QSet<PID> pids;
    if (!EnumWindows(enumWindowsProc, reinterpret_cast<LPARAM>(&pids))) {
        LOGE << "error enumerating windows";
        return false;
    }

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        DWORD error = GetLastError();
        LOGE << "error creating toolhelp32 process snapshot:" << get_error_string(error);
        return false;
    }
    
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(pe32);

    if (!Process32First(snapshot, &pe32))
        return false;

    do {
        if (!_tcscmp(pe32.szExeFile, _T("Dwarf Fortress.exe")))
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
    if (!m_proc) {
        DWORD error = GetLastError();
        LOGE << "Error opening process!" << get_error_string(error);
    }
    LOGI << "PROC HANDLE:" << m_proc;

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, m_pid);
    if (snapshot == INVALID_HANDLE_VALUE) {
        DWORD error = GetLastError();
        LOGE << "Error creating toolhelp32 snapshot!" << get_error_string(error);
        return;
    } else {
        MODULEENTRY32 me32;
        me32.dwSize = sizeof(MODULEENTRY32);
        if (!Module32First(snapshot, &me32)) {
            DWORD error = GetLastError();
            LOGE << "Error enumerating modules!" << get_error_string(error);
            return;
        } else {
            VIRTADDR base_addr = reinterpret_cast<VIRTADDR>(me32.modBaseAddr);
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

            switch (pe_header.FileHeader.Machine) {
            case IMAGE_FILE_MACHINE_I386:
                m_base_addr = base_addr - DEFAULT_BASE_ADDR_I386;
                m_pointer_size = 4;
                break;
            case IMAGE_FILE_MACHINE_AMD64:
                m_base_addr = base_addr - DEFAULT_BASE_ADDR_AMD64;
                m_pointer_size = 8;
                break;
            default:
                LOGE << "unsupported machine architecture";
                m_base_addr = base_addr - DEFAULT_BASE_ADDR;
                m_pointer_size = sizeof(VIRTADDR);
            }

            if (m_pointer_size > sizeof(VIRTADDR)) {
                LOGE << "pointers too big";
                return;
            }

            SYSTEM_INFO sysinfo;
            GetSystemInfo(&sysinfo);

            // Look for a trap instruction in the executable memory
            static constexpr int TrapOpCode = 0xcc;
            m_trap_addr = 0;
            for (VIRTADDR addr = base_addr; addr < base_addr+me32.modBaseSize;) {
                MEMORY_BASIC_INFORMATION meminfo;
                if (VirtualQueryEx(m_proc, reinterpret_cast<void *>(addr), &meminfo, sizeof(meminfo)) != sizeof(meminfo)) {
                    DWORD error = GetLastError();
                    LOGE << "VirtualQueryEx failed:" << get_error_string(error);
                    break;
                }
                if (meminfo.Protect & (PAGE_EXECUTE|PAGE_EXECUTE_READ)) {
                    std::vector<char> buffer(sysinfo.dwPageSize);
                    for (VIRTADDR page = addr; page < addr+meminfo.RegionSize; page += buffer.size()) {
                        if (read_raw(page, buffer.size(), buffer.data()) != buffer.size()) {
                            LOGE << "Failed to read executable page";
                            break;
                        }
                        char *res = reinterpret_cast<char *>(memchr(buffer.data(), TrapOpCode, buffer.size()));
                        if (res) {
                            m_trap_addr = page + (res-buffer.data());
                            LOGD << "found trap instruction at" << hexify(m_trap_addr);
                            break;
                        }
                    }
                    if (m_trap_addr)
                        break;
                }
                addr += meminfo.RegionSize;
            }

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

class DFInstanceWindows::FunctionCall: public DFInstance::FunctionCall
{
    DFInstanceWindows *df;
    bool debug_process;
    DWORD thread_id;
    HANDLE thread_handle;
    CONTEXT saved_ctx, work_ctx;

public:
    FunctionCall(DFInstanceWindows *df)
        : df(df)
    {
        if (!df || !df->m_trap_addr) {
            this->df = nullptr;
            return;
        }

        if (!(debug_process = DebugActiveProcess(df->m_pid))) {
            DWORD error = GetLastError();
            LOGE << "DebugActiveProcess failed:" << get_error_string(error);
            this->df = nullptr;
            return;
        }

        DEBUG_EVENT event;
        while (true) {
            if (!WaitForDebugEvent(&event, INFINITE)) {
                DWORD error = GetLastError();
                LOGE << "WaitForDebugEvent failed:" << get_error_string(error);
                this->df = nullptr;
                return;
            }
            LOGD << "Debug event received:" << event.dwDebugEventCode
                 << "PID" << event.dwProcessId
                 << "TID" << event.dwThreadId;
            if (event.dwDebugEventCode == CREATE_PROCESS_DEBUG_EVENT) {
                if (!CloseHandle(event.u.CreateProcessInfo.hFile)) {
                    DWORD error = GetLastError();
                    LOGE << "CloseHandle for image file failed:" << get_error_string(error);
                }
            }

            if (event.dwDebugEventCode == EXCEPTION_DEBUG_EVENT)
                break;

            if (!ContinueDebugEvent(event.dwProcessId, event.dwThreadId, DBG_CONTINUE)) {
                DWORD error = GetLastError();
                LOGE << "ContinueDebugEvent failed:" << get_error_string(error);
                this->df = nullptr;
                return;
            }
        }

        thread_id = event.dwThreadId;
        thread_handle = OpenThread(THREAD_GET_CONTEXT | THREAD_SET_CONTEXT | THREAD_SUSPEND_RESUME, FALSE, thread_id);
        if (thread_handle == NULL) {
            DWORD error = GetLastError();
            LOGE << "OpenThread failed:" << get_error_string(error);
            this->df = nullptr;
            return;
        }

        saved_ctx.ContextFlags = CONTEXT_ALL;
        if (!GetThreadContext(thread_handle, &saved_ctx)) {
            DWORD error = GetLastError();
            LOGE << "GetThreadContext failed:" << get_error_string(error);
            this->df = nullptr;
            return;
        }
        memcpy(&work_ctx, &saved_ctx, sizeof(CONTEXT));
        work_ctx.ContextFlags = CONTEXT_FULL;
    }

    ~FunctionCall() override
    {
        DWORD continue_status = DBG_CONTINUE;
        DEBUG_EVENT event;
        event.dwProcessId = df->m_pid;
        event.dwThreadId = thread_id;
        while (true) {
            if (!ContinueDebugEvent(event.dwProcessId, event.dwThreadId, continue_status)) {
                DWORD errcode = GetLastError();
                LOGE << "ContinueDebugEvent failed:" << get_error_string(errcode);
                break;
            }

            if (!WaitForDebugEvent(&event, INFINITE)) {
                DWORD errcode = GetLastError();
                LOGE << "WaitForDebugEvent failed:" << get_error_string(errcode);
                break;
            }
            LOGD << "Debug event received:" << event.dwDebugEventCode
                 << "PID" << event.dwProcessId
                 << "TID" << event.dwThreadId;

            if (event.dwDebugEventCode == EXCEPTION_DEBUG_EVENT) {
               switch (event.u.Exception.ExceptionRecord.ExceptionCode) {
               case EXCEPTION_BREAKPOINT:
               case EXCEPTION_SINGLE_STEP:
                   continue_status = DBG_CONTINUE;
                   break;
               default:
                   LOGW << "DF exception code" << hexify(event.u.Exception.ExceptionRecord.ExceptionCode)
                        << (event.u.Exception.dwFirstChance ? "first chance" : "last chance");

                   if (!GetThreadContext(thread_handle, &work_ctx)) {
                       DWORD errcode = GetLastError();
                       LOGE << "SetThreadContext failed:" << get_error_string(errcode);
                   }
                   else {
                       LOGD << "At address" << hexify(work_ctx.Rip) << "default:" << hexify(work_ctx.Rip-df->m_base_addr);
                       //LOGD << "CS" << hexify(work_ctx.SegCs) << "DS" << hexify(work_ctx.SegDs)
                       //     << "ES" << hexify(work_ctx.SegEs) << "FS" << hexify(work_ctx.SegFs);
                       //LOGD << "GS" << hexify(work_ctx.SegGs) << "SS" << hexify(work_ctx.SegSs)
                       //     << "FLAGS" << hexify(work_ctx.EFlags) << "RIP" << hexify(work_ctx.Rip);
                       //LOGD << "RAX" << hexify(work_ctx.Rax) << "RCX" << hexify(work_ctx.Rcx)
                       //     << "RDX" << hexify(work_ctx.Rdx) << "RBX" << hexify(work_ctx.Rbx);
                       //LOGD << "RSP" << hexify(work_ctx.Rsp) << "RBP" << hexify(work_ctx.Rbp)
                       //     << "RSI" << hexify(work_ctx.Rsi) << "RDI" << hexify(work_ctx.Rdi);
                       //LOGD << "R8" << hexify(work_ctx.R8) << "R9" << hexify(work_ctx.R9)
                       //     << "R10" << hexify(work_ctx.R10) << "R11" << hexify(work_ctx.R11);
                       //LOGD << "R12" << hexify(work_ctx.R12) << "R13" << hexify(work_ctx.R13)
                       //     << "R14" << hexify(work_ctx.R14) << "R15" << hexify(work_ctx.R15);
                   }
                   continue_status = DBG_EXCEPTION_NOT_HANDLED;
               }
            }
            else
                continue_status = DBG_CONTINUE;

            if (event.dwDebugEventCode == EXIT_THREAD_DEBUG_EVENT && event.dwThreadId == thread_id) {
                LOGD << "Debug thread exited";
                break;
            }
            if (event.dwDebugEventCode == EXIT_PROCESS_DEBUG_EVENT) {
                LOGE << "DF terminated";
                return;
            }
        }

        if (thread_handle != NULL) {
            if (!CloseHandle(thread_handle)) {
                DWORD error = GetLastError();
                LOGE << "CloseHandle for thread failed:" << get_error_string(error);
            }
        }
        if (debug_process) {
            if (!DebugActiveProcessStop(df->m_pid)) {
                DWORD error = GetLastError();
                LOGE << "DebugActiveProcessStop failed:" << get_error_string(error);
            }
        }
    }

    VIRTADDR push_data(void *data, std::size_t len) override
    {
        LOGE << "DFInstanceWindows::FunctionCall::push_data not implemented";
        return 0;
    }

    std::pair<bool, VIRTADDR> call(VIRTADDR fn_addr, const std::vector<VIRTADDR> &args) override
    {
        static constexpr std::pair<bool, VIRTADDR> error = {false, 0};
        if (!df) {
            LOGE << "invalid function call object";
            return error;
        }

        auto old_stack_pointer = work_ctx.Rsp;

        // prepare stack buffer with return address and arguments
        std::vector<char> stack;
        auto add_on_stack = [&stack, len=df->m_pointer_size] (VIRTADDR value) {
            std::copy_n(reinterpret_cast<char *>(&value), len, std::back_inserter(stack));
        };
        add_on_stack(df->m_trap_addr); // return address
        if (df->m_pointer_size == 4) {
            LOGE << "call not implemented for win32";
            return error;
        }
#ifdef Q_OS_WIN64
        else if (df->m_pointer_size == 8) { // Microsoft x64
            stack.resize(stack.size()+32); // Reserve shadow store
            for (auto i = 0u; i < args.size(); ++i) {
                auto val = args[i];
                switch (i) {
                case 0: work_ctx.Rcx = val; break;
                case 1: work_ctx.Rdx = val; break;
                case 2: work_ctx.R8 = val; break;
                case 3: work_ctx.R9 = val; break;
                default: add_on_stack(val);
                }
            }
        }
#endif
        else {
            LOGE << "architecture not supported";
            return error;
        }

        //work_ctx.Rsp -= stack.size()-df->m_pointer_size; // reserve space for args
        //work_ctx.Rsp &= -16; // align on 16 bytes
        //work_ctx.Rsp -= df->m_pointer_size; // space for return address
        work_ctx.Rsp -= stack.size(); // reserve space for stack data
        work_ctx.Rsp &= -16; // align on 16 bytes

        // copy buffer in remote stack
        if (df->write_raw(work_ctx.Rsp, stack.size(), stack.data()) != stack.size()) {
            LOGE << "Failed to write stack";
            return error;
        }

        // call function
        work_ctx.Rip = fn_addr;
        LOGD << "Calling function at" << hexify(fn_addr);

        const bool singlestep = true;
        if (singlestep)
            work_ctx.EFlags |= 0x0100; // trap flag

        if (!SetThreadContext(thread_handle, &work_ctx)) {
            DWORD errcode = GetLastError();
            LOGE << "SetThreadContext failed:" << get_error_string(errcode);
            return error;
        }

        bool ok = true;
        DWORD continue_status = DBG_CONTINUE;
        DEBUG_EVENT event;
        event.dwProcessId = df->m_pid;
        event.dwThreadId = thread_id;
        do {
            do {
                if (!ContinueDebugEvent(event.dwProcessId, event.dwThreadId, continue_status)) {
                    DWORD errcode = GetLastError();
                    LOGE << "ContinueDebugEvent failed:" << get_error_string(errcode);
                    ok = false;
                    break;
                }

                if (!WaitForDebugEvent(&event, INFINITE)) {
                    DWORD errcode = GetLastError();
                    LOGE << "WaitForDebugEvent failed:" << get_error_string(errcode);
                    ok = false;
                    break;
                }
                LOGD << "Debug event received:" << event.dwDebugEventCode
                     << "PID" << event.dwProcessId
                     << "TID" << event.dwThreadId;

                if (event.dwDebugEventCode == EXCEPTION_DEBUG_EVENT) {
                   switch (event.u.Exception.ExceptionRecord.ExceptionCode) {
                   case EXCEPTION_BREAKPOINT:
                   case EXCEPTION_SINGLE_STEP:
                       continue_status = DBG_CONTINUE;
                       break;
                   default:
                       LOGW << "DF exception code" << hexify(event.u.Exception.ExceptionRecord.ExceptionCode)
                            << (event.u.Exception.dwFirstChance ? "first chance" : "last chance");

                       if (!GetThreadContext(thread_handle, &work_ctx)) {
                           DWORD errcode = GetLastError();
                           LOGE << "SetThreadContext failed:" << get_error_string(errcode);
                       }
                       else {
                           LOGD << "At address" << hexify(work_ctx.Rip) << "default:" << hexify(work_ctx.Rip-df->m_base_addr);
                       }
                       continue_status = DBG_EXCEPTION_NOT_HANDLED;
                   }
                }
                else
                    continue_status = DBG_CONTINUE;

                if (event.dwDebugEventCode == EXIT_THREAD_DEBUG_EVENT) {
                    LOGE << "DF terminated";
                    return error;
                }
            } while (event.dwDebugEventCode != EXCEPTION_DEBUG_EVENT);

            if (!GetThreadContext(thread_handle, &work_ctx)) {
                DWORD errcode = GetLastError();
                LOGE << "SetThreadContext failed:" << get_error_string(errcode);
                ok = false;
                break;
            }
            if (singlestep) {
                LOGD << "CS" << hexify(work_ctx.SegCs) << "DS" << hexify(work_ctx.SegDs)
                     << "ES" << hexify(work_ctx.SegEs) << "FS" << hexify(work_ctx.SegFs);
                LOGD << "GS" << hexify(work_ctx.SegGs) << "SS" << hexify(work_ctx.SegSs)
                     << "FLAGS" << hexify(work_ctx.EFlags) << "RIP" << hexify(work_ctx.Rip)
                     << "Adjusted RIP" << hexify(work_ctx.Rip-df->m_base_addr);
                LOGD << "RAX" << hexify(work_ctx.Rax) << "RCX" << hexify(work_ctx.Rcx)
                     << "RDX" << hexify(work_ctx.Rdx) << "RBX" << hexify(work_ctx.Rbx);
                LOGD << "RSP" << hexify(work_ctx.Rsp) << "RBP" << hexify(work_ctx.Rbp)
                     << "RSI" << hexify(work_ctx.Rsi) << "RDI" << hexify(work_ctx.Rdi);
                LOGD << "R8" << hexify(work_ctx.R8) << "R9" << hexify(work_ctx.R9)
                     << "R10" << hexify(work_ctx.R10) << "R11" << hexify(work_ctx.R11);
                LOGD << "R12" << hexify(work_ctx.R12) << "R13" << hexify(work_ctx.R13)
                     << "R14" << hexify(work_ctx.R14) << "R15" << hexify(work_ctx.R15);
                work_ctx.EFlags |= 0x0100; // trap flag
                if (!SetThreadContext(thread_handle, &work_ctx)) {
                    DWORD errcode = GetLastError();
                    LOGE << "SetThreadContext failed:" << get_error_string(errcode);
                    ok = false;
                    break;
                }
            }
        } while (work_ctx.Rip != df->m_trap_addr+1);

        // restore context
        if (!SetThreadContext(thread_handle, &saved_ctx)) {
            DWORD errcode = GetLastError();
            LOGE << "SetThreadContext failed:" << get_error_string(errcode);
            return error;
        }

        work_ctx.Rsp = old_stack_pointer;
        return {ok, work_ctx.Rax};
    }

    operator bool() const override
    {
        return df;
    }
};

std::unique_ptr<DFInstance::FunctionCall> DFInstanceWindows::make_function_call()
{
    if (m_pointer_size == 8)
        return std::make_unique<FunctionCall>(this);
    else {
        LOGE << "Function calls not implemented";
        return nullptr;
    }
}
