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
#include "preference.h"

#include "dwarf.h"
#include "itemweaponsubtype.h"
#include "itemarmorsubtype.h"
#include "races.h"
#include "caste.h"
#include "plant.h"
#include "material.h"

Preference::Preference(PREF_TYPES type, const QString &name)
    : m_type(type)
    , m_name(name)
{
}

Preference::Preference(PREF_TYPES type, const FlagArray &flags, const QString &name)
    : m_type(type)
    , m_name(name)
    , m_flags(flags)
{
}

Preference::~Preference() noexcept
{
}

MaterialPreference::MaterialPreference(const Material *m, MATERIAL_STATES state)
    : Preference(LIKE_MATERIAL, m->flags(), m->get_material_name(state))
    , m_mat_state(state)
{
}

CreaturePreference::CreaturePreference(const Race *r)
    : Preference(LIKE_CREATURE, select_flags(r), r->plural_name().toLower())
{
}

FlagArray CreaturePreference::select_flags(const Race *r) {
    // CreaturePreference use flags from both CREATURE_FLAGS and CASTE_FLAGS.
    // We need to select flags form non-colliding subsets to avoid any confusion.
    FlagArray flags;

    if (r->flags().has_flag(HATEABLE))
        flags.set_flag(HATEABLE, true);

    if (const Caste *c = r->get_caste_by_id(0)) {
        for (auto f: {TRAINABLE, SHEARABLE, FISHABLE, BUTCHERABLE, MILKABLE, DOMESTIC}) {
            if (c->flags().has_flag(f))
                flags.set_flag(f, true);
        }

        if (c->flags().has_flag(HAS_EXTRACTS)) {
            flags.set_flag(HAS_EXTRACTS, true);
            if (r->caste_flag(FISHABLE))
                flags.set_flag(VERMIN_FISH, true);
            // from previous code, HAS_EXTRACTS was set twice:
            //else
            //    flags.set_flag(HAS_EXTRACTS, true);
        }
    }

    return flags;
}

CreatureDislike::CreatureDislike(const Race *r)
    : Preference(HATE_CREATURE, r->plural_name().toLower())
{
}

static FlagArray item_flags(ITEM_TYPE type) {
    FlagArray flags;
    if (Item::is_trade_good(type))
        flags.set_flag(IS_TRADE_GOOD, true);
    return flags;
}

ItemPreference::ItemPreference(ITEM_TYPE type)
    : Preference(LIKE_ITEM, item_flags(type), Item::get_item_name_plural(type))
    , m_item_type(type)
    , m_item_subtype(nullptr)
{
}

ItemPreference::ItemPreference(ITEM_TYPE type, const QString &name)
    : Preference(LIKE_ITEM, item_flags(type), name)
    , m_item_type(type)
    , m_item_subtype(nullptr)
{
}

ItemPreference::ItemPreference(const ItemSubtype *item)
    : Preference(LIKE_ITEM, item->flags(), item->name_plural())
    , m_item_type(item->type())
    , m_item_subtype(item)
{
}

bool ItemPreference::can_wield(const Dwarf *d) const {
    //if it's a weapon, ensure the dwarf can actually wield
    if (auto w = dynamic_cast<const ItemWeaponSubtype *>(m_item_subtype)) {
        return d->body_size(true) >= w->multi_grasp();
    }
    return true;
}

PlantPreference::PlantPreference(const Plant *p)
    : Preference(LIKE_PLANT, p->flags(), p->name_plural().toLower())
{
}

TreePreference::TreePreference(const Plant *p)
    : Preference(LIKE_TREE, p->name_plural().toLower())
{
}

OutdoorPreference::OutdoorPreference(int value)
    : Preference(LIKE_OUTDOORS,
                 value == 2
                 ? tr("Likes working outdoors")
                 : tr("Doesn't mind being outdoors"))
{
    m_flags.set_flag(999, true);
}
