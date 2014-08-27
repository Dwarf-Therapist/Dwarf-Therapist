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

#ifndef DFINSTANCE_LINUX_H
#define DFINSTANCE_LINUX_H
#include "dfinstance.h"
#include "dwarf.h"

#include <QHash>

#define STRING_SIZE 256

class MemoryLayout;

class DFInstanceLinux : public DFInstance {
    Q_OBJECT
public:
    DFInstanceLinux(QObject *parent=0);
    virtual ~DFInstanceLinux();

    // factory ctor
    bool find_running_copy(bool connect_anyway = false);
    USIZE read_raw_ptrace(const VIRTADDR &addr, const USIZE &bytes, void *buffer);
    USIZE read_raw(const VIRTADDR &addr, const USIZE &bytes, void *buffer);
    using DFInstance::read_raw;
    QString read_string(const VIRTADDR &addr);

    // Writing
    USIZE write_raw_ptrace(const VIRTADDR &addr, const USIZE &bytes, const void *buffer);
    USIZE write_raw(const VIRTADDR &addr, const USIZE &bytes, const void *buffer);
    USIZE write_string(const VIRTADDR &addr, const QString &str);

    void map_virtual_memory();

    bool attach();
    bool detach();

protected:
    pid_t m_pid;
    QString calculate_checksum();

private:
    int wait_for_stopped();
    VIRTADDR find_injection_address();
    qint32 remote_syscall(int syscall_id,
                          qint32 arg0 = 0, qint32 arg1 = 0, qint32 arg2 = 0,
                          qint32 arg3 = 0, qint32 arg4 = 0, qint32 arg5 = 0);

    VIRTADDR mmap_area(VIRTADDR start, int size);
    VIRTADDR alloc_chunk(USIZE size);
    VIRTADDR get_string(const QString &str);

    QFile m_memory_file;
    MemorySegment *m_executable;
    VIRTADDR m_inject_addr;
    VIRTADDR m_alloc_start, m_alloc_end;
    QHash<QString, VIRTADDR> m_string_cache;
    bool m_warned_glibc;
};

#endif // DFINSTANCE_H
