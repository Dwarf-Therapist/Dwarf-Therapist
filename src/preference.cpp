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

#include "dfinstance.h"
#include "roleaspect.h"
#include "dwarf.h"
#include "itemweaponsubtype.h"
#include "itemarmorsubtype.h"
#include "races.h"
#include "plant.h"

Preference::Preference(QObject *parent)
    : QObject(parent)
    , pref_aspect(new RoleAspect(parent))
    , m_name("")
    , m_pType(LIKES_NONE)
    , m_iType(NONE)
    , m_flags()
    , m_exact_match(false)
{}

Preference::Preference(PREF_TYPES category, ITEM_TYPE iType, QObject *parent)
    : QObject(parent)
    , pref_aspect(new RoleAspect(parent))
    , m_name("")
    , m_pType(category)
    , m_iType(iType)
    , m_flags()
    , m_exact_match(false)
{}

Preference::Preference(PREF_TYPES category, QString name, QObject *parent)
    : QObject(parent)
    , pref_aspect(new RoleAspect(parent))
    , m_name(name)
    , m_pType(category)
    , m_iType(NONE)
    , m_flags()
    , m_exact_match(false)
{}

Preference::Preference(const Preference &p)
    : QObject(p.parent())
    , pref_aspect(p.pref_aspect)
    , m_name(p.m_name)
    , m_pType(p.m_pType)
    , m_iType(p.m_iType)
    , m_flags(p.m_flags)
    , m_exact_match(p.m_exact_match)
{}

void Preference::add_flag(int flag){
    m_flags.set_flag(flag,true);
}

int Preference::matches(Preference *role_pref, Dwarf *d){
    int result = 0;

    if(m_pType == role_pref->get_pref_category()){
        result = 1; //so far so good..

        if(m_iType >= 0 && role_pref->get_item_type() >= 0 && m_iType != role_pref->get_item_type()){
            result = 0;
        }

        //check for an exact match on the string, if this is required, reset our result again and check
        if(role_pref->exact_match()){
            result = (QString::compare(role_pref->get_name(),m_name,Qt::CaseInsensitive) == 0);
        }else{

            //check our unit's pref's flags for all the role's pref's flags
            if(role_pref->flags().count() > 0){
                if(m_flags.count() > 0){
                    int matches = 0;
                    foreach(int f, role_pref->flags().active_flags()){
                        if(m_flags.has_flag(f)){
                            matches++;
                        }
                    }
                    result = (result & (matches == role_pref->flags().count()));
                }else{
                    result = 0;
                }
            }

            if(result <= 0){ //only check for an exact match if we don't already have a match
                result = (QString::compare(role_pref->get_name(),m_name,Qt::CaseInsensitive) == 0);
            }

        }
        if(d){
            //if it's a weapon, and a match, ensure the dwarf can actually wield it as well
            if(result > 0 && (role_pref->get_item_type() == WEAPON ||
                              (m_pType == LIKE_ITEM &&
                               role_pref->flags().count() > 0 &&
                               (m_flags.has_flag(ITEMS_WEAPON) || m_flags.has_flag(ITEMS_WEAPON_RANGED))))){
                ItemWeaponSubtype *w = d->get_df_instance()->find_weapon_def(m_name);
                if(w){
                    result = (d->body_size(true) >= w->multi_grasp());
                }
            }
        }
    }

    return result;
}

void Preference::set_pref_flags(Race *r){
    if(r){
        if(r->flags().has_flag(HATEABLE)){
            add_flag(HATEABLE);
        }
        //set trainable flags as well for like creatures
        if(r->caste_flag(TRAINABLE)){ //really only need to add one flag for a match..
            add_flag(TRAINABLE);
        }
        if(r->caste_flag(SHEARABLE)){
            add_flag(SHEARABLE);
        }
        //fishing
        if(r->caste_flag(FISHABLE)){
            add_flag(FISHABLE);
        }
        //butcher
        if(r->caste_flag(BUTCHERABLE)){
            add_flag(BUTCHERABLE);
        }
        //milker
        if(r->caste_flag(MILKABLE)){
            add_flag(MILKABLE);
        }
        //animal dissection / beekeeper / fish dissection
        if(r->caste_flag(HAS_EXTRACTS)){
            if(r->caste_flag(FISHABLE)){
                add_flag(VERMIN_FISH);
            }else{
                add_flag(HAS_EXTRACTS);
            }
        }
    }
    m_exact_match = true;
}

void Preference::set_pref_flags(Plant *p){
    if(p){
        if(!p->flags().has_flag(P_SAPLING) && !p->flags().has_flag(P_TREE)){
            if(p->flags().has_flag(P_DRINK)){
                add_flag(P_DRINK);
            }
            if(p->flags().has_flag(P_CROP)){
                add_flag(P_CROP);
                if(p->flags().has_flag(P_SEED)){
                    add_flag(P_SEED);
                }
            }
            if(p->flags().has_flag(P_MILL)){
                add_flag(P_MILL);
            }
            if(p->flags().has_flag(P_HAS_EXTRACTS)){
                add_flag(P_HAS_EXTRACTS);
            }
        }
    }
    m_exact_match = true;
}

void Preference::set_pref_flags(const FlagArray &flags){
    m_flags = FlagArray(flags);
}

void Preference::set_pref_flags(ItemWeaponSubtype *w){
    if(w){
        if(w->is_ranged()){
            add_flag(ITEMS_WEAPON_RANGED);
        }else{
            add_flag(ITEMS_WEAPON);
        }
    }
    m_exact_match = true;
}

void Preference::set_pref_flags(ItemArmorSubtype *ias){
    if(ias){
        if(ias->armor_use()){
            add_flag(IS_ARMOR);
        }
        if(ias->clothing_use()){
            add_flag(IS_CLOTHING);
        }
    }
    m_exact_match = true;
}

