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
#include "dwarftherapist.h"
#include "material.h"
#include "roleaspect.h"
#include "dwarf.h"
#include "itemweapon.h"

Preference::Preference(QObject *parent)
    :QObject(parent)
    ,pref_aspect(new RoleAspect())
    ,m_name("")
    ,m_pType(LIKES_NONE)
    ,m_iType(NONE)
    ,m_material_flags()
    ,m_special_flags()
    ,m_exact_match(false)
{}

Preference::Preference(PREF_TYPES category, ITEM_TYPE iType, QObject *parent)
    :QObject(parent)
    ,pref_aspect(new RoleAspect())
    ,m_name("")
    ,m_pType(category)
    ,m_iType(iType)
    ,m_material_flags()
    ,m_special_flags()
    ,m_exact_match(false)
{}

Preference::Preference(PREF_TYPES category, QString name, QObject *parent)
    :QObject(parent)
    ,pref_aspect(new RoleAspect())
    ,m_name(name)
    ,m_pType(category)
    ,m_iType(NONE)
    ,m_material_flags()
    ,m_special_flags()
    ,m_exact_match(false)
{}


Preference::Preference(const Preference &p)
    :QObject(p.parent())
{
    pref_aspect = p.pref_aspect;
    m_name = p.m_name;
    m_pType = p.m_pType;
    m_iType = p.m_iType;
    m_material_flags = p.m_material_flags;
    m_special_flags = p.m_special_flags;
    m_exact_match = p.m_exact_match;
}

Preference::~Preference(){
    delete(pref_aspect);
    pref_aspect = 0;
}

void Preference::add_flag(int flag){
    if(!m_special_flags.contains(flag))
        m_special_flags.append(flag);
}

int Preference::matches(Preference *role_pref, Dwarf *d){
    int result = 0;

    if(m_pType == role_pref->get_pref_category()){
        result = 1; //so far so good..

        if(m_iType != role_pref->get_item_type()){
            result = 0;
        }

        //compare any other flags ie. weapon melee/ranged flags
        if(role_pref->special_flags().count() > 0){
            if(result==1)
                result = 0; //reset to 0, only match on these flags
            if(m_special_flags.count() > 0){
                foreach(int f, role_pref->special_flags()){
                    if(m_special_flags.contains(f)){
                        result = 1;
                    }
                }
            }
        }

        //compare material flags
        if(!m_material_flags.no_flags()){
            if(result == 0) //no match yet, so assume we'll get a match here unless a flag fails
                result = 1;
            foreach(int f, role_pref->special_flags()){
                if(!m_material_flags.has_flag(f)){
                    result = 0;
                    break;
                }
            }
        }

        //check for an exact match on the string, if this is required, reset our result again and check
        if(role_pref->exact_match())
            result = 0;

        if(result <= 0) //only check for an exact match if we don't already have a match
            result = exact_matches(role_pref->get_name());

        if(d){
            //if it's a weapon, and a match, ensure the dwarf can actually wield it as well
            if(result > 0 && role_pref->get_item_type() == WEAPON){
                ItemWeaponSubtype *w = d->get_df_instance()->get_weapon_def(capitalizeEach(m_name));
                if(!w || d->body_size() < w->multi_grasp())
                    result = 0;
                w = 0;
            }
        }

    }

    return result;
}

int Preference::exact_matches(QString searchval){
    return m_name.count(searchval, Qt::CaseInsensitive);
}
