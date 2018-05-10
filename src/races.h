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
#ifndef RACES_H
#define RACES_H

#include <QCoreApplication>

#include "flagarray.h"
#include "global_enums.h"
#include "utils.h"

class DFInstance;
class MemoryLayout;
class Caste;
class Material;

class Race {
    Q_DECLARE_TR_FUNCTIONS(Race)
public:
    Race(DFInstance *df, VIRTADDR address,  int id);
    virtual ~Race();

    static Race* get_race(DFInstance *df, const VIRTADDR &address, int id);

    //! Return the memory address (in hex) of this race in the remote DF process
    VIRTADDR address() {return m_address;}

    int race_id() {return m_id;}
    const QString &name(int count = 1) const {return (count > 1 ? m_name_plural : m_name);}
    const QString &plural_name() const {return m_name_plural;}
    const QString &adjective() const {return m_adjective;}
    const QString &description() const {return m_description;}
    const QString &baby_name() const {return m_baby_name;}
    const QString &baby_name_plural() const {return m_baby_name_plural;}
    const QString &child_name() const {return m_child_name;}
    const QString &child_name_plural() const {return m_child_name_plural;}
    const QVector<QString> &pref_strings() const { return m_pref_strings; }
    VIRTADDR pop_ratio_vector() const {return m_pop_ratio_vector;}
    VIRTADDR castes_vector() const {return m_castes_vector;}
    Material *get_creature_material(int index);
    QHash<int, Material *> get_creature_materials();
    Caste *get_caste_by_id(int idx) { return m_castes.value(idx); }
    const Caste *get_caste_by_id(int idx) const { return m_castes.value(idx); }
    int adult_size();

    void load_data();
    const FlagArray &flags() const {return m_flags;}

    bool caste_flag(CASTE_FLAGS cf) const;

    void load_caste_ratios();

    VIRTADDR get_tissue_address(int index);
private:
    VIRTADDR m_address;

    int m_id;
    QString m_name;
    QString m_description;
    QString m_name_plural;
    QString m_adjective;
    QString m_baby_name;
    QString m_baby_name_plural;
    QString m_child_name;
    QString m_child_name_plural;
    QList<Caste*> m_castes;
    QHash<int, Material*> m_creature_mats;

    QVector<QString> m_pref_strings;
    VIRTADDR m_pop_ratio_vector;
    VIRTADDR m_castes_vector;

    QVector<VIRTADDR> m_materials_addr;
    QVector<VIRTADDR> m_tissues_addr;

    DFInstance * m_df;
    MemoryLayout * m_mem;
    FlagArray m_flags;

    bool loaded_stats;

    void read_race();
    void load_materials(int idx = -1);
};

#endif // RACES_H
