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

#include "itemarmorsubtype.h"
#include "dfinstance.h"
#include "memorylayout.h"
#include "truncatingfilelogger.h"

ItemArmorSubtype::ItemArmorSubtype(const ITEM_TYPE itype, DFInstance *df, const VIRTADDR address, QObject *parent)
    : ItemSubtype(itype,df,address,parent)
{
    switch(m_iType){
    case ARMOR:
        m_offset_props = m_mem->armor_subtype_offset("chest_armor_properties");
        m_offset_level = m_mem->armor_subtype_offset("armor_level");
        break;
    case PANTS:
        m_offset_props = m_mem->armor_subtype_offset("pants_armor_properties");
        m_offset_level = m_mem->armor_subtype_offset("armor_level");
        break;
    case HELM:
    case GLOVES:
    case SHOES:
        m_offset_props = m_mem->armor_subtype_offset("other_armor_properties");
        m_offset_level = m_mem->armor_subtype_offset("other_armor_level");
        break;
    default:
        m_offset_props = -1;
        m_offset_level = -1;
        break;
    }

    if(m_offset_props != -1){
        m_layer = m_df->read_int(m_mem->armor_subtype_field(m_address + m_offset_props, "layer"));

        if(m_offset_level != -1){
            m_armor_level = m_df->read_byte(m_address + m_offset_level);
        }else{
            m_armor_level = 0;
        }
        if(m_armor_level == 0){
            m_flags.set_flag(ITEM_IS_CLOTHING,true);
        }

        //if it has an armor level over 0 or can be made from metal, bone or shell, consider it armor
        m_armor_flags = FlagArray(m_df, m_address+m_offset_props);
        if(m_armor_level > 0 ||
                (m_armor_flags.has_flag(ARMOR_METAL) || m_armor_flags.has_flag(ARMOR_BONE) || m_armor_flags.has_flag(ARMOR_SHELL))){
            m_flags.set_flag(ITEM_IS_ARMOR,true);
        }

        if(m_armor_flags.has_flag(ARMOR_CHAIN)){
            m_name.prepend(tr("Chain "));
            m_name_plural.prepend(tr("Chain "));
        }

        if(!m_flags.has_flag(ITEM_IS_ARMOR) && !m_flags.has_flag(ITEM_IS_CLOTHING)){
            LOGW << m_name_plural << "are neither armor nor clothing. setting to clothing by default...";
            m_flags.set_flag(ITEM_IS_CLOTHING,true);
        }

    }else{
        m_layer = -1;
        m_armor_flags = FlagArray();
        if(m_iType != SHIELD){
            LOGW << "Failed to read armor properties" << m_name;
        }
    }
}

ItemArmorSubtype::~ItemArmorSubtype(){
}

QString ItemArmorSubtype::get_layer_name(){
    switch(m_layer){
    case 0:
        return tr("Under");
    case 1:
        return tr("Over");
    case 2:
        return tr("Armor");
    case 3:
        return tr("Cover");
    default:
        return "";
    }
}
