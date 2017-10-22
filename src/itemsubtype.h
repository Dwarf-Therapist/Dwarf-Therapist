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
#ifndef ITEMSUBTYPE_H
#define ITEMSUBTYPE_H

#include <QObject>
#include "utils.h"
#include "global_enums.h"
#include "dfinstance.h"
#include "memorylayout.h"
#include "flagarray.h"

class ItemSubtype : public QObject {
    Q_OBJECT
public:
    ItemSubtype(ITEM_TYPE itype, DFInstance *df, VIRTADDR address, QObject *parent = 0)
        : QObject(parent)
        , m_address(address)
        , m_df(df)
        , m_mem(df->memory_layout())
        , m_iType(itype)
        , m_subType(-1)
    {
        set_base_offsets();
    }

    ItemSubtype(DFInstance *df, VIRTADDR address, QObject *parent = 0)
        : QObject(parent)
        , m_address(address)
        , m_df(df)
        , m_mem(df->memory_layout())
        , m_iType(NONE)
        , m_subType(-1)
    {
        set_base_offsets();
    }

    virtual ~ItemSubtype(){
        m_df = 0;
        m_mem = 0;
    }

    VIRTADDR address() {return m_address;}
    QString name() const {return m_name;}
    QString name_plural() const {return m_name_plural;}
    ITEM_TYPE type() const {return m_iType;}
    short subType() const {return m_subType;}
    FlagArray flags() const {return m_flags;}

    void name(QString val){m_name = val;}
    void name_plural(QString val){m_name_plural = val;}

protected:
    VIRTADDR m_address;
    QString m_name;
    QString m_name_plural;
    DFInstance * m_df;
    MemoryLayout * m_mem;
    ITEM_TYPE m_iType;
    short m_subType;
    FlagArray m_flags;

    int m_offset_adj;
    int m_offset_preplural;
    int m_offset_mat;

    virtual void read_data();
    virtual void set_base_offsets();
};

#endif // ITEMSUBTYPE_H
