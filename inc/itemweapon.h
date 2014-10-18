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
#ifndef ITEMWEAPON_H
#define ITEMWEAPON_H

#include "item.h"
#include "itemweaponsubtype.h"

class ItemWeaponSubtype;

class ItemWeapon : public Item {

public:
    ItemWeapon(const Item &baseItem)
        :Item(baseItem)
    {
        read_def();
    }

    ItemWeapon(DFInstance *df, VIRTADDR item_addr)
        :Item(df,item_addr)
    {
        read_def();
    }

    virtual ~ItemWeapon(){
        m_df = 0;
        m_weapon = 0;
    }

    ItemWeaponSubtype * get_details(){return m_weapon;}
    short item_subtype(){
        if(m_weapon)
            return m_weapon->subType();
        else
            return -1;
    }
    short melee_skill(){
        if(m_weapon)
            return m_weapon->melee_skill();
        else
            return -1;
    }
    short ranged_skill(){
        if(m_weapon)
            return m_weapon->ranged_skill();
        else
            return -1;
    }

private:
    ItemWeaponSubtype *m_weapon;

    void read_def(){
        if(m_addr > 0){
            m_weapon = new ItemWeaponSubtype(m_df,m_df->read_addr(m_addr+m_df->memory_layout()->item_offset("item_def")),this);
            m_item_name = m_weapon->name();
        }
    }
};
#endif // ITEMWEAPON_H
