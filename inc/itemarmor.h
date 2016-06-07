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
#ifndef ITEMARMOR_H
#define ITEMARMOR_H

#include "item.h"
#include "itemarmorsubtype.h"
#include "races.h"

class ItemArmor : public Item {
    Q_OBJECT
public:

    ItemArmor(const Item &baseItem)
        :Item(baseItem)
        , m_armor_def(0)
    {
        read_def();
    }

    ItemArmor(DFInstance *df, VIRTADDR item_addr)
        :Item(df,item_addr)
        , m_armor_def(0)
    {
        read_def();
    }

    ItemSubtype * get_subType(){return m_armor_def;}
    short item_subtype() const {return m_armor_def->subType();}

private:
    ItemArmorSubtype *m_armor_def;

    void read_def(){
        if(m_addr > 0){
            m_armor_def = new ItemArmorSubtype(m_iType,m_df,m_df->read_addr(m_addr+m_df->memory_layout()->item_offset("item_def")),this);
            if(m_armor_def){

                QString layer_name = m_armor_def->get_layer_name();
                if(layer_name != ""){
                    m_layer_name = layer_name;
                }

                if(m_armor_def->armor_flags().has_flag(ItemArmorSubtype::ARMOR_SHAPED)){
                    m_layer_name.append(tr("[S]"));
                }
            }
            if(m_maker_race >= 0 && m_maker_race != m_df->dwarf_race_id()){
                Race *r_maker = m_df->get_race(m_maker_race);
                if(r_maker){
                    int item_size = r_maker->adult_size();
                    int our_size = item_size;
                    Race *r_fort = m_df->get_race(m_df->dwarf_race_id());
                    if(r_fort){
                        our_size = r_fort->adult_size();
                    }
                    if(item_size > our_size){
                        m_size_prefix = tr("Large ");
                    }else if(item_size < our_size){
                        m_size_prefix = tr("Small ");
                    }
                }
            }
        }
    }
};

#endif // ITEMARMOR_H
