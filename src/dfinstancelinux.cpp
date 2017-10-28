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

SSIZE DFInstanceLinux::process_vm(long number, const VIRTADDR addr
                                  , const USIZE bytes, void *buffer) {
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

USIZE DFInstanceLinux::read_raw_ptrace(const VIRTADDR addr, const USIZE bytes, void *buffer) {
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

USIZE DFInstanceLinux::read_raw(const VIRTADDR addr, const USIZE bytes, void *buffer) {
    SSIZE bytes_read = process_vm(SYS_process_vm_readv, addr, bytes, buffer);
    if (bytes_read < 0) {
        memset(buffer, 0, bytes);

        if (errno != ENOSYS) {
            LOGE << "READ_RAW:" << QString(strerror(errno)) << "READING" << bytes << "BYTES FROM" << hexify(addr) << "TO" << buffer;
            return -1;
        }

        return read_raw_ptrace(addr, bytes, buffer);
    }

    TRACE << "Read" << bytes_read << "bytes of" << bytes << "bytes from" << hexify(addr) << "to" << buffer;

    if ((size_t)bytes_read < bytes)
        memset((char *)buffer + bytes_read, 0, bytes - bytes_read);

    return bytes_read;
}

USIZE DFInstanceLinux::write_raw_ptrace(const VIRTADDR addr, const USIZE bytes,
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

USIZE DFInstanceLinux::write_raw(const VIRTADDR addr, const USIZE bytes,
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

bool DFInstanceLinux::set_pid(){
    QSet<PID> pids;
    QDirIterator iter("/proc", QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable | QDir::Executable);
    while (iter.hasNext()) {
        QString fn = iter.next();
        QFile file(QString("%1/comm").arg(fn));
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray comm = file.readAll();
            if (comm == "dwarfort.exe\n" || comm == "Dwarf_Fortress\n") {
                pids << iter.fileName().toLong();
            }
        }
    }

    m_pid = select_pid(pids);

    if (!m_pid)
        return false;

    return true;
}

#define ELF_MAGIC "\x7f" "ELF"

void DFInstanceLinux::find_running_copy() {
    m_status = DFS_DISCONNECTED;
    // find PID of DF
    TRACE << "attempting to find running copy of DF by executable name";

    if(set_pid()){
        m_memory_file.setFileName(QString("/proc/%1/mem").arg(m_pid));
        TRACE << "USING PID:" << m_pid;
    }else{
        return;
    }

    m_inject_addr = unsigned(-1);
    m_alloc_start = 0;
    m_alloc_end = 0;

    m_loc_of_dfexe = QString("/proc/%1/exe").arg(m_pid);
    m_df_dir = QDir(QFileInfo(QString("/proc/%1/cwd").arg(m_pid)).symLinkTarget());
    LOGI << "Dwarf fortress path:" << m_df_dir.absolutePath();

    { // check class from ELF header to find the pointer size
        QFile exe(m_loc_of_dfexe);
        if (!exe.open(QIODevice::ReadOnly)) {
            LOGE << "Failed to open DF executable" << m_loc_of_dfexe;
            return;
        }
        auto header = exe.read(5);
        if (header == ELF_MAGIC "\x01")
            m_pointer_size = 4;
        else if (header == ELF_MAGIC "\x02")
            m_pointer_size = 8;
        else {
            LOGE << "invalid ELF header" << header;
            m_pointer_size = sizeof(VIRTADDR);
        }
        exe.close();
    }

    if (m_pointer_size > sizeof(VIRTADDR)) {
        LOGE << "pointers too big";
        return;
    }

    m_status = DFS_CONNECTED;
    set_memory_layout(calculate_checksum());
}

/* Support for executing system calls in the context of the game process. */

static const char syscall_code[] = {
    // SYSCALL
    static_cast<char>(0x0f), static_cast<char>(0x05)
};

static_assert(sizeof(syscall_code) <= sizeof(long), "syscall code must fit in long");

static const VIRTADDR df_exe_base = 0x00400000;

long DFInstanceLinux::remote_syscall(int syscall_id,
     long arg0, long arg1, long arg2,
     long arg3, long arg4, long arg5)
{
    attach();

    // get the old value of some RAM
    long old_code_word = ptrace(PTRACE_PEEKTEXT, m_pid, df_exe_base, 0);
    errno = 0;
    if (old_code_word == -1 && errno != 0) {
        LOGE << "could not retrieve old RAM for syscall";
        return -1;
    }

    // insert new text
    long syscall_code_word;
    memcpy(&syscall_code_word, syscall_code, sizeof(syscall_code));
    if (ptrace(PTRACE_POKETEXT, m_pid, df_exe_base, syscall_code_word) == -1) {
        LOGE << "could not insert syscall .text";
        return -1;
    }

    /* Save the current value of the main thread registers */
    struct user_regs_struct saved_regs, work_regs;

    if (ptrace(PTRACE_GETREGS, m_pid, 0, &saved_regs) == -1) {
        LOGE << "Could not retrieve original register information:" << strerror(errno);
        return -1;
    }

    /* Prepare the registers */
    work_regs = saved_regs;
    work_regs.rip = df_exe_base;
    work_regs.rax = syscall_id;
    work_regs.rdi = arg0;
    work_regs.rsi = arg1;
    work_regs.rdx = arg2;
    work_regs.r10 = arg3;
    work_regs.r8 = arg4;
    work_regs.r9 = arg5;

    /* Upload the registers. Note that after this point,
       if this process crashes before the context is
       restored, the game will immediately crash too. */
    if (ptrace(PTRACE_SETREGS, m_pid, 0, &work_regs) == -1) {
        LOGE << "Could not set register information:" << strerror(errno);
        return -1;
    }

    if (ptrace(PTRACE_SINGLESTEP, m_pid, 0, 0) == -1) {
        LOGE << "Failed to single step:" << strerror(errno);
        return -1;
    }

    wait_for_stopped();

    /* Retrieve registers with the syscall result. */
    if (ptrace(PTRACE_GETREGS, m_pid, 0, &work_regs) == -1) {
        LOGE << "Could not retrieve register information after stepping:" << strerror(errno);
        return -1;
    }

    /* Restore the registers. */
    if (ptrace(PTRACE_SETREGS, m_pid, 0, &saved_regs) == -1) {
        LOGE << "Could not restore register information:" << strerror(errno);
    }

    // restore the text
    if (ptrace(PTRACE_POKETEXT, m_pid, df_exe_base, old_code_word) == -1) {
        LOGE << "Could not restore the injection area:" << strerror(errno);
    }

    detach();

    return work_regs.rax;
}

/* Memory allocation for strings using remote mmap. */

VIRTADDR DFInstanceLinux::mmap_area(VIRTADDR start, USIZE size) {
    VIRTADDR return_value = remote_syscall(SYS_mmap, start, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

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
        if (int(new_block) == -1)
            return 0;

        if (new_block != m_alloc_end)
            m_alloc_start = new_block;
        m_alloc_end = new_block + asize;
    }

    VIRTADDR rv = m_alloc_start;
    m_alloc_start += size;
    return rv;
}
