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
        if (!wcscmp(pe32.szExeFile, L"Dwarf Fortress.exe"))
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

struct ParameterStack
{
    std::size_t word_size;
    std::vector<char> vec;

    explicit ParameterStack(std::size_t word_size)
        : word_size(word_size)
    {
    }

    void add(VIRTADDR value)
    {
        std::copy_n(reinterpret_cast<char *>(&value), word_size, std::back_inserter(vec));
    }

    void reserve(std::size_t size)
    {
        vec.resize(vec.size() + size);
    }

    auto data() const { return vec.data(); }
    auto size() const { return vec.size(); }
};

struct Win32Context
{
    using context_t = CONTEXT;
    static constexpr auto GetContext = GetThreadContext;
    static constexpr auto SetContext = SetThreadContext;
#ifdef Q_OS_WIN64
    static constexpr auto SP = &CONTEXT::Rsp;
    static constexpr auto IP = &CONTEXT::Rip;
    static void debug_context(const CONTEXT &ctx)
    {
        LOGD << "CS" << hexify(ctx.SegCs) << "DS" << hexify(ctx.SegDs)
             << "ES" << hexify(ctx.SegEs) << "FS" << hexify(ctx.SegFs);
        LOGD << "GS" << hexify(ctx.SegGs) << "SS" << hexify(ctx.SegSs)
             << "FLAGS" << hexify(ctx.EFlags) << "RIP" << hexify(ctx.Rip);
        LOGD << "RAX" << hexify(ctx.Rax) << "RCX" << hexify(ctx.Rcx)
             << "RDX" << hexify(ctx.Rdx) << "RBX" << hexify(ctx.Rbx);
        LOGD << "RSP" << hexify(ctx.Rsp) << "RBP" << hexify(ctx.Rbp)
             << "RSI" << hexify(ctx.Rsi) << "RDI" << hexify(ctx.Rdi);
        LOGD << "R8" << hexify(ctx.R8) << "R9" << hexify(ctx.R9)
             << "R10" << hexify(ctx.R10) << "R11" << hexify(ctx.R11);
        LOGD << "R12" << hexify(ctx.R12) << "R13" << hexify(ctx.R13)
             << "R14" << hexify(ctx.R14) << "R15" << hexify(ctx.R15);
    }
#else
    static constexpr auto SP = &CONTEXT::Esp;
    static constexpr auto IP = &CONTEXT::Eip;
    static void debug_context(const CONTEXT &ctx)
    {
        LOGD << "CS" << hexify(ctx.SegCs) << "DS" << hexify(ctx.SegDs)
             << "ES" << hexify(ctx.SegEs) << "FS" << hexify(ctx.SegFs);
        LOGD << "GS" << hexify(ctx.SegGs) << "SS" << hexify(ctx.SegSs)
             << "FLAGS" << hexify(ctx.EFlags) << "EIP" << hexify(ctx.Eip);
        LOGD << "EAX" << hexify(ctx.Eax) << "ECX" << hexify(ctx.Ecx)
             << "EDX" << hexify(ctx.Edx) << "EBX" << hexify(ctx.Ebx);
        LOGD << "ESP" << hexify(ctx.Esp) << "EBP" << hexify(ctx.Ebp)
             << "ESI" << hexify(ctx.Esi) << "EDI" << hexify(ctx.Edi);
    }
#endif
    enum Flags: DWORD {
        Full = CONTEXT_FULL,
        All = CONTEXT_ALL,
    };
};

#ifdef Q_OS_WIN64
struct Wow64Context
{
    using context_t = WOW64_CONTEXT;
    static constexpr auto GetContext = Wow64GetThreadContext;
    static constexpr auto SetContext = Wow64SetThreadContext;
    static constexpr auto SP = &WOW64_CONTEXT::Esp;
    static constexpr auto IP = &WOW64_CONTEXT::Eip;
    static void debug_context(const WOW64_CONTEXT &ctx)
    {
        LOGD << "CS" << hexify(ctx.SegCs) << "DS" << hexify(ctx.SegDs)
             << "ES" << hexify(ctx.SegEs) << "FS" << hexify(ctx.SegFs);
        LOGD << "GS" << hexify(ctx.SegGs) << "SS" << hexify(ctx.SegSs)
             << "FLAGS" << hexify(ctx.EFlags) << "EIP" << hexify(ctx.Eip);
        LOGD << "EAX" << hexify(ctx.Eax) << "ECX" << hexify(ctx.Ecx)
             << "EDX" << hexify(ctx.Edx) << "EBX" << hexify(ctx.Ebx);
        LOGD << "ESP" << hexify(ctx.Esp) << "EBP" << hexify(ctx.Ebp)
             << "ESI" << hexify(ctx.Esi) << "EDI" << hexify(ctx.Edi);
    }
    enum Flags: DWORD {
        Full = WOW64_CONTEXT_FULL,
        All = WOW64_CONTEXT_ALL,
    };
};
#endif

struct X86CallConventions
{
    template<typename ContextType>
    static void set_parameters(DFInstance::FunctionCall::CallType call_type,
                               ContextType &ctx,
                               ParameterStack &stack,
                               const std::vector<VIRTADDR> &args)
    {
        switch (call_type) {
        case DFInstance::FunctionCall::CallType::MethodCall:
            // thiscall
            if (args.size() < 1) {
                LOGE << "Missing this pointer for thiscall";
                return;
            }
            ctx.Ecx = args[0];
            for (auto i = 1u; i < args.size(); ++i)
                stack.add(args[i]);
        }
    }

    template<typename ContextType>
    static VIRTADDR return_value(const ContextType &ctx)
    {
        return ctx.Eax;
    }
};

#ifdef Q_OS_WIN64
struct X64CallConventions
{
    static void set_parameters(DFInstance::FunctionCall::CallType,
                               CONTEXT &ctx,
                               ParameterStack &stack,
                               const std::vector<VIRTADDR> &args)
    {
        // Microsoft x64 convention for all call types
        stack.reserve(32); // Reserve shadow store
        for (auto i = 0u; i < args.size(); ++i) {
            auto val = args[i];
            switch (i) {
            case 0: ctx.Rcx = val; break;
            case 1: ctx.Rdx = val; break;
            case 2: ctx.R8 = val; break;
            case 3: ctx.R9 = val; break;
            default: stack.add(val);
            }
        }
    }

    static VIRTADDR return_value(const CONTEXT &ctx)
    {
        return ctx.Rax;
    }
};
#endif

template<typename Context, typename CallConvention>
class DFInstanceWindows::FunctionCall: public DFInstance::FunctionCall
{
    DFInstanceWindows *df;
    bool debug_process;
    DWORD thread_id;
    HANDLE thread_handle;
    typename Context::context_t saved_ctx, work_ctx;

    DEBUG_EVENT debug_event;
    DWORD continue_status;

    template<typename EventHandler>
    bool doDebugLoop(EventHandler event_handler)
    {
        do {
            if (!ContinueDebugEvent(debug_event.dwProcessId, debug_event.dwThreadId, continue_status)) {
                DWORD error = GetLastError();
                LOGE << "ContinueDebugEvent failed:" << get_error_string(error);
                return false;
            }
            if (!WaitForDebugEvent(&debug_event, INFINITE)) {
                DWORD error = GetLastError();
                LOGE << "WaitForDebugEvent failed:" << get_error_string(error);
                return false;
            }
            LOGD << "Debug event received:" << debug_event.dwDebugEventCode
                 << "PID" << debug_event.dwProcessId
                 << "TID" << debug_event.dwThreadId;

            continue_status = DBG_CONTINUE;
            switch (debug_event.dwDebugEventCode) {
                case EXCEPTION_DEBUG_EVENT:
                    switch (debug_event.u.Exception.ExceptionRecord.ExceptionCode) {
                        case EXCEPTION_BREAKPOINT:
                        case EXCEPTION_SINGLE_STEP:
                            break;
                        default:
                            LOGW << "DF exception code" << hexify(debug_event.u.Exception.ExceptionRecord.ExceptionCode)
                                 << (debug_event.u.Exception.dwFirstChance ? "first chance" : "last chance");

                            if (DT->get_log_manager()->get_appender("core")->minimum_level() <= LL_DEBUG) {
                                if (!Context::GetContext(thread_handle, &work_ctx)) {
                                    DWORD error = GetLastError();
                                    LOGE << "GetThreadContext failed:" << get_error_string(error);
                                }
                                else {
                                    auto ip = work_ctx.*Context::IP;
                                    LOGD << "At address" << hexify(ip)
                                         << "(with default base:" << hexify(ip-df->m_base_addr) << ")";
                                    Context::debug_context(work_ctx);
                                }
                            }
                            continue_status = DBG_EXCEPTION_NOT_HANDLED;
                    }
                    break;
                case CREATE_PROCESS_DEBUG_EVENT:
                    // image file handle need to be closed
                    if (!CloseHandle(debug_event.u.CreateProcessInfo.hFile)) {
                        DWORD error = GetLastError();
                        LOGE << "CloseHandle for image file failed:" << get_error_string(error);
                    }
                    break;
                case EXIT_PROCESS_DEBUG_EVENT:
                    LOGE << "DF terminated";
                    break;
                default:
                    break;
            }
        } while (event_handler(debug_event));
        return true;
    }

public:
    FunctionCall(DFInstanceWindows *df)
        : df(df)
        , thread_id(0)
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

        if (!WaitForDebugEvent(&debug_event, INFINITE)) {
            DWORD error = GetLastError();
            LOGE << "WaitForDebugEvent failed:" << get_error_string(error);
            this->df = nullptr;
            return;
        }
        continue_status = DBG_CONTINUE;

        if (!doDebugLoop([](const DEBUG_EVENT &event){return event.dwDebugEventCode != EXCEPTION_DEBUG_EVENT;})) {
                this->df = nullptr;
                return;
        }

        thread_id = debug_event.dwThreadId;
        thread_handle = OpenThread(THREAD_GET_CONTEXT | THREAD_SET_CONTEXT | THREAD_SUSPEND_RESUME, FALSE, thread_id);
        if (thread_handle == NULL) {
            DWORD error = GetLastError();
            LOGE << "OpenThread failed:" << get_error_string(error);
            this->df = nullptr;
            return;
        }

        saved_ctx.ContextFlags = Context::All;
        if (!Context::GetContext(thread_handle, &saved_ctx)) {
            DWORD error = GetLastError();
            LOGE << "GetThreadContext failed:" << get_error_string(error);
            this->df = nullptr;
            return;
        }
        memcpy(&work_ctx, &saved_ctx, sizeof(typename Context::context_t));
        work_ctx.ContextFlags = Context::Full;
    }

    ~FunctionCall() override
    {
        if (thread_id != 0) {
            doDebugLoop([this](const DEBUG_EVENT &event){
                    // wait for debug thread to exit
                    return !(event.dwDebugEventCode == EXIT_THREAD_DEBUG_EVENT && event.dwThreadId == thread_id);
            });
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

    std::pair<bool, VIRTADDR> call(DFInstance::FunctionCall::CallType call_type,
                                   VIRTADDR fn_addr, const std::vector<VIRTADDR> &args) override
    {
        static constexpr std::pair<bool, VIRTADDR> error = {false, 0};
        if (!df) {
            LOGE << "invalid function call object";
            return error;
        }

        auto old_stack_pointer = work_ctx.*Context::SP;

        // prepare stack buffer with return address and arguments
        ParameterStack stack(df->m_pointer_size);
        stack.add(df->m_trap_addr); // return address
        CallConvention::set_parameters(call_type, work_ctx, stack, args);

        work_ctx.*Context::SP -= stack.size(); // reserve space for stack data
        work_ctx.*Context::SP &= -16; // align on 16 bytes

        // copy buffer in remote stack
        if (df->write_raw(work_ctx.*Context::SP, stack.size(), stack.data()) != stack.size()) {
            LOGE << "Failed to write stack";
            return error;
        }

        // call function
        work_ctx.*Context::IP = fn_addr;
        LOGD << "Calling function at" << hexify(fn_addr);

#ifdef DEBUG_SINGLESTEP
        work_ctx.EFlags |= 0x0100; // trap flag
#endif

        if (!Context::SetContext(thread_handle, &work_ctx)) {
            DWORD errcode = GetLastError();
            LOGE << "SetThreadContext failed:" << get_error_string(errcode);
            return error;
        }

        bool ok = true;
        if (!doDebugLoop([this, &ok](const DEBUG_EVENT &event) {
                switch (event.dwDebugEventCode) {
                case EXIT_THREAD_DEBUG_EVENT:
                    LOGE << "Debug thread terminated during call";
                    thread_id = 0;
                    if (!Context::GetContext(thread_handle, &work_ctx)) {
                        DWORD error = GetLastError();
                        LOGE << "GetThreadContext failed:" << get_error_string(error);
                        ok = false;
                        return false;
                    }
                    {
                        auto ip = work_ctx.*Context::IP;
                        LOGD << "Exit at" << hexify(ip)
                             << "(with default base:" << hexify(ip-df->m_base_addr) << ")";
                    }
                    return false;
                case EXCEPTION_DEBUG_EVENT:
                    if (!event.u.Exception.dwFirstChance) {
                        // stop call for last chance exceptions
                        LOGE << "Call stopped by exception";
                        ok = false;
                        return false;
                    }
                    if (!Context::GetContext(thread_handle, &work_ctx)) {
                        DWORD error = GetLastError();
                        LOGE << "GetThreadContext failed:" << get_error_string(error);
                        ok = false;
                        return false;
                    }
                    if (work_ctx.*Context::IP == df->m_trap_addr+1) // breakpoint reached
                        return false;
#ifdef DEBUG_SINGLESTEP
                    {
                        auto ip = work_ctx.*Context::IP;
                        LOGD << "Singlestep at" << hexify(ip)
                             << "(with default base:" << hexify(ip-df->m_base_addr) << ")";
                        Context::debug_context(work_ctx);
                        work_ctx.EFlags |= 0x0100; // trap flag
                        if (!Context::SetContext(thread_handle, &work_ctx)) {
                            DWORD error = GetLastError();
                            LOGE << "SetThreadContext failed:" << get_error_string(error);
                            ok = false;
                            return false;
                        }
                    }
#endif
                    return true;
                default:
                    return true;
                }
        })) {
            ok = false;
        }
        if (thread_id == 0)
            return error;

        // restore context
        if (!Context::SetContext(thread_handle, &saved_ctx)) {
            DWORD errcode = GetLastError();
            LOGE << "SetThreadContext failed:" << get_error_string(errcode);
            return error;
        }

        work_ctx.*Context::SP = old_stack_pointer;
        return {ok, CallConvention::return_value(work_ctx)};
    }

    operator bool() const override
    {
        return df;
    }
};

std::unique_ptr<DFInstance::FunctionCall> DFInstanceWindows::make_function_call()
{
#ifdef Q_OS_WIN64
        if (m_pointer_size == 4)
            return std::make_unique<FunctionCall<Wow64Context, X86CallConventions>>(this);
        else if (m_pointer_size == 8)
            return std::make_unique<FunctionCall<Win32Context, X64CallConventions>>(this);
        else
            return nullptr;
#else
        return std::make_unique<FunctionCall<Win32Context, X86CallConventions>>(this);
#endif
}
