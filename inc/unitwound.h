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
#ifndef UNITWOUND_H
#define UNITWOUND_H

#include "truncatingfilelogger.h"
#include "utils.h"
#include "healthinfo.h"
#include "flagarray.h"

class MemoryLayout;
class DFInstance;
class UnitHealth;

class UnitWound
{
public:
    UnitWound();
    UnitWound(DFInstance *df, VIRTADDR base_addr, FlagArray caste_flags, UnitHealth *uh);
    UnitWound(DFInstance *df, int body_part_id, UnitHealth *uh);
    virtual ~UnitWound();

    QHash<QString,QList<HealthInfo*> > get_wound_details();

    struct wounded_part_details{
        QString body_part_name;
        int body_part_id;

        bool diagnosed;
        bool visible;

        quint32 wound_flags1;
        quint32 wound_flags2;

//        QVector<short> effect_perc_1;
//        QVector<short> effect_perc_2;
        QVector<short> effect_types;

        QHash<eHealth::H_INFO, QList<short> > wnd_info;

        int cur_pen;
        int pen_max;

        int strained_amount;
        int bleeding;
        int pain;
        int nauseous;
        int dizziness;
        int paralysis;
        int numbness;
        int swelling;
        int impaired;

        bool is_scarred;
        bool is_old_wound;

        QString layer_name;

        QString token;
    };

    QList<wounded_part_details> get_wounded_parts() {return m_wounded_parts;}

    bool is_critical() {return m_is_critical;}
private:
    DFInstance *m_df;
    VIRTADDR m_addr;
    UnitHealth *m_unitHealth;
    FlagArray m_caste_flags;

protected:
    bool m_severed, m_mortal, m_stuck, m_diagnosed, m_sutured, m_infection;

    QList<wounded_part_details> m_wounded_parts;

    //list of health info by body part
    QHash<QString,QList<HealthInfo*> > m_bp_info;

    void read_wound();
    void add_detail(wounded_part_details &wpd, eHealth::H_INFO id, bool idx0, bool idx1 = false, bool idx2 = false);
    bool m_is_critical;
};

#endif // UNITWOUND_H
