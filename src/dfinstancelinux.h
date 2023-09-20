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

class DFInstanceLinux : public DFInstance {
    Q_OBJECT
public:
    DFInstanceLinux(QObject *parent=0);
    virtual ~DFInstanceLinux();
    void find_running_copy();

    USIZE read_raw(const VIRTADDR addr, const USIZE bytes, void *buffer);
    QString read_string(const VIRTADDR addr);

    // Writing
    USIZE write_raw(const VIRTADDR addr, const USIZE bytes, const void *buffer);
    USIZE write_string(const VIRTADDR addr, const QString &str);

    int VM_TYPE_OFFSET() {return 0x5;}

    bool df_running();

    bool attach();
    bool detach();

protected:
    bool set_pid();

private:
    int wait_for_stopped();

    pid_t m_pid;
};

#endif // DFINSTANCE_LINUX_H
