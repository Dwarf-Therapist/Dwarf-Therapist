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
#ifndef ITEMWEAPONSUBTYPE_H
#define ITEMWEAPONSUBTYPE_H

#include <QtWidgets>
#include "utils.h"

class DFInstance;
class MemoryLayout;

class ItemWeaponSubtype : public QObject {
    Q_OBJECT
public:
    ItemWeaponSubtype(DFInstance *df, VIRTADDR address, QObject *parent = 0);
    virtual ~ItemWeaponSubtype();

    static ItemWeaponSubtype* get_weapon(DFInstance *df, const VIRTADDR &address, QObject *parent = 0);

    //! Return the memory address (in hex) of this weapon in the remote DF process
    VIRTADDR address() {return m_address;}

    QString name() const {return m_name;}
    QString name_plural() const {return m_name_plural;}
    void name_plural(const QString& new_name) {m_name_plural = new_name;}

    bool is_ranged() {return m_ammo.isEmpty() ? false : true;}

    int single_grasp() {return m_single_grasp_size;}
    int multi_grasp() {return  m_multi_grasp_size;}
    int melee_skill() {return m_melee_skill_id;}
    int ranged_skill() {return m_ranged_skill_id;}

    short subType() const {return m_subType;}

    void load_data();

    QString group_name;

private:
    VIRTADDR m_address;

    QString m_name;
    QString m_name_plural;
    int m_single_grasp_size;
    int m_multi_grasp_size;
    QString m_ammo;
    short m_subType;
    int m_melee_skill_id;
    int m_ranged_skill_id;

    DFInstance * m_df;
    MemoryLayout * m_mem;

    void read_weapon();
};

#endif // ITEMWEAPONSUBTYPE_H
