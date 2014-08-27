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
#ifndef DFINSTANCE_OSX_H
#define DFINSTANCE_OSX_H
#include <mach/vm_region.h>
#include <mach/vm_map.h>

#include "dfinstance.h"
#include "dwarf.h"

#include <QHash>

class MemoryLayout;

class DFInstanceOSX : public DFInstance {
    Q_OBJECT
public:
    DFInstanceOSX(QObject *parent=0);
    virtual ~DFInstanceOSX();

    // factory ctor
    bool find_running_copy(bool connect_anyway = false);
    USIZE read_raw(const VIRTADDR &addr, const USIZE &bytes, void *buffer);
    using DFInstance::read_raw;
    QString read_string(const VIRTADDR &addr);

    // Writing
    USIZE write_raw(const VIRTADDR &addr, const USIZE &bytes, const void *buffer);
    USIZE write_string(const VIRTADDR &addr, const QString &str);

    void map_virtual_memory();

    bool attach();
    bool detach();

    static bool isAuthorized();
    static bool checkPermissions();

protected:
    QString calculate_checksum();
    vm_map_t m_task;
    QString m_loc_of_dfexe;

private:
    uintptr_t get_string(const QString &str);
    QHash<QString, uintptr_t> m_string_cache;

    VIRTADDR alloc_chunk(mach_vm_size_t size);
    VIRTADDR m_alloc_start, m_alloc_end;
    int m_alloc_remaining, m_size_allocated;
};

#endif // DFINSTANCE_H
