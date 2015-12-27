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

#include <QObject>
#include "utils.h"
#include "global_enums.h"

class Dwarf;
class DFInstance;
class MemoryLayout;
class Uniform;
class QTreeWidgetItem;

class Squad : public QObject {
    Q_OBJECT
public:
    Squad(int id, DFInstance *df, VIRTADDR address, QObject *parent = 0);
    virtual ~Squad();

    //! Return the memory address (in hex) of this creature in the remote DF process
    VIRTADDR address() {return m_address;}
    int id() {return m_id;}
    QString name() {return m_name;}
    int assigned_count();

    void rename_squad(QString alias);
    void assign_to_squad(Dwarf *d, bool committing = false);
    bool remove_from_squad(Dwarf *d, bool committing = false);
    Uniform* get_uniform(int position){return m_uniforms.value(position);}

    QTreeWidgetItem* get_pending_changes_tree();
    void commit_pending();
    void clear_pending();
    int pending_changes();

    QPair<int,QString> get_order(int histfig_id);

    typedef enum{
        ORD_MOVE,
        ORD_KILL,
        ORD_DEFEND,
        ORD_PATROL,
        ORD_TRAIN
    } SQ_ORDER_TYPE;

private:
    VIRTADDR m_address;
    int m_id;
    QString m_name;
    DFInstance * m_df;
    MemoryLayout * m_mem;
    //! position, dwarf hist_id
    QMap<int,int> m_members;
    QVector<VIRTADDR> m_members_addr;
    QHash<int,Uniform*> m_uniforms;
    bool m_inactive;
    QString m_pending_name;

    int m_squad_order;
    QHash<int,int> m_orders; //histfig_id, job_id

    void read_data();
    void read_id();
    void read_name();
    void read_members();
    void read_orders();
    void read_equip_category(VIRTADDR vec_addr, ITEM_TYPE itype, Uniform *u);
    int find_position(int hist_id);

    void read_order(VIRTADDR addr, int histfig_id, bool unit = true);

signals:
    void squad_leader_changed();
};

#endif
