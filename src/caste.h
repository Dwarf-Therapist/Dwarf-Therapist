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
#ifndef CASTE_H
#define CASTE_H

#include <QObject>
#include "utils.h"
#include "global_enums.h"
#include "flagarray.h"

class DFInstance;
class MemoryLayout;
class BodyPart;
class Race;

class Caste : public QObject {
    Q_OBJECT
public:
    Caste(DFInstance *df, VIRTADDR address, Race *r, QObject *parent = 0);
    virtual ~Caste();

    //! Return the memory address (in hex) of this creature in the remote DF process
    VIRTADDR address() {return m_address;}

    struct att_range{
        QList<int> raw_bins;
        QList<int> display_bins;
    };

    QString name(int count = 1) {return (count > 1 ? m_name_plural : m_name);}
    QString name_plural() {return m_name_plural;}
    QString tag() {return m_tag;}
    QString description();
    QPair<int,QString> get_attribute_descriptor_info(ATTRIBUTES_TYPE id, int value);
    int get_attribute_cost_to_improve(int id);

    int get_skill_rate(int skill_id);

    int child_age() {return m_child_age;}
    int baby_age() {return m_baby_age;}

    void load_data();
    void load_skill_rates();

    FlagArray flags() {return m_flags;}

    QList<int> get_attribute_raws(int attrib_id) {return m_attrib_ranges.value(attrib_id).raw_bins;}
    Caste::att_range get_attribute_range(int attrib_id) {return m_attrib_ranges.value(attrib_id);}

    void load_attribute_info();

    BodyPart *get_body_part(int body_part_id);

    bool is_geldable();
    int adult_size(){return m_adult_size;}

private:
    VIRTADDR m_address;
    Race *m_race;
    QString m_tag;
    QString m_name;
    QString m_name_plural;
    QString m_description;
    int m_baby_age;
    int m_child_age;
    int m_can_geld;
    int m_adult_size;

    QHash<int,att_range> m_attrib_ranges;

    DFInstance * m_df;
    MemoryLayout * m_mem;

    FlagArray m_flags;

    QHash<int,float> m_skill_rates;
    QStringList m_bonuses;

    //holds the trait id, and a vector of 3 values (min,median,max)
    QHash<int,QList<short> > m_trait_ranges;

    //attribute id, cost to improve (from attribute rates)
    QHash<int,int> m_attrib_costs;

    void read_caste();

    VIRTADDR m_body_addr;
    QVector<VIRTADDR> m_body_parts_addr;
    QHash<int, BodyPart*> m_body_parts;

};

#endif // CASTE_H
