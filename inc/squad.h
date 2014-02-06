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
#ifndef SQUAD_H
#define SQUAD_H

#include <QtWidgets>
#include "utils.h"
#include "global_enums.h"
//#include "itemdefuniform.h"

class Dwarf;
class Word;
class DFInstance;
class MemoryLayout;
class Uniform;

class Squad : public QObject {
    Q_OBJECT
public:
    Squad(DFInstance *df, VIRTADDR address, QObject *parent = 0);
    virtual ~Squad();

    static Squad* get_squad(DFInstance *df, const VIRTADDR &address);

    //! Return the memory address (in hex) of this creature in the remote DF process
    VIRTADDR address() {return m_address;}
    int id() {return m_id;}
    QString name() {return m_name;}
    //QVector<Dwarf *> members() {return m_members;}
    QVector<int> members() {return m_members;}
    int assigned_count();
    void refresh_data();

    void rename_squad(QString alias);
    int assign_to_squad(Dwarf *d);
    void remove_from_squad(Dwarf *d);

//    int get_equip_count(ITEM_TYPE itype,int position);
//    QHash<ITEM_TYPE,QList<ItemDefUniform*> > get_uniform(int position) {return m_uniform.value(position);}
    Uniform* get_uniform(int position){return m_uniforms.value(position);}

private:
    VIRTADDR m_address;
    int m_id;
    QString m_name;
    DFInstance * m_df;
    MemoryLayout * m_mem;    
    QVector<int> m_members;

    QVector<VIRTADDR> members_addr;

    QHash<int,Uniform*> m_uniforms;

    void read_id();
    void read_name();
    void read_members();
    void read_equip_category(VIRTADDR vec_addr, ITEM_TYPE itype, Uniform *u);
};

#endif
