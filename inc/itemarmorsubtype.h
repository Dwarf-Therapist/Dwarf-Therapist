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

#include "itemsubtype.h"
#include "global_enums.h"

class ItemArmorSubtype : public ItemSubtype {
    Q_OBJECT
public:
    ItemArmorSubtype(const ITEM_TYPE itype, DFInstance *df, const VIRTADDR address, QObject *parent = 0);
    ~ItemArmorSubtype();

    typedef enum {
        ARMOR_METAL=2,
        ARMOR_BONE=3,
        ARMOR_SHELL=4,
        ARMOR_SHAPED=6,
        ARMOR_CHAIN=7
    } ARMOR_FLAGS;

    int layer() const {return m_layer;}
    QString get_layer_name();
    FlagArray armor_flags(){return m_armor_flags;}

private:
    int m_layer;
    FlagArray m_armor_flags;
    quint8 m_armor_level;

    int m_offset_props;
    int m_offset_level;

    void set_base_offsets();
    void read_data();
};
#endif // ITEMARMORSUBTYPE_H
