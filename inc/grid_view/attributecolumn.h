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
#ifndef ATTRIBUTE_COLUMN_H
#define ATTRIBUTE_COLUMN_H

#include "viewcolumn.h"
#include "dwarf.h"

class AttributeColumn : public ViewColumn {
    Q_OBJECT
public:
    typedef enum {
        DTA_STRENGTH,
        DTA_AGILITY,
        DTA_TOUGHNESS
    } DWARF_ATTRIBUTE_TYPE;

    AttributeColumn(const QString &title, DWARF_ATTRIBUTE_TYPE type, ViewColumnSet *set = 0, QObject *parent = 0);
    AttributeColumn(QSettings &s, ViewColumnSet *set = 0, QObject *parent = 0);
    AttributeColumn(const AttributeColumn &to_copy); // copy ctor
    AttributeColumn* clone() {return new AttributeColumn(*this);}
    QStandardItem *build_cell(Dwarf *d);
    QStandardItem *build_aggregate(const QString &group_name, const QVector<Dwarf*> &dwarves);

    DWARF_ATTRIBUTE_TYPE attribute() {return m_attribute_type;}
    void set_attribute(DWARF_ATTRIBUTE_TYPE type) {m_attribute_type = type;}

    //override
    void write_to_ini(QSettings &s) {ViewColumn::write_to_ini(s); s.setValue("attribute", m_attribute_type);}

private:
    DWARF_ATTRIBUTE_TYPE m_attribute_type;
};

#endif
