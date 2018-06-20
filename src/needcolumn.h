/*
Dwarf Therapist
Copyright (c) 2018 Clement Vuchener

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
#ifndef NEED_COLUMN_H
#define NEED_COLUMN_H

#include "viewcolumn.h"

class NeedColumn : public ViewColumn {
    Q_OBJECT
public:
    NeedColumn(QSettings &s, ViewColumnSet *set = 0, QObject *parent = 0);
    NeedColumn(QString title, int need_id, ViewColumnSet *set = 0, QObject *parent = 0);
    NeedColumn(const NeedColumn &to_copy); // copy ctor
    NeedColumn* clone() override {return new NeedColumn(*this);}
    QStandardItem *build_cell(Dwarf *d) override;
    QStandardItem *build_aggregate(const QString &group_name, const QVector<Dwarf*> &dwarves) override;

public slots:
    void refresh_sort(COLUMN_SORT_TYPE sType) override;

protected:
    void init_states() override;
    QColor get_state_color(int state) const override;
    void refresh_color_map() override;

private:
    int m_need_id;

    void refresh_sort(const Dwarf *d, QStandardItem *item);
};

#endif
