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
    m_attach_count--;
    if (m_attach_count > 0) {
        TRACE << "NO NEED TO DETACH SKIPPING..." << m_attach_count;
        return true;
    }

    ptrace(PTRACE_DETACH, m_pid, 0, 0);
    TRACE << "FINISHED DETACH" << m_attach_count;
    return m_attach_count > 0;
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

struct Elf32
{
    using Ehdr = Elf32_Ehdr;
    using Shdr = Elf32_Shdr;
    using Sym = Elf32_Sym;
    using Rel = Elf32_Rel;
    using Rela = Elf32_Rela;

    static auto RSym(uint32_t info) { return ELF32_R_SYM(info); }
    static auto RType(uint32_t info) { return ELF32_R_TYPE(info); }

    static constexpr auto JumpSlot = R_386_JMP_SLOT;

    using Addr = Elf32_Addr;
    using Off = Elf32_Off;

};

struct Elf64
{
    using Ehdr = Elf64_Ehdr;
    using Shdr = Elf64_Shdr;
    using Sym = Elf64_Sym;
    using Rel = Elf64_Rel;
    using Rela = Elf64_Rela;

    static auto RSym(uint64_t info) { return ELF64_R_SYM(info); }
    static auto RType(uint64_t info) { return ELF64_R_TYPE(info); }

    static constexpr auto JumpSlot = R_X86_64_JUMP_SLOT;

    using Addr = Elf64_Addr;
    using Off = Elf64_Off;
};

template<typename Elf, typename Rel>
static uintptr_t search_jump_slot(std::ifstream &exe, const typename Elf::Shdr &reloc, unsigned int symbol_index)
{
    std::vector<Rel> reltab (reloc.sh_size / sizeof(Rel));
    exe.seekg(reloc.sh_offset);
    exe.read(reinterpret_cast<char *>(reltab.data()), reloc.sh_size);
    for (const auto &rel: reltab) {
        if (Elf::RSym(rel.r_info) == symbol_index) {
            if (Elf::RType(rel.r_info) == Elf::JumpSlot)
                return rel.r_offset;
            else {
                LOGE << "Unexpected relocation type:" << Elf::RType(rel.r_info)
                     << "for symbol" << symbol_index;
            }
        }
    }
    return 0;
}

template<typename Off>
static std::string read_string(std::ifstream &file, Off offset)
{
    std::string str;
    file.seekg(offset);
    while (auto c = file.get())
        str.push_back(c);
    return str;
}

template<typename Elf>
static typename Elf::Shdr read_section_header(std::ifstream &exe, const typename Elf::Ehdr &ehdr, uint16_t index)
{
    typename Elf::Shdr shdr;
    exe.seekg(ehdr.e_shoff + index * ehdr.e_shentsize);
    exe.read(reinterpret_cast<char *>(&shdr), sizeof(typename Elf::Shdr));
    return shdr;
}

template<typename Elf>
static uintptr_t find_relocation(std::ifstream &exe, const std::string &symbol)
{
    // Read headers
    typename Elf::Ehdr ehdr;
    exe.seekg(0);
    exe.read(reinterpret_cast<char *>(&ehdr), sizeof(ehdr));
    auto shstrtab_offset = read_section_header<Elf>(exe, ehdr, ehdr.e_shstrndx).sh_offset;
    std::map<std::string, typename Elf::Shdr> sections;
    for (uint16_t i = 0; i < ehdr.e_shnum; ++i) {
        auto shdr = read_section_header<Elf>(exe, ehdr, i);
        sections.emplace(read_string(exe, shstrtab_offset + shdr.sh_name), shdr);
    }
    // Search dynamic symbol
    auto dynstr_it = sections.find(".dynstr");
    auto dynsym_it = sections.find(".dynsym");
    int symbol_index = -1;
    if (dynstr_it != sections.end() && dynsym_it != sections.end()) {
        const auto &dynstr = dynstr_it->second;
        const auto &dynsym = dynsym_it->second;
        std::vector<typename Elf::Sym> symtab (dynsym.sh_size / sizeof(typename Elf::Sym));
        exe.seekg(dynsym.sh_offset);
        exe.read(reinterpret_cast<char *>(symtab.data()), dynsym.sh_size);
        for (auto i = 0u; i < symtab.size(); ++i) {
            const auto &sym = symtab[i];
            if (read_string(exe, dynstr.sh_offset + sym.st_name) == symbol) {
                symbol_index = i;
                break;
            }
        }
    }
    if (symbol_index < 0) {
        LOGE << "Symbol" << QString::fromStdString(symbol) << "not found";
        return 0;
    }
    // Search relocation sections
    uintptr_t addr = 0;
    for (const auto &p: sections) {
        const auto &section = p.second;
        if (section.sh_type == SHT_RELA)
            addr = search_jump_slot<Elf, typename Elf::Rela>(exe, section, symbol_index);
        else if (section.sh_type == SHT_REL)
            addr = search_jump_slot<Elf, typename Elf::Rel>(exe, section, symbol_index);
        if (addr)
            return addr;
    }
    LOGE << "Could not find relocation section containing symbol" << symbol_index;
    return 0;
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

static constexpr int TrapOpCode = 0xcc;

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
        VIRTADDR string_assign_relocation = 0;
        switch (get_elf_class(exe)) {
            case ELFCLASS32:
                m_pointer_size = 4;
                string_assign_relocation = find_relocation<Elf32>(exe, "_ZNSs6assignEPKcj");
                break;
            case ELFCLASS64:
                m_pointer_size = 8;
                string_assign_relocation = find_relocation<Elf64>(exe, "_ZNSs6assignEPKcm");
                break;
            default:
                LOGE << "invalid ELF header";
                m_pointer_size = sizeof(VIRTADDR);
        }
        if (!string_assign_relocation) {
            LOGE << "Failed to find std::string::assign relocation";
            m_string_assign_addr = 0;
        }
        else {
            m_string_assign_addr = read_addr(string_assign_relocation);
            LOGD << "found std::string::assign at" << hexify(m_string_assign_addr);
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

    // look for symbols and instructions
    std::regex procmaps_re("([0-9a-f]+)-([0-9a-f]+) ([-r][-w][-x][-p]) ([0-9a-f]+) ([0-9a-f]{2}):([0-9a-f]{2}) ([0-9]+)(?:\\s*(.+))");
    m_trap_addr = 0;
    if (auto maps = std::ifstream(proc_path + "/maps")) {
        std::string line;
        while (std::getline(maps, line)) {
            std::smatch res;
            if (!std::regex_search(line, res, procmaps_re))
                continue;
            VIRTADDR start = std::stoull(res[1], nullptr, 16);
            VIRTADDR end = std::stoull(res[2], nullptr, 16);

            // Scan executable sections for a trap instruction
            auto perm = res[3].str();
            if (m_trap_addr == 0 && perm.at(2) == 'x') {
                char buffer[PAGE_SIZE];
                for (VIRTADDR offset = start; offset < end; offset += PAGE_SIZE) {
                    if (read_raw(offset, PAGE_SIZE, buffer) != PAGE_SIZE) {
                        LOGE << "Failed to read executable section";
                        break;
                    }
                    char *res = reinterpret_cast<char *>(memchr(buffer, TrapOpCode, PAGE_SIZE));
                    if (res) {
                        m_trap_addr = offset+(res-buffer);
                        LOGD << "found trap instruction at" << hexify(m_trap_addr);
                        break;
                    }
                }
            }
            if (m_trap_addr != 0)
                break; // we found everything we needed
        }
    }
    else {
        LOGE << "Failed to open memory maps";
    }

    m_status = DFS_CONNECTED;
    set_memory_layout(md5);
}

bool DFInstanceLinux::df_running(){
    pid_t cur_pid = m_pid;
    return (set_pid() && cur_pid == m_pid);
}

static std::pair<VIRTADDR, std::size_t> check_string (DFInstance *df, VIRTADDR addr) {
    constexpr auto error_pair = std::make_pair<VIRTADDR, std::size_t> (0, 0);
    auto data_addr = df->read_addr(addr);
    if (!data_addr)
        return error_pair;
    auto pointer_size = df->pointer_size();
    std::vector<char> rep(3 * pointer_size);
    if (!df->read_raw(data_addr-rep.size(), rep.size(), rep.data()))
        return error_pair;
    std::size_t length = 0, capacity = 0;
    std::copy_n(&rep[0], pointer_size, reinterpret_cast<char *>(&length));
    std::copy_n(&rep[pointer_size], pointer_size, reinterpret_cast<char *>(&capacity));
    return length <= capacity ? std::make_pair(data_addr, length) : error_pair;
}

QString DFInstanceLinux::read_string(VIRTADDR addr) {
    auto data = check_string(this, addr);
    if (data.first) {
        std::vector<char> buffer(data.second);
        read_raw(data.first, buffer.size(), buffer.data());

        return QTextCodec::codecForName("IBM437")->toUnicode(buffer.data(), buffer.size());
    }
    else {
        LOGE << "Invalid string at" << hexify(addr);
        return QString ();
    }
}

USIZE DFInstanceLinux::write_string(const VIRTADDR addr, const QString &str) {
    if (m_string_assign_addr == 0) {
        LOGE << "Cannot write string without std::string::assign addresses.";
        return 0;
    }

    if (!check_string(this, addr).first) {
        LOGE << "Invalid string at" << hexify(addr);
        return 0;
    }

    auto call = make_function_call();
    if (!call || !*call) {
        LOGE << "Invalid function call object";
        return 0;
    }

    QByteArray str_data = QTextCodec::codecForName("IBM437")->fromUnicode(str);
    auto str_data_addr = call->push_data(str_data.data(), str_data.size());
    if (!str_data_addr) {
        LOGE << "Failed to write string in stack";
        return 0;
    }

    auto ret = call->call(m_string_assign_addr,
            {addr, str_data_addr, (unsigned)str_data.size()});
    return ret.first ? str.size() : 0;
}

class DFInstanceLinux::FunctionCall: public DFInstance::FunctionCall
{
#ifdef __x86_64__
    static constexpr auto sp = &user_regs_struct::rsp;
    static constexpr auto ip = &user_regs_struct::rip;
    static constexpr auto orig_ax = &user_regs_struct::orig_rax;
    static constexpr auto ip_offset = offsetof(user, regs.rip);
    static constexpr auto ax_offset = offsetof(user, regs.rax);
#else
    static constexpr auto sp = &user_regs_struct::esp;
    static constexpr auto ip = &user_regs_struct::eip;
    static constexpr auto orig_ax = &user_regs_struct::orig_eax;
    static constexpr auto ip_offset = offsetof(user, regs.eip);
    static constexpr auto ax_offset = offsetof(user, regs.eax);
#endif

    DFInstanceLinux *df;
    struct user_regs_struct saved_regs, work_regs;
public:
    FunctionCall(DFInstanceLinux *df)
        : df(df)
    {
        if (!df || !df->m_trap_addr) {
            df = nullptr; // set invalid state
            return;
        }

        if (!df->attach()) {
            LOGE << "Failed to attach to DF instance";
            df = nullptr; // set invalid state
            return;
        }

        /* Save the current value of the main thread registers */
        if (ptrace(PTRACE_GETREGS, df->m_pid, 0, &saved_regs) == -1) {
            auto errstr = strerror(errno);
            LOGE << "Could not retrieve original register information:" << errstr;
            df = nullptr; // set invalid state
            return;
        }

        work_regs = saved_regs;
        work_regs.*orig_ax = -1; // prevents syscall restart
        work_regs.*sp = saved_regs.*sp;

#ifdef __x86_64__
        if (df->m_pointer_size == 8) {
            // x86_64 ABI reserve a 128 byte "red zone" for storing local
            // variable without changing rsp
            work_regs.rsp -= 128;
        }
#endif
    }

    ~FunctionCall() override
    {
        if (df)
            df->detach();
    }

    VIRTADDR push_data(void *data, std::size_t len) override
    {
        if (!df) {
            LOGE << "Invalid function call object";
            return 0;
        }
        work_regs.*sp -= len;
        VIRTADDR data_addr = work_regs.*sp;
        if (df->write_raw(data_addr, len, data) != len) {
            LOGE << "Failed to write data in stack";
            return 0;
        }
        return data_addr;
    }

    std::pair<bool, VIRTADDR> call(VIRTADDR fn_addr, const std::vector<VIRTADDR> &args) override
    {
        static constexpr std::pair<bool, VIRTADDR> error = {false, 0};
        if (!df) {
            LOGE << "Invalid function call object";
            return error;
        }

        auto old_stack_pointer = work_regs.*sp;

        // prepare stack buffer with return address and arguments
        std::vector<char> stack;
        auto add_on_stack = [&stack, len=df->m_pointer_size] (VIRTADDR value) {
            std::copy_n(reinterpret_cast<char *>(&value), len, std::back_inserter(stack));
        };
        add_on_stack(df->m_trap_addr); // return address
        if (df->m_pointer_size == 4) { // cdecl
            for (const auto &arg: args)
                add_on_stack(arg);
        }
#ifdef __x86_64__
        else if (df->m_pointer_size == 8) { // SysV AMD64 ABI
            for (auto i = 0u; i < args.size(); ++i) {
                auto val = args[i];
                switch (i) {
                case 0: work_regs.rdi = val; break;
                case 1: work_regs.rsi = val; break;
                case 2: work_regs.rdx = val; break;
                case 3: work_regs.rcx = val; break;
                case 4: work_regs.r8 = val; break;
                case 5: work_regs.r9 = val; break;
                default: add_on_stack(val);
                }
            }
        }
#endif
        else {
            LOGE << "architecture not supported";
            return error;
        }

        work_regs.*sp -= stack.size()-df->m_pointer_size; // reserve space for args
        work_regs.*sp &= -16; // align on 16 bytes
        work_regs.*sp -= df->m_pointer_size; // space for return address

        // copy buffer in remote stack
        if (df->write_raw(work_regs.*sp, stack.size(), stack.data()) != stack.size()) {
            LOGE << "Failed to write arguments in stack";
            return error;
        }

        // call function
        work_regs.*ip = fn_addr;

        /* Upload the registers. Note that after this point,
           if this process crashes before the context is
           restored, the game will immediately crash too. */
        if (ptrace(PTRACE_SETREGS, df->m_pid, 0, &work_regs) == -1) {
            auto errstr = strerror(errno);
            LOGE << "Could not set register information:" << errstr;
            return error;
        }

        // Execute until the trap is reached
        VIRTADDR pc;
        do {
            if (ptrace(PTRACE_CONT, df->m_pid, 0, 0) == -1) {
                LOGE << "ptrace continue failed" << strerror(errno);
                break;
            }
            int status = df->wait_for_stopped();
            if (WSTOPSIG(status) != SIGTRAP) {
                LOGD << "std::string::assign interrupted by signal" << WSTOPSIG(status);
            }
            if (WSTOPSIG(status) == SIGSEGV) {
                LOGE << "Fatal error during remote function call.";
                LOGE << "Trying to restore DF state, but memory may be corrupted.";
                break;
            }
            // TODO: Handle more signals
            // TODO: Handle program termination
            // TODO: Catch exceptions
            auto ret = ptrace(PTRACE_PEEKUSER, df->m_pid, ip_offset, 0);
            if (ret == -1) {
                auto errstr = strerror(errno);
                LOGE << "ptrace getregs failed" << errstr;
                break;
            }
            pc = ret;
        } while (pc != df->m_trap_addr+1);


        auto ret_value = ptrace(PTRACE_PEEKUSER, df->m_pid, ax_offset, 0);
        if (ret_value == -1) {
            auto errstr = strerror(errno);
            LOGE << "ptrace getregs failed" << errstr;
        }

        /* Restore the registers. */
        if (ptrace(PTRACE_SETREGS, df->m_pid, 0, &saved_regs) == -1) {
            auto errstr = strerror(errno);
            LOGE << "Could not restore register information:" << errstr;
        }

        // Restore stack for next calls
        work_regs.*sp = old_stack_pointer;

        return {true, ret_value};
    }

    operator bool() const override
    {
        return df;
    }
};

std::unique_ptr<DFInstance::FunctionCall> DFInstanceLinux::make_function_call()
{
    return std::make_unique<FunctionCall>(this);
}

