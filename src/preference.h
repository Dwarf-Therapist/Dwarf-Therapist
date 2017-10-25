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

class Dwarf;
class ItemSubtype;
class Plant;
class Race;
class Material;

class Preference {
    Q_DECLARE_TR_FUNCTIONS(Preference)
public:

    static const QString get_pref_desc(const PREF_TYPES &type) {
        switch (type) {
        case LIKE_MATERIAL: return tr("Materials");
        case LIKE_CREATURE: return tr("Creatures");
        case LIKE_FOOD: return tr("Food & Drink");
        case HATE_CREATURE: return tr("Dislikes");
        case LIKE_ITEM: return tr("Items");
        case LIKE_PLANT: return tr("Plants");
        case LIKE_TREE: return tr("Trees");
        case LIKE_COLOR: return tr("Colors");
        case LIKE_SHAPE: return tr("Shapes");
        case LIKE_POETRY: return tr("Poems");
        case LIKE_MUSIC: return tr("Music");
        case LIKE_DANCE: return tr("Dances");
        case LIKE_OUTDOORS: return tr("Outdoors");
        default: return tr("N/A");
        }
    }

    Preference(PREF_TYPES type, const QString &name); // For food/color/shape/poetry/music/dance
    virtual ~Preference() noexcept;

    const QString &get_name() const {return m_name;}
    virtual QString get_description() const;
    PREF_TYPES get_pref_category() const {return m_type;}
    bool has_flag(int f) const { return m_flags.has_flag(f); }

private:
    PREF_TYPES m_type;
    QString m_name; // compare names when doing exact matching

protected:
    Preference(PREF_TYPES type, const FlagArray &flags, const QString &name);

    FlagArray m_flags;
};

class MaterialPreference: public Preference {
public:
    MaterialPreference(const Material *m, MATERIAL_STATES state);

    QString get_description() const override;

    const Material *get_material() const { return m_mat; }
    MATERIAL_STATES get_mat_state() const { return m_mat_state; }

private:
    const Material *m_mat;
    MATERIAL_STATES m_mat_state;
};

class CreaturePreference: public Preference {
public:
    CreaturePreference(const Race *r);

    static FlagArray select_flags(const Race *r);
};

class CreatureDislike: public Preference {
public:
    CreatureDislike(const Race *r);
};

class ItemPreference: public Preference {
public:
    ItemPreference(ITEM_TYPE type);
    ItemPreference(ITEM_TYPE type, const QString &name); // item with subtype but only name is loaded
    ItemPreference(const ItemSubtype *item);

    ITEM_TYPE get_item_type() const { return m_item_type; }
    bool can_wield(const Dwarf *d) const;

private:
    ITEM_TYPE m_item_type;
    const ItemSubtype *m_item_subtype;
};

class PlantPreference: public Preference {
public:
    PlantPreference(const Plant *p);
};

class TreePreference: public Preference {
public:
    TreePreference(const Plant *p);
};

class OutdoorPreference: public Preference {
public:
    OutdoorPreference(int value);
};

#endif // PREFERENCE_H
