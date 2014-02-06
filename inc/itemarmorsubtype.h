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
#ifndef ITEMARMORSUBTYPE_H
#define ITEMARMORSUBTYPE_H

#include "utils.h"
#include "dfinstance.h"
#include "memorylayout.h"

class ItemArmorSubtype : public QObject {
    Q_OBJECT
public:
    ItemArmorSubtype(DFInstance *df, VIRTADDR address, QObject *parent = 0)
        : QObject(parent)
        , m_address(address)
        , m_df(df)
        , m_mem(df->memory_layout())
    {
        read_names();
    }

    static ItemArmorSubtype* get_armor(DFInstance *df, const VIRTADDR &address, QObject *parent = 0){
        return new ItemArmorSubtype(df,address,parent);
    }

    virtual ~ItemArmorSubtype(){
        m_df = 0;
        m_mem = 0;
    }

    VIRTADDR address() {return m_address;}
    QString name() const {return m_name;}
    QString name_plural() const {return m_name_plural;}
    int layer() const {return m_layer;}

    short subType() const {return m_subType;}

    QString get_layer_name(){
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

    void set_item_type(ITEM_TYPE i_type){
        m_iType=i_type;
        read_properties();
    }

private:
    VIRTADDR m_address;
    QString m_name;
    QString m_name_plural;
    DFInstance * m_df;
    MemoryLayout * m_mem;
    ITEM_TYPE m_iType;
    int m_layer;
    short m_subType;

    void read_names(){
        m_subType = m_df->read_short(m_address + m_df->memory_layout()->item_subtype_offset("sub_type"));
        m_name = capitalizeEach(m_df->read_string(m_address + m_df->memory_layout()->item_subtype_offset("name")));
        m_name_plural = capitalizeEach(m_df->read_string(m_address + m_df->memory_layout()->item_subtype_offset("name_plural")));
    }
    void read_properties(){
        int base_addr = find_property_offset();
        if(base_addr != 0x0)
            m_layer = m_df->read_int(m_address + base_addr+m_df->memory_layout()->armor_subtype_offset("layer"));
        else
            m_layer = -1;
    }

    int find_property_offset(){
        switch(m_iType){
        case ARMOR:
        {
            return m_df->memory_layout()->armor_subtype_offset("chest_armor_properties");
        }break;
        case HELM:
        {
            return m_df->memory_layout()->armor_subtype_offset("other_armor_properties");
        }break;
        case GLOVES:
        {
            return m_df->memory_layout()->armor_subtype_offset("other_armor_properties");
        }break;
        case PANTS:
        {
            return m_df->memory_layout()->armor_subtype_offset("pants_armor_properties");
        }break;
        case SHOES:
        {
            return m_df->memory_layout()->armor_subtype_offset("other_armor_properties");
        }break;
        default:
            return 0x0;
        }
    }
};

#endif // ITEMARMORSUBTYPE_H
