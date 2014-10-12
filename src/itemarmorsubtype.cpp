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
#include "flagarray.h"

ItemArmorSubtype::ItemArmorSubtype(ITEM_TYPE itype, DFInstance *df, VIRTADDR address, QObject *parent)
    : QObject(parent)
    , m_address(address)
    , m_df(df)
    , m_mem(df->memory_layout())
    , m_iType(itype)
    , m_clothing(false)
    , m_armor(false)
    , m_offset_props(0x0)
    , m_offset_adj(0x0)
    , m_offset_level(0x0)
    , m_offset_mat(0x0)
{
    m_subType = m_df->read_short(m_address + m_mem->item_subtype_offset("sub_type"));
    set_offsets();
    read_names();
    read_properties();
}

ItemArmorSubtype::~ItemArmorSubtype(){
    m_df = 0;
    m_mem = 0;
}

QString ItemArmorSubtype::get_layer_name(){
    switch(m_layer){
    case 0:
    {
        return tr("Under");
    }break;
    case 1:
    {
        return tr("Over");
    }break;
    case 2:
    {
        return tr("Armor");
    }break;
    case 3:
    {
        return tr("Cover");
    }break;
    default:
        return "";
    }
}

void ItemArmorSubtype::read_names(){
    QString mat_name;

    if(m_offset_mat != 0x0)
        mat_name = m_df->read_string(m_address + m_offset_mat);

    QStringList name_parts;
    name_parts.append(m_df->read_string(m_address + m_offset_adj));
    name_parts.append(mat_name);
    name_parts.append(m_df->read_string(m_address + m_mem->item_subtype_offset("name")));
    m_name = capitalizeEach(name_parts.join(" ")).simplified().trimmed();

    name_parts.removeLast();
    name_parts.append(m_df->read_string(m_address + m_mem->item_subtype_offset("name_plural")));
    m_name_plural = capitalizeEach(name_parts.join(" ")).simplified().trimmed();
}

void ItemArmorSubtype::read_properties(){
    if(m_offset_props != 0x0){
        m_layer = m_df->read_int(m_address + m_offset_props + m_mem->armor_subtype_offset("layer"));

        if(m_offset_level != 0x0){
            m_armor_level = m_df->read_byte(m_address + m_offset_level);
        }else{
            m_armor_level = 0;
        }
        m_clothing = (m_armor_level == 0);

        //if it has an armor level over 0 or can be made from metal, bone or shell, consider it armor
        m_flags = FlagArray(m_df, m_address+m_offset_props);
        m_armor = (m_armor_level > 0 ||
                   (m_flags.has_flag(ARMOR_METAL) || m_flags.has_flag(ARMOR_BONE) || m_flags.has_flag(ARMOR_SHELL)));

        if(!m_clothing && !m_armor)
            qDebug() << m_name_plural << "is neither armor nor clothing!";

        qDebug() << m_name_plural << m_flags.output_flag_string() << "armor level" << m_armor_level << "is clothing:" << m_clothing << "is armor:" << m_armor;
    }else{
        m_layer = -1;
        m_flags = FlagArray();
        m_clothing = true;
        qDebug() << "FAILED TO READ ARMOR PROPERTIES" << m_name;
    }
}

void ItemArmorSubtype::set_offsets(){
    m_offset_adj = m_mem->item_subtype_offset("adjective");

    switch(m_iType){
    case ARMOR: case PANTS:
    {
        //armor can also have a preplural (eg. suits of leather armor) but it's only covering chest/torso
        if(m_iType == ARMOR){
            m_offset_props = m_mem->armor_subtype_offset("chest_armor_properties");
        }else{
            m_offset_props = m_mem->armor_subtype_offset("pants_armor_properties");
        }
        m_offset_level = 0x00d0;
        m_offset_adj = 0x00b0;
        m_offset_mat = m_mem->armor_subtype_offset("mat_name");
    }
        break;
    case HELM: case GLOVES: case SHOES:
    {
        m_offset_props = m_mem->armor_subtype_offset("other_armor_properties");
        m_offset_level = 0x0098;
    }
        break;
    default:
        qDebug() << "skipping offsets for" << m_iType;
        break;
    }
}

