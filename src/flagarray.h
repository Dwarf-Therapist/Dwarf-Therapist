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
#ifndef FLAGARRAY_H
#define FLAGARRAY_H

#include <QBitArray>
#include "utils.h"

class DFInstance;

class FlagArray
{
private:
    QBitArray m_flags;
    QHash<int,bool> m_flags_custom;
    DFInstance *m_df;

    QString output_flag(int f, bool val, bool active);

public:
    FlagArray();
    FlagArray(DFInstance *df, VIRTADDR base_addr);
    FlagArray(const FlagArray &f);
    virtual ~FlagArray();

    int count();
    bool has_flag(const int f);
    void set_flag(int f,bool state);
    QList<int> active_flags();

    QString output_flag_string(bool active = true);
};
#endif // FLAGARRAY_H
