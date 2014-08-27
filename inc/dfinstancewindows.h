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
#ifndef DFINSTANCE_WINDOWS_H
#define DFINSTANCE_WINDOWS_H
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tlhelp32.h>
#include "dfinstance.h"

class Dwarf;
class MemoryLayout;

class DFInstanceWindows : public DFInstance {
    Q_OBJECT
public:
    DFInstanceWindows(QObject *parent=0);
    virtual ~DFInstanceWindows();

    // factory ctor
    bool find_running_copy(bool connect_anyway = false);

    QString get_last_error();

    USIZE read_raw(const VIRTADDR &addr, const USIZE &bytes, void *buffer);
    QString read_string(const VIRTADDR &addr);

    // Writing
    USIZE write_raw(const VIRTADDR &addr, const USIZE &bytes, const void *buffer);
    USIZE write_string(const VIRTADDR &addr, const QString &str);

    // pure virtual methods
    void map_virtual_memory();

    // windows doesn't really have a concept of
    // attaching/detaching from the process like Linux does, so just
    // make them no-ops
    bool attach(){return true;}
    bool detach(){return true;}


protected:
    // handy util methods
    QString calculate_checksum();

    HWND m_hwnd;
    HANDLE m_proc;

    IMAGE_DOS_HEADER m_dos_header;
    IMAGE_NT_HEADERS m_pe_header;
};

#endif // DFINSTANCE_H
