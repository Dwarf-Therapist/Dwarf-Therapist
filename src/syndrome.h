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
#ifndef SYNDROME_H
#define SYNDROME_H

#include "utils.h"
#include "global_enums.h"

#include <QString>

class DFInstance;
class MemoryLayout;

class Syndrome{

public:
    struct syn_att_change{
        int percent;
        int added;
        bool is_permanent;
    };

    Syndrome();
    Syndrome(DFInstance *df, VIRTADDR addr);
    virtual ~Syndrome();

    QStringList class_names() {return m_class_names;}
    QString name() {return m_name;}
    int id() {return m_id;}
    bool is_sickness() {return m_is_sickness;}

    QString display_name(bool show_name = true,bool show_class = true);
    QString syn_effects();

    QHash<ATTRIBUTES_TYPE, syn_att_change> get_attribute_changes() {return m_attribute_changes;}
    int get_transform_race(){return m_transform_race;}
    bool has_transformation(){return m_has_transform;}

    bool operator==(const Syndrome &other) const {
        if(this == &other)
            return true;
        return (this->m_id == other.m_id);
    }

private:
    DFInstance *m_df;
    MemoryLayout *m_mem;
    VIRTADDR m_addr;
    int m_transform_race;
    bool m_has_transform;
    bool m_is_sickness;
    QString m_name;
    QStringList m_class_names;
    QStringList m_syn_effects;
    int m_id;
//    int m_year;
//    int m_time;
    //pairs of percent change, and flat additive changes
    QHash<ATTRIBUTES_TYPE, syn_att_change> m_attribute_changes;

    void load_attribute_changes(VIRTADDR addr, int start_idx, int count, int add_offset, int end);

};

#endif // SYNDROME_H

