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

#include <QObject>
#include "global_enums.h"
#include "utils.h"
#include "flagarray.h"

class DFInstance;
class MemoryLayout;

class ItemArmorSubtype : public QObject {
    Q_OBJECT
public:
    ItemArmorSubtype(ITEM_TYPE itype, DFInstance *df, VIRTADDR address, QObject *parent = 0);
    virtual ~ItemArmorSubtype();

    typedef enum {
        ARMOR_METAL=2,
        ARMOR_BONE=3,
        ARMOR_SHELL=4
    } ARMOR_FLAGS;

    VIRTADDR address() {return m_address;}
    QString name() const {return m_name;}
    QString name_plural() const {return m_name_plural;}
    int layer() const {return m_layer;}
    bool clothing_use() const {return m_clothing;}
    bool armor_use() const {return m_armor;}
    short subType() const {return m_subType;}

    FlagArray flags() const {return m_flags;}
    QString get_layer_name();

private:
    VIRTADDR m_address;
    QString m_name;
    QString m_name_plural;
    DFInstance * m_df;
    MemoryLayout * m_mem;
    ITEM_TYPE m_iType;
    int m_layer;
    short m_subType;
    FlagArray m_flags;
    bool m_clothing;
    bool m_armor;
    quint8 m_armor_level;

    int m_offset_props;
    int m_offset_adj;
    int m_offset_preplural;
    int m_offset_level;
    int m_offset_mat;

    void read_names();
    void read_properties();
    void set_offsets();
};
#endif // ITEMARMORSUBTYPE_H
