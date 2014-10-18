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
#ifndef ITEMAMMO_H
#define ITEMAMMO_H

#include "item.h"
#include "itemgenericsubtype.h"

class ItemAmmo : public Item {
public:

    ItemAmmo(const Item &baseItem)
        : Item(baseItem)
        , m_ammo_def(0)
    {
        read_def();
    }

    ItemAmmo(DFInstance *df, VIRTADDR item_addr)
        : Item(df,item_addr)
        , m_ammo_def(0)
    {
        read_def();
    }

    ~ItemAmmo()
    {
        delete m_ammo_def;
    }

    short item_subtype(){return m_ammo_def->subType();}

private:
    ItemGenericSubtype *m_ammo_def;

    void read_def(){
        if(m_addr){
            m_ammo_def = new ItemGenericSubtype(m_iType,m_df, m_df->read_addr(m_addr + m_df->memory_layout()->item_offset("item_def")), this);
            if(m_stack_size <= 1)
                m_item_name = m_ammo_def->name();
            else
                m_item_name = m_ammo_def->name_plural();
        }
    }
};

#endif // ITEMAMMO_H
