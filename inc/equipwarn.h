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
#ifndef EQUIPWARN_H
#define EQUIPWARN_H

#include "global_enums.h"
#include <QPair>
#include <QVariant>
#include "item.h"

class Dwarf;

class EquipWarn : public QObject
{
    Q_OBJECT

public:
    EquipWarn(QObject *parent = 0);

    struct warn_info{
        ITEM_TYPE iType;
        QPair<QString,Item::ITEM_STATE> key; //item name, wear
        int count;
    };

    struct warn_count{
        int count;
        QMap<QString,QVariant> unit_ids;
    };

    int get_total_count() {return m_total_count;}
    QHash<Item::ITEM_STATE,int> get_wear_counts() {return m_wear_counts;}
    QHash<QPair<QString,Item::ITEM_STATE>, warn_count> get_details() {return m_details;}

    void add_detail(Dwarf *d, warn_info wi);

private:
    //list of items/wear and count info
    QHash<QPair<QString,Item::ITEM_STATE>, warn_count> m_details;
    QHash<Item::ITEM_STATE,int> m_wear_counts;
    int m_total_count;
};

#endif // EQUIPWARN_H
