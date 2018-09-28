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

#include "itemweaponsubtype.h"
#include "dfinstance.h"
#include "memorylayout.h"

ItemWeaponSubtype::ItemWeaponSubtype(DFInstance *df, VIRTADDR address, QObject *parent)
    : ItemSubtype(WEAPON,df,address,parent)
    , m_single_grasp_size(0)
    , m_multi_grasp_size(0)
{
    auto mem = df->memory_layout();

    //set the group name as the simple plural name (no adjective or preplural)
    QString plural = capitalizeEach(df->read_string(mem->item_subtype_field(address, "name_plural")));
    m_group_name = plural;

    m_single_grasp_size = df->read_int(mem->weapon_subtype_field(address, "single_size")); //two_hand size
    m_multi_grasp_size = df->read_int(mem->weapon_subtype_field(address, "multi_size")); //minimum size
    m_melee_skill_id = df->read_short(mem->weapon_subtype_field(address, "melee_skill"));
    m_ranged_skill_id = df->read_short(mem->weapon_subtype_field(address, "ranged_skill"));

    m_ammo = df->read_string(mem->weapon_subtype_field(address, "ammo"));
    if(m_ammo.isEmpty()){
        m_flags.set_flag(ITEM_MELEE_WEAPON,true);
    }else{
        m_flags.set_flag(ITEM_RANGED_WEAPON,true);
    }
}

ItemWeaponSubtype::~ItemWeaponSubtype() {
}
