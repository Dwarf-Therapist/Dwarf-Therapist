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
#ifndef UNITHEALTH_H
#define UNITHEALTH_H

#include "utils.h"
#include "memorylayout.h"
#include "truncatingfilelogger.h"
#include "global_enums.h"

#include "healthinfo.h"
#include "bodypart.h"
#include "bodypartdamage.h"
#include "unitwound.h"
#include "healthcategory.h"

class MemoryLayout;
class DFInstance;

class UnitHealth
{
public:
    UnitHealth();
    UnitHealth(DFInstance *df, Dwarf *d, bool req_diagnosis);
    virtual ~UnitHealth();
    static void cleanup();

    static QStringList get_all_category_desc(eHealth::H_INFO hs, bool symbol_only = false, bool colored = true);

    HealthInfo *get_health_info(eHealth::H_INFO hs, short idx = 0);

    static void load_health_descriptors(QSettings &s);

    QStringList get_treatment_summary(bool colored, bool symbols);
    QStringList get_status_summary(bool colored, bool symbols);
    QMap<QString,QStringList> get_wound_summary(bool colored, bool symbols);

    HealthInfo* get_most_severe(eHealth::H_INFO h);
    bool has_info_detail(eHealth::H_INFO h, int idx);

    //list of unique wound types by category for this unit
    QHash<eHealth::H_INFO, QList<HealthInfo*> > m_wounds_info;
    //list of all treatments, these are usually a single category to description match ie. diagnosis
    QHash<eHealth::H_INFO, QList<HealthInfo*> > m_treatment_info;
    //list of all statuses, some of these overlap with wound info
    QHash<eHealth::H_INFO, QList<HealthInfo*> > m_status_info;

    static QList<QPair<eHealth::H_INFO, QString> > ordered_category_names() {return m_ordered_category_names;}
    static QHash<eHealth::H_INFO, HealthCategory*> get_display_categories() {return m_health_descriptions;}

    void add_info( eHealth::H_INFO id, bool idx0, bool idx1 = false, bool idx2 = false, bool idx3 = false);
    void add_info(eHealth::H_INFO h, QList<short> indexes, bool wound_visible = true);
    void add_info(eHealth::H_INFO h, QList<short> indexes, QList<HealthInfo *> &info_list);
    void add_info(HealthInfo *hi, QList<HealthInfo *> &info_list);

    QHash<QString, QList<HealthInfo*> > get_wound_details() {return m_wound_details;}
    QHash<eHealth::H_INFO, QList<HealthInfo*> > get_treatment_info() {return m_treatment_info;}
    QHash<eHealth::H_INFO, QList<HealthInfo*> > get_status_info() {return m_status_info;}

    bool has_critical_wounds() {return m_critical_wounds;}

    BodyPartDamage get_body_part(int body_part_id);

    bool isEmpty() {return (m_df == 0x0);}
    bool required_diagnosis() {return m_req_diagnosis;}

    QVector<VIRTADDR> layer_status_flags;

    short limb_count() {return m_limb_stand_count;}

private:
    DFInstance *m_df;
    VIRTADDR m_dwarf_addr;

    Dwarf *m_dwarf;

    QVector<VIRTADDR> health_req_flags;
    QVector<VIRTADDR> body_part_status_flags;

    QHash<int, BodyPartDamage> m_body_parts;

    //descriptive lists of health stuff. exception is wounds which are grouped by body part
    //keep multiple descriptions for permutations of colored and symbols
    QHash<int, QMap<QString,QStringList> > m_wound_summary;
    QHash<int, QStringList> m_treatment_summary;
    QHash<int, QStringList> m_status_summary;

    static QHash<eHealth::H_INFO, HealthCategory*> m_health_descriptions;
    static QList<QPair<eHealth::H_INFO, QString> > m_ordered_category_names;
    static HealthCategory *get_health_description(eHealth::H_INFO id);

    QVector<UnitWound> m_wounds;
    //keep a list of each body part, and the related health info
    QHash<QString, QList<HealthInfo*> > m_wound_details;

    bool m_critical_wounds;
    bool m_req_diagnosis;

    short m_limb_stand_count;
protected:
    void read_health_info();
    void read_wounds();
    void build_wounds_summary();
};

#endif // UNITHEALTH_H
