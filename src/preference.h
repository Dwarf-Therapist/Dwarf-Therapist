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
#ifndef PREFERENCE_H
#define PREFERENCE_H

#include <QCoreApplication>
#include "global_enums.h"
#include "flagarray.h"
#include "roleaspect.h"

class Dwarf;
class ItemSubtype;
class Plant;
class Race;

class Preference {
    Q_DECLARE_TR_FUNCTIONS(Preference)
public:

    static const QString get_pref_desc(const PREF_TYPES &type) {
        QMap<PREF_TYPES, QString> desc;
        desc[LIKE_MATERIAL] = tr("Materials");
        desc[LIKE_CREATURE] = tr("Creatures");
        desc[LIKE_FOOD] = tr("Food & Drink");
        desc[HATE_CREATURE] = tr("Dislikes");
        desc[LIKE_ITEM] = tr("Items");
        desc[LIKE_PLANT] = tr("Plants");
        desc[LIKE_TREE] = tr("Trees");
        desc[LIKE_COLOR] = tr("Colors");
        desc[LIKE_SHAPE] = tr("Shapes");
        desc[LIKE_POETRY] = tr("Poems");
        desc[LIKE_MUSIC] = tr("Music");
        desc[LIKE_DANCE] = tr("Dances");
        desc[LIKE_OUTDOORS] = tr("Outdoors");
        return desc.value(type, tr("N/A"));
    }

    Preference();
    Preference(PREF_TYPES category, QString name);
    Preference(PREF_TYPES category, ITEM_TYPE iType);
    Preference(const Preference &p);

    int matches(Preference *role_pref, Dwarf *d = 0);

    void add_flag(int);
    void set_name(QString value) {m_name = value;}
    void set_category(PREF_TYPES cat) {m_pType = cat;}
    void set_item_type(ITEM_TYPE iType) {m_iType = iType;}
    void set_exact(bool m) {m_exact_match = m;}
    void set_mat_state (MATERIAL_STATES mat_state) {m_mat_state = mat_state;}

    QString get_name() {return m_name;}
    PREF_TYPES get_pref_category() {return m_pType;}
    ITEM_TYPE get_item_type() {return m_iType;}
    FlagArray &flags() {return m_flags;}
    bool exact_match() {return m_exact_match;}
    MATERIAL_STATES mat_state() {return m_mat_state;}

    RoleAspect pref_aspect;

    //this is a general setter for a unit's preference's flags
    void set_pref_flags(const FlagArray &flags);

    //these setters to load specific flags for the role
    void set_pref_flags(Race *r);
    void set_pref_flags(Plant *p);
    void set_pref_flags(ItemSubtype *i);

protected:
    QString m_name; //actual value to search for when doing string comparisons
    PREF_TYPES m_pType; //preference category
    ITEM_TYPE m_iType; //type of item for an dwarf's item preference
    MATERIAL_STATES m_mat_state; // for material preferences, ANY_STATE matches all

    //these flags are used when writing to the ini. they're specifically chosen to find matches of particular materials
    FlagArray m_flags;

    //if it's not a general category preference (ie. 'wood') an exact match of the string is required
    bool m_exact_match;

    bool set_flag(FlagArray origin, const int flag);
    void set_flags(FlagArray origin, const QList<int> flags);
};

#endif // PREFERENCE_H
