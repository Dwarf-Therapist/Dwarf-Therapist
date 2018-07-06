/*
Dwarf Therapist
Copyright (c) 2010 Justin Ehlert

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

#include "itemammo.h"
#include "itemsubtype.h"

ItemAmmo::ItemAmmo(const Item &baseItem)
    : Item(baseItem)
    , m_ammo_def(0)
{
    read_def();
}

ItemAmmo::ItemAmmo(DFInstance *df, VIRTADDR item_addr)
    : Item(df,item_addr)
    , m_ammo_def(0)
{
    read_def();
}

ItemAmmo::~ItemAmmo()
{
    delete m_ammo_def;
}

short ItemAmmo::item_subtype() const {return m_ammo_def->subType();}
ItemSubtype * ItemAmmo::get_subType(){return m_ammo_def;}

void ItemAmmo::read_def(){
    if(m_addr){
        m_ammo_def = new ItemSubtype(m_iType,m_df, m_df->read_addr(m_df->memory_layout()->item_field(m_addr, "item_def")), this);
    }
}
