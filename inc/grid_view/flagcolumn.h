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

#ifndef FLAGCOLUMN_H
#define FLAGCOLUMN_H

#include "viewcolumn.h"

class FlagColumn : public ViewColumn {
public:
        FlagColumn(QString title, int bit_pos, bool bit_value, ViewColumnSet *set = 0, QObject *parent = 0);
    FlagColumn(QSettings &s, ViewColumnSet *set = 0, QObject *parent = 0);
    FlagColumn(const FlagColumn &to_copy); // copy ctor
    FlagColumn* clone() {return new FlagColumn(*this);}
        QStandardItem *build_cell(Dwarf *d);
        QStandardItem *build_aggregate(const QString &group_name, const QVector<Dwarf*> &dwarves);

        int bit_pos() {return m_bit_pos;}
        void set_bit_pos(int bit_pos) {m_bit_pos = bit_pos;}
        bool bit_value() {return m_bit_value;}
        void set_bit_value(bool bit_value) {m_bit_value = bit_value;}

        // override
        void write_to_ini(QSettings &s);

protected:
        int m_bit_pos;
        bool m_bit_value;
};

#endif // FLAGCOLUMN_H
