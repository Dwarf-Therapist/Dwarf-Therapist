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
#ifndef ITEMAMMOSUBTYPE_H
#define ITEMAMMOSUBTYPE_H

#include "utils.h"
#include "dfinstance.h"
#include "memorylayout.h"

class ItemAmmoSubtype : public QObject {
    Q_OBJECT
public:
    ItemAmmoSubtype(DFInstance *df, VIRTADDR address, QObject *parent = 0)
        : QObject(parent)
        , m_address(address)
        , m_df(df)
        , m_mem(df->memory_layout())
    {
        read_ammo();
    }

    virtual ~ItemAmmoSubtype(){
        m_df = 0;
        m_mem = 0;
    }

    static ItemAmmoSubtype* get_ammo(DFInstance *df, const VIRTADDR &address, QObject *parent = 0){
        return new ItemAmmoSubtype(df,address,parent);
    }

    VIRTADDR address() {return m_address;}
    QString name() const {return m_name;}
    QString name_plural() const {return m_name_plural;}
//    QString ammo_class() const {return m_ammo_class;}

    short subType() const {return m_subType;}

    void set_item_type(ITEM_TYPE i_type){
        m_iType=i_type;
    }

private:
    VIRTADDR m_address;
    QString m_name;
    QString m_name_plural;
    DFInstance * m_df;
    MemoryLayout * m_mem;
    ITEM_TYPE m_iType;
//    QString m_ammo_class;
    short m_subType;

    void read_ammo(){
        m_subType = m_df->read_short(m_address + m_df->memory_layout()->item_subtype_offset("sub_type"));
        m_name = capitalizeEach(m_df->read_string(m_address + m_df->memory_layout()->item_subtype_offset("name")));
        m_name_plural = capitalizeEach(m_df->read_string(m_address + m_df->memory_layout()->item_subtype_offset("name_plural")));
//        m_ammo_class = capitalizeEach(m_df->read_string(m_address + 0x74));
//        int size = m_df->read_int(m_address+0x98);
    }

};

#endif // ITEMAMMOSUBTYPE_H
