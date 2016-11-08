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
#ifndef UNIFORM_H
#define UNIFORM_H

#include <QObject>
#include "global_enums.h"
#include "utils.h"

class DFInstance;
class ItemDefUniform;
class Item;

class Uniform : public QObject {
    Q_OBJECT
public:
    Uniform(DFInstance *df, QObject *parent = 0);
    virtual ~Uniform();

    int get_missing_equip_count(ITEM_TYPE itype);

    QHash<ITEM_TYPE,QList<ItemDefUniform*> > get_uniform() {return m_uniform_items;}
    QHash<ITEM_TYPE,QList<ItemDefUniform*> > get_missing_items(){return m_missing_items;}
    float get_uniform_rating(ITEM_TYPE itype);
    void check_uniform(QString category_name, Item *item_inv);

    void add_equip_count(ITEM_TYPE itype, int count);

    void add_uniform_item(VIRTADDR ptr, ITEM_TYPE itype, int count=1);
    void add_uniform_item(ITEM_TYPE itype, short sub_type, short job_skill, int count=1);
    void add_uniform_item(ITEM_TYPE itype, short sub_type, QList<int> job_skills, int count=1);
    void add_uniform_item(ITEM_TYPE itype, ItemDefUniform *uItem, int count=1);

    bool has_items(){return m_uniform_items.count();}

    void clear();

protected:
    DFInstance *m_df;

    QHash<ITEM_TYPE,int> m_equip_counts; //the original equipment counts for the uniform, and group counts
    QHash<ITEM_TYPE,int> m_missing_counts; //same as equip counts but for missing items

    QHash<ITEM_TYPE,QList<ItemDefUniform*> > m_uniform_items;
    QHash<ITEM_TYPE,QList<ItemDefUniform*> > m_missing_items;

    bool m_first_check;

    int get_remaining_required(ITEM_TYPE itype);
    void load_missing_counts();

};

#endif // UNIFORM_H
