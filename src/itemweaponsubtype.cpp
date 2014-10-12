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
#include "truncatingfilelogger.h"

ItemWeaponSubtype::ItemWeaponSubtype(DFInstance *df, VIRTADDR address, QObject *parent)
    : QObject(parent)
    , m_address(address)
    , m_name_plural(QString::null)
    , m_adjective(QString::null)
    , m_group_name(QString::null)
    , m_single_grasp_size(0)
    , m_multi_grasp_size(0)
    , m_df(df)
    , m_mem(df->memory_layout())
{
    load_data();
}

ItemWeaponSubtype::~ItemWeaponSubtype() {
}

ItemWeaponSubtype* ItemWeaponSubtype::get_weapon(DFInstance *df, const VIRTADDR & address, QObject *parent) {
    return new ItemWeaponSubtype(df, address, parent);
}

void ItemWeaponSubtype::load_data() {
    if (!m_df || !m_mem || !m_mem->is_valid()) {
        LOGW << "load of weapons called but we're not connected";
        return;
    }
    // make sure our reference is up to date to the active memory layout
    TRACE << "Starting refresh of weapon data at" << hexify(m_address);
    read_weapon();
}

void ItemWeaponSubtype::read_weapon() {
    m_subType = m_df->read_short(m_address + m_mem->item_subtype_offset("sub_type"));
    m_adjective = capitalizeEach(m_df->read_string(m_address + m_mem->item_subtype_offset("adjective")));
    QString name = capitalizeEach(m_df->read_string(m_address + m_mem->item_subtype_offset("name")));
    QString plural = capitalizeEach(m_df->read_string(m_address + m_mem->item_subtype_offset("name_plural")));
    m_name = QString("%1 %2").arg(m_adjective).arg(name).trimmed();
    m_name_plural = QString("%1 %2").arg(m_adjective).arg(plural).trimmed();
    m_group_name = plural;
    m_single_grasp_size = m_df->read_int(m_address + m_mem->weapon_subtype_offset("single_size")); //two_hand size
    m_multi_grasp_size = m_df->read_int(m_address + m_mem->weapon_subtype_offset("multi_size")); //minimum size
    m_ammo = m_df->read_string(m_address + m_mem->weapon_subtype_offset("ammo"));
    m_melee_skill_id = m_df->read_short(m_address + m_mem->weapon_subtype_offset("melee_skill"));
    m_ranged_skill_id = m_df->read_short(m_address + m_mem->weapon_subtype_offset("ranged_skill"));
}
