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
#ifndef DWARF_DETAILS_WIDGET_H
#define DWARF_DETAILS_WIDGET_H

#include <QtGui>

class Dwarf;

namespace Ui {
    class DwarfDetailsWidget;
}

class DwarfDetailsWidget: public QWidget {
    Q_OBJECT
public:
    DwarfDetailsWidget(QWidget *parent = 0, Qt::WindowFlags flags = 0);

public slots:
    void clear();
    void show_dwarf(Dwarf *d);

private:
    Ui::DwarfDetailsWidget *ui;
    QGridLayout *m_skills_layout;
//    QVector<QObject*> m_cleanup_list;
    QVector<QTableWidget*> m_tables;
    QByteArray m_splitter_sizes;

    int m_skill_sort_col;
    int m_skill_sort_desc;

    int m_attribute_sort_col;
    int m_attribute_sort_desc;

    int m_trait_sort_col;
    int m_trait_sort_desc;

    int m_role_sort_col;
    int m_role_sort_desc;

    int m_pref_sort_col;
    int m_pref_sort_desc;

    int m_current_id;

    void clear_table(QTableWidget &t);

};

#endif
