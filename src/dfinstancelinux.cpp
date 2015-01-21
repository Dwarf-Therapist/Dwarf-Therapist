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
#include "memorysegment.h"
#include "truncatingfilelogger.h"
#include "cp437codec.h"
#include "utils.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QCryptographicHash>
#include <QDirIterator>
#if QT_VERSION >= 0x050000
# include <QRegularExpression>
#else
# include <QRegExp>
#endif

#include <errno.h>
#include <sys/mman.h>
#include <sys/ptrace.h>
#include <sys/syscall.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>

struct iovec {
    void *iov_base;
    size_t iov_len;
};

DFInstanceLinux::DFInstanceLinux(QObject* parent)
    : DFInstanceNix(parent)
    , m_pid(0)
    , m_inject_addr(-1)
    , m_alloc_start(0)
    , m_alloc_end(0)
    , m_warned_pvm(false)
{
}

DFInstanceLinux::~DFInstanceLinux() {
    if (m_attach_count > 0) {
        detach();
    }
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
    TRACE << "STARTING ATTACH" << m_attach_count;
    if (is_attached()) {
        m_attach_count++;
        TRACE << "ALREADY ATTACHED SKIPPING..." << m_attach_count;
        return true;
    }

    if (ptrace(PTRACE_ATTACH, m_pid, 0, 0) == -1) { // unable to attach
        LOGE << "attach:" << strerror(errno) << "attaching to PID" << m_pid;
        return false;
    }

    wait_for_stopped();

    m_attach_count++;
    TRACE << "FINISHED ATTACH" << m_attach_count;
    return m_attach_count > 0;
}

bool DFInstanceLinux::detach() {
    //TRACE << "STARTING DETACH" << m_attach_count;
    if(m_memory_file.isOpen())
        m_memory_file.close();
    m_attach_count--;
    if (m_attach_count > 0) {
        TRACE << "NO NEED TO DETACH SKIPPING..." << m_attach_count;
        return true;
    }

    ptrace(PTRACE_DETACH, m_pid, 0, 0);
    TRACE << "FINISHED DETACH" << m_attach_count;
    return m_attach_count > 0;
}

SSIZE DFInstanceLinux::process_vm(long number, const VIRTADDR &addr
                                  , const USIZE &bytes, void *buffer) {
    struct iovec local_iov = {buffer, bytes};
    struct iovec remote_iov = {reinterpret_cast<void *>(addr), bytes};

    SSIZE r = syscall(number, m_pid, &local_iov, 1UL, &remote_iov, 1UL, 0UL);

    if (r == -1 && errno == ENOSYS && !m_warned_pvm) {
        m_warned_pvm = true;
        LOGI << "Kernel does not support process_vm API, falling back to ptrace.";
        // reset errno, logger may have modified it
        errno = ENOSYS;
    }

    return r;
}

USIZE DFInstanceLinux::read_raw_ptrace(const VIRTADDR &addr, const USIZE &bytes, void *buffer) {
    int bytes_read = 0;

    // try to attach, will be ignored if we're already attached
    attach();

    // open the memory virtual file for this proc (can only read once
    // attached and child is stopped
    if (!m_memory_file.isOpen() && !m_memory_file.open(QIODevice::ReadOnly)) {
        LOGE << "Unable to open" << m_memory_file.fileName();
        detach();
        return bytes_read;
    }

    m_memory_file.seek(addr);
    bytes_read = m_memory_file.read(static_cast<char *>(buffer), bytes);

    detach();
    return bytes_read;
}

USIZE DFInstanceLinux::read_raw(const VIRTADDR &addr, const USIZE &bytes, void *buffer) {
    memset(buffer, 0, bytes);

    SSIZE bytes_read = process_vm(SYS_process_vm_readv, addr, bytes, buffer);
    if (bytes_read == -1) {
        if (errno != ENOSYS) {
            LOGE << "READ_RAW:" << QString(strerror(errno)) << "READING" << bytes << "BYTES FROM" << hexify(addr) << "TO" << buffer;
        }
        return read_raw_ptrace(addr, bytes, buffer);
    }

    TRACE << "Read" << bytes_read << "bytes of" << bytes << "bytes from" << hexify(addr) << "to" << buffer;

    return bytes_read;
}

USIZE DFInstanceLinux::write_raw_ptrace(const VIRTADDR &addr, const USIZE &bytes,
                                        const void *buffer) {
    /* Since most kernels won't let us write to /proc/<pid>/mem, we have to poke
     * out data in n bytes at a time. Good thing we read way more than we write.
     *
     * On x86-64 systems, the size that POKEDATA writes is 8 bytes instead of
     * 4, so we need to use the sizeof( long ) as our step size.
     */

    attach();

    const USIZE stepsize = sizeof(unsigned long);
    USIZE bytes_written = 0; // keep track of how much we've written
    USIZE steps = bytes / stepsize;
    LOGD << "WRITE_RAW_PTRACE: WILL WRITE" << bytes << "BYTES FROM" << buffer
         << "TO" << hexify(addr) << "OVER" << steps + !!bytes % stepsize
         << "STEPS, WITH STEPSIZE " << stepsize;

    USIZE offset = 0;

    // for each step write a single word to the child
    for (USIZE i = 0; i < steps; ++i) {
        const unsigned long data = static_cast<const unsigned long *>(buffer)[i];
        LOGD << "WRITE_RAW_PTRACE: WRITING" << hexify(data) << "TO" << addr + offset;
        if (ptrace(PTRACE_POKEDATA, m_pid, addr + offset, data)) {
            LOGE << "WRITE_RAW_PTRACE:" << QString(strerror(errno)) << "WRITING"
                 << bytes << "BYTES FROM" << buffer << "TO" << hexify(addr);
            break;
        } else {
            bytes_written += stepsize;
        }
        offset += stepsize;
    }

    // write any last stragglers
    if (bytes % stepsize) {
        unsigned long buf;
        if (read_raw(addr + offset, stepsize, &buf) == stepsize) {
            memcpy(&buf, static_cast<const char *>(buffer) + offset, bytes % stepsize);
            if (ptrace(PTRACE_POKEDATA, m_pid, addr + offset, buf)) {
                LOGE << "WRITE_RAW_PTRACE:" << QString(strerror(errno))
                     << "WRITING LAST" << stepsize << "BYTES OF" << bytes
                     << "BYTES FROM" << buffer << "TO" << hexify(addr);
            } else {
                bytes_written += stepsize;
            }
        }
    }

    detach();
    return bytes_written;
}

USIZE DFInstanceLinux::write_raw(const VIRTADDR &addr, const USIZE &bytes,
                                 const void *buffer) {
    // const_cast is safe because process_vm passes the params as is
    SSIZE bytes_written = process_vm(SYS_process_vm_writev, addr, bytes
                                     , const_cast<void *>(buffer));
    if (bytes_written == -1) {
        if (errno == ENOSYS) {
            return write_raw_ptrace(addr, bytes, buffer);
        } else {
            LOGE << "WRITE_RAW:" << QString(strerror(errno)) << "WRITING" << bytes << "BYTES FROM" << buffer << "TO" << hexify(addr);
        }
    } else if ((USIZE)bytes_written != bytes) {
        LOGW << "WRITE_RAW: PARTIALLY WROTE:" << bytes_written << "BYTES OF" << bytes << "BYTES FROM" << buffer << "TO" << hexify(addr);
    } else {
        LOGD << "WRITE_RAW: WROTE" << bytes << "BYTES FROM" << buffer << "TO" << hexify(addr);
    }

    // tell the caller how many bytes we wrote
    return bytes_written;
}

bool DFInstanceLinux::find_running_copy(bool connect_anyway) {
    // find PID of DF
    TRACE << "attempting to find running copy of DF by executable name";

    QStringList str_pids;
    QDirIterator iter("/proc", QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Readable | QDir::Executable);
    while (iter.hasNext()) {
        QString fn = iter.next();
        QFile file(QString("%1/comm").arg(fn));
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray comm = file.readAll().trimmed();
            if (comm == "dwarfort.exe" || comm == "Dwarf_Fortress") {
                str_pids << iter.fileName();
            }
        }
    }

    int count = str_pids.count();
    if (count) { //found it
        if (count > 1) {
            str_pids.sort();
            m_pid = QInputDialog::getItem(0, tr("Warning"),tr("Multiple Dwarf Fortress processes found, please choose the process to use."),str_pids,str_pids.count()-1,false).toInt();
        } else {
            m_pid = str_pids.at(0).toInt();
        }

        m_memory_file.setFileName(QString("/proc/%1/mem").arg(m_pid));

        TRACE << "USING PID:" << m_pid;
    } else {
        QMessageBox::warning(0, tr("Warning"),
                             tr("Unable to locate a running copy of Dwarf "
                                "Fortress, are you sure it's running?"));
        LOGW << "can't find running copy";
        m_is_ok = false;
        return m_is_ok;
    }

    m_inject_addr = unsigned(-1);
    m_alloc_start = 0;
    m_alloc_end = 0;

    m_loc_of_dfexe = QString("/proc/%1/exe").arg(m_pid);

    QString checksum = calculate_checksum();

    m_layout = get_memory_layout(checksum.toLower(), !connect_anyway);

    m_df_dir = QDir(QFileInfo(QString("/proc/%1/cwd").arg(m_pid)).symLinkTarget());
    LOGI << "Dwarf fortress path:" << m_df_dir.absolutePath();

    return m_is_ok || connect_anyway;
}

/* Support for executing system calls in the context of the game process. */

static const char nop_code[] = {
    /* This is the byte pattern used to pad function
       addresses to multiples of 16 bytes. It consists
       of RET and a sequence of NOPs. The NOPs are not
       supposed to be used, so they can be overwritten. */
    static_cast<char>(0xC3), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90)
};

static const char injection_code[] = {
    /* This is the injected pattern. It keeps the
       original RET, but adds:
           INT 80h
           INT 3h (breakpoint) */
    static_cast<char>(0xC3), static_cast<char>(0xCD), static_cast<char>(0x80), static_cast<char>(0xCC)
};

VIRTADDR DFInstanceLinux::find_injection_address()
{
    if (m_inject_addr != unsigned(-1))
        return m_inject_addr;

    const USIZE step = 0x8000; // 32K steps

    char buf[step];

    // This loop is expected to succeed on the first try
    // Assume that DF doesn't get -fPIE any time soon.
    for (VIRTADDR pos = 0x08048000; read_raw(pos, step, buf) == step; pos += step)
    {
        // Try searching for existing injection code
        char *ptr = static_cast<char *>(memmem(buf, step, injection_code, sizeof(injection_code)));
        // Try searching for empty space
        if (!ptr)
            ptr = static_cast<char *>(memmem(buf, step, nop_code, sizeof(nop_code)));

        if (ptr) {
            m_inject_addr = pos + ptr - buf;
            LOGD << "injection point found at" << hex << m_inject_addr;
            return m_inject_addr;
        }

        LOGI << "couldn't find injection point in" << step << "bytes after 0x08048000, trying again...";
    }

    return m_inject_addr = 0;
}

#if __WORDSIZE == 64
#define R_ESP  rsp
#define R_EIP  rip
#define R_ORIG_EAX orig_rax
#define R_SCID rax
#define R_ARG0 rbx
#define R_ARG1 rcx
#define R_ARG2 rdx
#define R_ARG3 rsi
#define R_ARG4 rdi
#define R_ARG5 rbp
#else
#define R_ESP  esp
#define R_EIP  eip
#define R_ORIG_EAX orig_eax
#define R_SCID eax
#define R_ARG0 ebx
#define R_ARG1 ecx
#define R_ARG2 edx
#define R_ARG3 esi
#define R_ARG4 edi
#define R_ARG5 ebp
#endif

qint32 DFInstanceLinux::remote_syscall(int syscall_id,
     qint32 arg0, qint32 arg1, qint32 arg2,
     qint32 arg3, qint32 arg4, qint32 arg5)
{
    /* Find the injection place; on failure bail out */
    VIRTADDR inj_addr = find_injection_address();
    if (!inj_addr)
        return -1;

    /* Save the current value of the main thread registers */
    struct user_regs_struct saved_regs, work_regs;

    if (ptrace(PTRACE_GETREGS, m_pid, 0, &saved_regs) == -1) {
        perror("ptrace getregs");
        LOGE << "Could not retrieve register information";
        return -1;
    }

    /* Prepare the injected code */
    char inj_area_save[sizeof(injection_code)];
    const USIZE inj_size = sizeof(injection_code);
    if (read_raw(inj_addr, inj_size, inj_area_save) != inj_size ||
        write_raw_ptrace(inj_addr, inj_size, injection_code) < inj_size) {
        LOGE << "Could not prepare the injection area";
        return -1;
    }

    /* Prepare the registers */
    VIRTADDR jump_eip = inj_addr+1; // skip the first RET

    work_regs = saved_regs;
    work_regs.R_EIP = jump_eip;
    work_regs.R_ORIG_EAX = -1; // clear the interrupted syscall state
    work_regs.R_SCID = syscall_id;
    work_regs.R_ARG0 = arg0;
    work_regs.R_ARG1 = arg1;
    work_regs.R_ARG2 = arg2;
    work_regs.R_ARG3 = arg3;
    work_regs.R_ARG4 = arg4;
    work_regs.R_ARG5 = arg5;

    /* Upload the registers. Note that after this point,
       if this process crashes before the context is
       restored, the game will immediately crash too. */
    if (ptrace(PTRACE_SETREGS, m_pid, 0, &work_regs) == -1) {
        perror("ptrace setregs");
        LOGE << "Could not set register information";
        return -1;
    }

    /* Run the thread until the breakpoint is reached */
    int status;
    if (ptrace(PTRACE_CONT, m_pid, 0, SIGCONT) != -1) {
        status = wait_for_stopped();
        // If the process is stopped for some reason, restart it
        while (WSTOPSIG(status) == SIGSTOP &&
               ptrace(PTRACE_CONT, m_pid, 0, SIGCONT) != -1)
            status = wait_for_stopped();
        if (WSTOPSIG(status) != SIGTRAP) {
            LOGE << "Stopped on" << WSTOPSIG(status) << "instead of SIGTRAP";
        }
    } else {
        LOGE << "Failed to run the call.";
    }

    /* Retrieve registers with the syscall result. */
    if (ptrace(PTRACE_GETREGS, m_pid, 0, &work_regs) == -1) {
        perror("ptrace getregs");
        LOGE << "Could not retrieve register information after stepping";
    }

    /* Restore the registers. */
    if (ptrace(PTRACE_SETREGS, m_pid, 0, &saved_regs) == -1) {
        perror("ptrace setregs");
        LOGE << "Could not restore register information";
    }

    /* Defuse the pending signal state:
       If this process crashes before it officially detaches
       from the game, the last signal will be delivered to it.
       If the signal is SIGSTOP, nothing irreversible happens.
       SIGTRAP on the other hand will crash it, losing all data.
       To avoid it, send SIGSTOP to the thread, and resume it
       to immediately be stopped again.
    */
    syscall(SYS_tkill, m_pid, SIGSTOP);

    if (ptrace(PTRACE_CONT, m_pid, 0, 0) == -1 ||
        (status = wait_for_stopped(), WSTOPSIG(status)) != SIGSTOP) {
        LOGE << "Failed to restore thread and stop.";
    }

    /* Restore the modified injection area (not really necessary,
       since it is supposed to be inside unused padding, but...) */
    if (write_raw_ptrace(inj_addr, inj_size, inj_area_save) != inj_size) {
        LOGE << "Could not restore the injection area";
    }

    if (VIRTADDR(work_regs.R_EIP) == jump_eip+3)
        return work_regs.R_SCID;
    else {
        LOGE << "Single step failed: EIP" << hex << work_regs.R_EIP << "; expected" << jump_eip+3;
        return -4095;
    }
}

/* Memory allocation for strings using remote mmap. */

VIRTADDR DFInstanceLinux::mmap_area(VIRTADDR start, int size) {
    VIRTADDR return_value = remote_syscall(192/*mmap2*/, start, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    // Raw syscalls can't use errno, so the error is in the result.
    // No valid return value can be in the -4095..-1 range.
    if (int(return_value) < 0 && int(return_value) >= -4095) {
        LOGE << "Injected MMAP failed with error: " << -return_value;
        return_value = -1;
    }

    return return_value;
}

VIRTADDR DFInstanceLinux::alloc_chunk(USIZE size) {
    if (size > 1048576) {
        return 0;
    }
    if ((m_alloc_end - m_alloc_start) < size) {
        int apages = (size*2 + 4095)/4096;
        int asize = apages*4096;

        // Try to request contiguous allocation as a hint
        VIRTADDR new_block = mmap_area(m_alloc_end, asize);
        if (new_block == VIRTADDR(-1))
            return 0;

        if (new_block != m_alloc_end)
            m_alloc_start = new_block;
        m_alloc_end = new_block + asize;
    }

    VIRTADDR rv = m_alloc_start;
    m_alloc_start += size;
    return rv;
}
